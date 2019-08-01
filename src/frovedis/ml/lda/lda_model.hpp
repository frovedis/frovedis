#ifndef _LDA_MODEL_HPP_
#define _LDA_MODEL_HPP_

#include "lda_cgs_config.hpp"

namespace frovedis {
// namespace lda {

template <typename TC>
class lda_model{
public:
    
    lda_model(){
        this->corpus_topic_count.val.clear();
        this->word_topic_count.val.clear();
    }
       
    ~lda_model() {}
        
    void initialize(size_t num_voc, size_t num_topics);
    
    static void mpi_gather_model(lda_model<TC>& model_l,lda_model<TC>& model_g);
    static void mpi_gather_model_sparse(lda_model<TC>& model_l,lda_model<TC>& model_g,crs_matrix_local<TC>& v);
        
    void clear_value();
    void clear();    

    void load(const std::string& filename);
    void loadbinary(const std::string& filename);
    void load_as(const std::string& filename, bool binary);

    void save(const std::string& filename);
    void savebinary(const std::string& filename);
    void save_as(const std::string& filename, bool binary);
    
    rowmajor_matrix_local<TC> corpus_topic_count;  // nr=1, nc=topic_count
    rowmajor_matrix_local<TC> word_topic_count;    // nr=vocabulary_count, nc=topic_count
        
    SERIALIZE(corpus_topic_count,word_topic_count)
};

template <typename TC>
void lda_model<TC>::clear(){
    std::vector<TC> tmp1, tmp2;
    tmp1.swap(this->corpus_topic_count.val);
    tmp2.swap(this->word_topic_count.val);
}

template <typename TC>
void lda_model<TC>::initialize( size_t num_voc, size_t num_topics){
    this->corpus_topic_count.set_local_num(1,num_topics);
    this->word_topic_count.set_local_num(num_voc,num_topics);
    this->corpus_topic_count.val.assign(num_topics,0);
    this->word_topic_count.val.assign(num_topics*num_voc,0);
}

template <typename TC>
void lda_model<TC>::clear_value(){
    for(size_t i=0; i<this->corpus_topic_count.val.size(); i++) this->corpus_topic_count.val[i]=0;
    for(size_t i=0; i<this->word_topic_count.val.size(); i++) this->word_topic_count.val[i]=0;
}

template <typename TC>
void lda_model<TC>::loadbinary(const std::string& filename){
    RLOG(TRACE) << "Loading word topic model \n";
    std::string in1 = filename + "/word_topic";
    std::string in2 = filename + "/corpus_topic";
    word_topic_count = make_crs_matrix_local_loadbinary<TC>(in1).to_rowmajor();
    corpus_topic_count = make_crs_matrix_local_loadbinary<TC>(in2).to_rowmajor();
    if (word_topic_count.local_num_col != corpus_topic_count.local_num_col) {
        throw std::runtime_error("inconsistent model file");
    }
    RLOG(TRACE) << "Finish saving word topic model \n";
}

template <typename TC>
void lda_model<TC>::load(const std::string& filename){
    RLOG(TRACE) << "Loading word topic model \n";
    std::string in1 = filename + "/word_topic";
    std::string in2 = filename + "/corpus_topic";
    word_topic_count = make_crs_matrix_local_load<TC>(in1).to_rowmajor();
    corpus_topic_count = make_crs_matrix_local_load<TC>(in2).to_rowmajor();
    if (word_topic_count.local_num_col != corpus_topic_count.local_num_col) {
        throw std::runtime_error("inconsistent model file");
    }
    RLOG(TRACE) << "Finish saving word topic model \n";
}

template <typename TC>
void lda_model<TC>::load_as(const std::string& filename, bool binary){
    if (binary) {
        loadbinary(filename);
    } else {
        load(filename);
    }
}

template <typename TC>
void lda_model<TC>::save(const std::string& filename){
    RLOG(TRACE) << "Saving word topic model \n";
    auto wtc_crs = word_topic_count.to_crs();
    auto ctc_crs = corpus_topic_count.to_crs();
    make_directory(filename);
    std::string out1 = filename + "/word_topic";
    std::string out2 = filename + "/corpus_topic";
    make_crs_matrix_scatter(wtc_crs).save(out1);
    make_crs_matrix_scatter(ctc_crs).save(out2);
    RLOG(TRACE) << "Finish saving word topic model \n";
}

template <typename TC>
void lda_model<TC>::savebinary(const std::string& filename){
    RLOG(TRACE) << "Saving word topic model \n";
    auto wtc_crs = word_topic_count.to_crs();
    auto ctc_crs = corpus_topic_count.to_crs();
    make_directory(filename);
    std::string out1 = filename + "/word_topic";
    std::string out2 = filename + "/corpus_topic";
    wtc_crs.savebinary(out1);
    ctc_crs.savebinary(out2);
    RLOG(TRACE) << "Finish saving word topic model \n";
}

template <typename TC>
void lda_model<TC>::save_as(const std::string& filename, bool binary){
    if (binary) {
        savebinary(filename);
    } else {
        save(filename);
    }
}

template <typename TC>
void lda_model<TC>::mpi_gather_model(lda_model<TC>& model_l,lda_model<TC>& model_g){  
    size_t size1 = model_l.corpus_topic_count.local_num_col;
    size_t size2 = model_l.word_topic_count.local_num_col * model_l.word_topic_count.local_num_row;  
    if(get_nodesize() > 1) {  
        typed_allreduce<TC>(model_l.corpus_topic_count.val.data(),model_g.corpus_topic_count.val.data(),size1,MPI_SUM,MPI_COMM_WORLD);
        typed_allreduce<TC>(model_l.word_topic_count.val.data(),model_g.word_topic_count.val.data(),size2,MPI_SUM,MPI_COMM_WORLD);
    } else {
        memcpy(model_g.corpus_topic_count.val.data(), model_l.corpus_topic_count.val.data(), sizeof(TC)*size1);
        memcpy(model_g.word_topic_count.val.data(), model_l.word_topic_count.val.data(), sizeof(TC)*size2);
    }
    model_l.clear_value();
}

template <typename TC>
void lda_model<TC>::mpi_gather_model_sparse(lda_model<TC>& model_l,lda_model<TC>& model_g,crs_matrix_local<TC>& v){         
    model_g.clear_value(); 
    size_t size = model_l.corpus_topic_count.local_num_col;
    typed_allreduce<TC>(model_l.corpus_topic_count.val.data(),model_g.corpus_topic_count.val.data(),size,MPI_SUM,MPI_COMM_WORLD); 
    model_g.word_topic_count = v.to_rowmajor();
    model_l.clear_value();
}

// }
}

#endif /* _LDA_MODEL_HPP_ */

