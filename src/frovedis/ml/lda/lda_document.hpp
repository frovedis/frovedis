
#ifndef _LDA_DOCUMENT_HPP_
#define _LDA_DOCUMENT_HPP_

#include "lda_cgs_config.hpp"
#include "lda_model.hpp"
namespace frovedis {
namespace lda {
    
template <typename TD, typename TW = int32_t, typename TK = int32_t>
class lda_document;       
template <typename TD, typename TW = int32_t, typename TK = int32_t>
using lda_corpus = std::vector< lda_document<TD,TW,TK> >;

template <typename TD, typename TW, typename TK>  
class lda_document{  
        
public:
    size_t token_index;
    size_t word_index;
    TD finished_token;
    std::vector<TK> topic_id;     // topic_id
    sparse_vector<TD,TW> word_id;    // word_id and token count for each word
    std::vector<size_t> voc_par_word_index;
    rowmajor_matrix_local<TD> doc_topic_count;   // topic count of this document     
        
    lda_document() {   
        this->voc_par_word_index.clear();
        this->word_id.val.clear();
        this->topic_id.clear();
        this->doc_topic_count.val.clear();
    }    
    ~lda_document() {}
    
    void initilize(size_t& num_topics){
        this->voc_par_word_index.assign(LDA_CGS_VLEN+1,this->num_word());   
        this->doc_topic_count.set_local_num(1,num_topics);
        this->doc_topic_count.val.assign(num_topics,0);  
        this->token_index = 0;
        this->word_index = 0;
        this->finished_token = 0;
        this->compute_doc_topic_count();
    }
    
    size_t num_token(){return this->topic_id.size();}    
    size_t num_word(){return this->word_id.val.size();}                      
    void compute_doc_topic_count();      
                   
    size_t get_start_word_index_by_par_index (size_t par_id){return this->voc_par_word_index[par_id];}
    size_t get_end_word_index_by_par_index (size_t par_id){return this->voc_par_word_index[par_id+1]-1;}  
    TD get_token_count_by_par_index(size_t par_id){
        size_t start = this->get_start_word_index_by_par_index(par_id);
        size_t end = this->get_end_word_index_by_par_index(par_id);
        TD tmp = this->word_id.val[end];
        if (start != 0) tmp -= this->word_id.val[start-1];
        return tmp;
    } 
    size_t get_start_token_index_by_word_index(size_t idx){return idx==0? 0:this->word_id.val[idx-1];}
    size_t get_end_token_index_by_word_index(size_t idx){return this->word_id.val[idx]-1;}    
    TD get_token_count_by_word_index(size_t idx){return idx==0? this->word_id.val[idx]: this->word_id.val[idx] - this->word_id.val[idx-1];}    
    void set_voc_par_word_idx(std::vector<size_t>& voc_par_size);           
    
    template <typename TC>
    static void distribute_doc(std::vector<TD>& token_per_doc, size_t& num_tokens, std::vector<size_t>& doc_per_thread, std::vector<TC>& token_per_thread);       
    static void dump_document(lda_corpus<TD,TW,TK>& corpus, std::string& filename, std::vector<size_t>& doc_offset, bool train);    
    template <typename TC>
    static void get_local_model(lda_corpus<TD,TW,TK>& corpus, lda_model<TC>& model);     
    static lda_corpus<TD,TW,TK> gen_corpus_from_crs(crs_matrix_local<TD>& data, lda_config& config, std::vector<TD>& token_in_doc, size_t& num_token);   
    SERIALIZE(token_index,word_index,finished_token,topic_id,word_id,voc_par_word_index,doc_topic_count)
};

template <typename TD, typename TW, typename TK>
void lda_document<TD,TW,TK>::set_voc_par_word_idx(std::vector<size_t>& voc_par_size){    
    size_t voc_idx = 0;
    this->voc_par_word_index[0] = 0;
    for (size_t i=0; i<this->num_word(); i++){
        TW word_id = this->word_id.idx[i]; 
        while(word_id >= voc_par_size[voc_idx]){
            voc_idx++;
            voc_par_word_index[voc_idx]=i;
        }
    }    
}

template <typename TD, typename TW, typename TK>
void lda_document<TD,TW,TK>::compute_doc_topic_count(){
    for(size_t i=0; i<this->doc_topic_count.val.size(); i++) 
        this->doc_topic_count.val[i] = 0;
    for(size_t i=0; i<this->num_token(); i++){
        auto topic_id = this->topic_id[i];
        this->doc_topic_count.val[topic_id]++;
    }
}

template <typename TD, typename TW, typename TK>
template <typename TC>
void lda_document<TD,TW,TK>::get_local_model(lda_corpus<TD,TW,TK>& corpus, lda_model<TC>& model){
    size_t num_topics = model.corpus_topic_count.local_num_col;
    for(size_t d=0; d<corpus.size(); d++){    
        model.corpus_topic_count = model.corpus_topic_count + corpus[d].doc_topic_count;        
        size_t word_idx = 0;
        for (size_t i=0; i<corpus[d].num_token(); i++){
            if(i > corpus[d].get_end_token_index_by_word_index(word_idx)) word_idx++;       
            TW wid = corpus[d].word_id.idx[word_idx]; 
            TK tid = corpus[d].topic_id[i];  
            size_t idx = wid*num_topics+tid;
            model.word_topic_count.val[idx]++;        
        }
    }    
}

template <typename TD, typename TW, typename TK>
void lda_document<TD,TW,TK>::dump_document(lda_corpus<TD,TW,TK>& corpus, std::string& filename, std::vector<size_t>& doc_per_thread, bool train){
    RLOG(TRACE) << "Saving doc topic model \n";
    int rank = frovedis::get_selfid(), r = 0;
    size_t doc_offset = 0;
    while ( r < rank ) {
        doc_offset += doc_per_thread[r];
        r ++;
    }
    std::string dir = "model/doc_topic_"+filename;
    if(train) dir += "_train";
    else dir += "_test";
    if(rank == 0) {
        struct stat sb;
        if(stat(dir.c_str(), &sb) != 0) { // no file/directory
            mode_t mode = S_IRWXU | S_IRWXG | S_IRWXO; // man 2 stat
            if(mkdir(dir.c_str(), mode) != 0) {
                perror("mkdir failed:");
                throw std::runtime_error("mkdir failed");
            }
        } else if(!S_ISDIR(sb.st_mode)) {
          throw std::runtime_error(dir + " is not a directory");
        }
    }
    MPI_Barrier(frovedis_comm_rpc);
    std::ofstream out(dir+"/"+std::to_string(rank)+".txt");
    size_t num_topics = corpus[0].doc_topic_count.local_num_col;
    for(size_t d=0; d<corpus.size(); d++) {
        out << d + doc_offset << ":" << "\t";
        for(size_t t=0; t<num_topics; t++) {
            auto topic_count = corpus[d].doc_topic_count.val[t];
            if(topic_count!=0)
                out << t << ":" << topic_count << "\t";
        }        
        out << "\n";
    }
    RLOG(TRACE) << "Finish saving doc topic model \n";
}

template <typename TD, typename TW, typename TK>
lda_corpus<TD,TW,TK> lda_document<TD,TW,TK>::gen_corpus_from_crs(crs_matrix_local<TD>& data, lda_config& config, std::vector<TD>& token_in_doc, size_t& num_token){
    size_t num_doc = data.local_num_row, empty_doc = 0;
    size_t num_voc = data.local_num_col, num_topics = config.num_topics;
    lda_corpus<TD,TW,TK> corpus(num_doc);
    token_in_doc.resize(num_doc);
    lda_document document;  
    config.max_token = 0; num_token = 0;
    std::vector<double> rand_num;    
#if (defined(_SX) || defined(__ve__))
    asl_library_initialize();     
    asl_random_t rng;
    asl_random_create(&rng, ASL_RANDOMMETHOD_MT19937_64);
    asl_random_distribute_uniform(rng);    
#endif
    for (size_t d = 0; d < num_doc; d++) {
        size_t start_idx = data.off[d];
        size_t voc_in_doc = data.off[d+1] - data.off[d]; 
        if(voc_in_doc==0){
            empty_doc ++;
        } else {
            document.word_id.size = num_voc;
            document.word_id.idx.resize(voc_in_doc);
            document.word_id.val.resize(voc_in_doc);
            token_in_doc[d-empty_doc] = 0;     
#pragma _NEC ivdep
            for(size_t i = 0; i < voc_in_doc; i ++){
                TW word_idx = data.idx[i+start_idx];
                TD word_count = data.val[i+start_idx];   
                document.word_id.idx[i] = word_idx;   
                document.word_id.val[i] = word_count;           
            } 
            for(size_t w=1; w<document.num_word(); w++){         
                document.word_id.val[w] += document.word_id.val[w-1];
            } 
            token_in_doc[d-empty_doc] = document.word_id.val[document.num_word()-1];
            corpus[d-empty_doc] = document;
            corpus[d-empty_doc].topic_id.resize(token_in_doc[d-empty_doc]);
            num_token += token_in_doc[d-empty_doc];
            if(token_in_doc[d-empty_doc] > config.max_token) config.max_token = token_in_doc[d-empty_doc];
        }
    }
    corpus.resize(num_doc-empty_doc);
    token_in_doc.resize(num_doc-empty_doc); 
    rand_num.resize(config.max_token);
    for(size_t d=0; d<corpus.size(); d++){    
        size_t token_count = corpus[d].num_token();
#if (defined(_SX) || defined(__ve__)) 
        asl_random_generate_d(rng,token_count,&rand_num[0]);
#else  
        for(size_t i=0; i<token_count; i++) {
            rand_num[i] = rand() / (static_cast<double>(RAND_MAX) + 0.000001);
        }
#endif 
        for (size_t i = 0; i < token_count; i ++) {
            corpus[d].topic_id[i] = rand_num[i] * num_topics;
        }
    }
#if (defined(_SX) || defined(__ve__)) 
    asl_random_destroy(rng);asl_library_finalize();
#endif
    //data.clear(); // data is not movable
    return corpus;
}

template <typename TD, typename TW, typename TK>
template <typename TC>
void lda_document<TD,TW,TK>::distribute_doc(std::vector<TD>& token_per_doc, 
                                            size_t& num_tokens, 
                                            std::vector<size_t>& doc_per_thread, 
                                            std::vector<TC>& token_per_thread) {
    auto nproc = get_nodesize();
    auto num_docs = token_per_doc.size();
    TC num_tok_per_thread = num_tokens / nproc;
    TD num_tok_doc = 0;
    TC num_tok_now = 0, tok_left = num_tokens;
    size_t num_doc_now = 0, doc_left = num_docs;
    doc_per_thread.resize(nproc); token_per_thread.resize(nproc);
    if (nproc == 1) {
      doc_per_thread[0] = num_docs;
      token_per_thread[0] = num_tokens;
    }    
    else {    
      size_t k = 0;
      for(size_t i = 0; i < num_docs; i++){
        TC diff_old = num_tok_now - num_tok_per_thread;
        diff_old = diff_old < 0 ? -diff_old : diff_old;
        num_tok_doc = token_per_doc[i];
        num_tok_now += num_tok_doc;
        TC diff_new = num_tok_now - num_tok_per_thread;
        diff_new = diff_new < 0 ? -diff_new : diff_new;
        if(diff_new <= diff_old) num_doc_now++;
        else {
          doc_per_thread[k] = num_doc_now;
          token_per_thread[k] = num_tok_now - num_tok_doc;
          doc_left -= doc_per_thread[k];
          tok_left -= token_per_thread[k];
          num_doc_now = 1;
          num_tok_now = num_tok_doc;
          k++;
          if (k == nproc - 1) break; 
        } 
      }           
      doc_per_thread[k] = doc_left;
      token_per_thread[k] = tok_left;
    }
    vector_clear(token_per_doc);
}

}
}
#endif /* _LDA_DOCUMENT_HPP_ */

