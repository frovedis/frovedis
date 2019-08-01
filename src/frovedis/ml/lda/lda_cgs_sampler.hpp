
#ifndef _LDA_CGS_SAMPLER_HPP_
#define _LDA_CGS_SAMPLER_HPP_

#include "lda_document.hpp"
#include "lda_cgs_alias_table.hpp"
#include "lda_cgs_config.hpp"

#define PRIO_MAX 1000      // at most change priority PRIO_MAX times
#define PRIO_STEP 50       // explore frequency of chaning priority: 50 100 150 ...
#define GEN_RAND_FREQ 1000 // generate the random values needed for GEN_RAND_FREQ samplings at one time
#define SHRINK_FREQ 10     // shrink the size of alias tables to number of nonzero entries every SHRINK_FREQ epochs

namespace frovedis {
namespace lda {
    
template <typename TC, typename TD = int32_t, typename TW = int32_t, typename TK = int32_t, typename TA = int16_t>
class lda_sampler {
    
private:    
    int itr_count;   
    lda_config config; 
    std::vector<size_t> voc_par_size;   //size=256, voc_par_size[i] = # of words in 0~i partition
    std::vector<size_t> doc_par_size;   //size=257, doc_par_size[i] = # of documents in 0~i partition, doc_par_size[256]= max # of documents in a partition
    
    //MH-based LDA
    alias_table<TC,TK,TA> table_wp;    
    //SparseLDA
    std::vector<double> coefficient;
    std::vector<double> dist1;
    std::vector<double> dist2;
    std::vector< std::pair<TK,double> > dist3;
    
    //best sequence for sampling
    std::vector<int> opt_seq;
    int min_loop_count;
    int opt_prio_step;
    bool explore;    
    std::vector<TC> token_count;
    std::vector<TC> token_left;
    
    //local model duplicates
    std::vector<TC> ct_count_v;   
    
    int32_t subtractor;    
    
    SERIALIZE_NONE
        
    void set_doc_par_size (lda_corpus<TD,TW,TK>& corpus, TC& token_count); 
    void set_voc_par_size (lda_corpus<TD,TW,TK>& corpus, TC& token_count);
    void cal_doc_par_efficiency(lda_corpus<TD,TW,TK>& corpus);
    void cal_doc_voc_par_efficiency(lda_corpus<TD,TW,TK>& corpus);  
    
    // used for vectorized version
    void sample_for_doc_par_mh(lda_corpus<TD,TW,TK>& corpus, std::vector<TC>& c1, std::vector<TC>& c2, std::vector<TC>& c3);     
    void sample_for_doc_par_mh_delay_update(lda_corpus<TD,TW,TK>& corpus, lda_model<TC>& model, std::vector<TC>& c1, std::vector<TC>& c2); 
    void sample_for_doc_par_cgs(lda_corpus<TD,TW,TK>& corpus, std::vector<TC>& c1, std::vector<TC>& c2, std::vector<TC>& c3);     
    void sample_for_doc_par_cgs_delay_update(lda_corpus<TD,TW,TK>& corpus, lda_model<TC>& model, std::vector<TC>& c3, std::vector<TC>& c4); 
    
    // used for un-vectorized version
    void sample_for_doc(lda_corpus<TD,TW,TK>& corpus, lda_model<TC>& model_g, lda_model<TC>& model_l, size_t& doc_id);
    TK sample_for_word_cgs(lda_document<TD,TW,TK>& doc, lda_model<TC>& model, TW& word_id);
    TK sample_for_word_mh(lda_document<TD,TW,TK>& doc, lda_model<TC>& model, TW& word_id, TK& topic_id);
    TK sample_for_word_sparse();
    
    // calculate distribution used in SparseLDA
    void cal_distribution_1(lda_model<TC>& model);
    void cal_distribution_2(lda_document<TD,TW,TK>& doc);
    void cal_distribution_3(lda_document<TD,TW,TK>& doc, lda_model<TC>& model, TW& word_id); 
    
#if (defined(_SX) || defined(__ve__)) 
    asl_random_t rng;
    void initialize_asl_rng(){
        asl_library_initialize(); 
        asl_random_create(&rng, ASL_RANDOMMETHOD_MT19937_64);
        asl_int_t len = 10;
        std::vector<asl_uint32_t> seed(len);
        for(auto& i:seed) i = rand();
        asl_random_initialize(rng,len,&seed[0]);
        asl_random_distribute_uniform(rng);    
    } 
    void finalize_asl_rng()  {asl_random_destroy(rng);asl_library_finalize();} 
#endif
    
public:   
    
    lda_sampler() {}
    
    ~lda_sampler(){
#if (defined(_SX) || defined(__ve__))
        finalize_asl_rng();
#endif
    }   
    
    void sample_for_corpus_vec(lda_corpus<TD,TW,TK>& corpus, lda_model<TC>& model_g, lda_model<TC>& model_l);
    void sample_for_corpus_unvec(lda_corpus<TD,TW,TK>& corpus, lda_model<TC>& model_g, lda_model<TC>& model_l);     
    void initialize(lda_corpus<TD,TW,TK>& corpus, std::vector<TC>& local_token_count, lda_config& config);         
    
};

template <typename TC, typename TD, typename TW, typename TK, typename TA>
void lda_sampler<TC,TD,TW,TK,TA>::sample_for_corpus_vec(lda_corpus<TD,TW,TK>& corpus, lda_model<TC>& model_g, lda_model<TC>& model_l){
        
// ------------------- generate alias table ------------------------------ //
    if((config.train || this->itr_count==0) && (config.algorithm==lda_config::wp_cgs || config.algorithm==lda_config::cp_cgs)) {
        this->table_wp.gen_wp_table_sparse(model_g,config);
        if(this->itr_count % SHRINK_FREQ == 0) {
            this->table_wp.shrink_table(config.num_voc);
        }
    }  
    
// ------------------- get local count copy ------------------------------ //
    const size_t num_topics = config.num_topics, t_x_v = num_topics * LDA_CGS_VLEN;  
    bool not_delay_ = config.delay_update == false;
    if(not_delay_) {
        for (size_t t=0; t<num_topics; t++) {     // copy corpus-topic count 
            for(size_t v=0; v<LDA_CGS_VLEN; v++) 
                ct_count_v[t_x_v+v*num_topics+t] = model_g.corpus_topic_count.val[t];
        }     
    }    
         
    // ------------------- vectorized sampling ------------------------------          
    if (config.algorithm == lda_config::original_cgs) {
        if(not_delay_) sample_for_doc_par_cgs(corpus,ct_count_v,model_g.word_topic_count.val,model_l.word_topic_count.val); 
        else sample_for_doc_par_cgs_delay_update(corpus,model_g,ct_count_v,model_l.word_topic_count.val); 
    } else if (config.algorithm == lda_config::dp_cgs || config.algorithm == lda_config::wp_cgs || config.algorithm == lda_config::cp_cgs ) {
        if(not_delay_) sample_for_doc_par_mh(corpus,ct_count_v,model_g.word_topic_count.val,model_l.word_topic_count.val); 
        else sample_for_doc_par_mh_delay_update(corpus,model_g,ct_count_v,model_l.word_topic_count.val); 
    } else {
        RLOG(ERROR) << " Vectorized sampling not defined for current algorithm ! " << std::endl;
    }  
    
// --------------------- aggregate local model ----------------------------- //
    if(config.train){
        for (size_t v=0; v<LDA_CGS_VLEN; v++){
            for (size_t t=0; t<num_topics; t++){
                model_l.corpus_topic_count.val[t] += ct_count_v[v*num_topics+t];
                ct_count_v[v*num_topics+t] = 0;
            }
        }     
    }      
    itr_count++;
}

template <typename TC, typename TD, typename TW, typename TK, typename TA>
void lda_sampler<TC,TD,TW,TK,TA>::sample_for_doc_par_mh(lda_corpus<TD,TW,TK>& corpus, std::vector<TC>& c1, std::vector<TC>& c2, std::vector<TC>& c3){ 
    size_t num_topics = config.num_topics, num_voc = config.num_voc;    
    size_t k_x_v = num_topics * LDA_CGS_VLEN; 
    double alpha = config.alpha, alphaK = config.alphaK, beta = config.beta, betaW = config.betaW;
    double betaK = num_topics * beta;
    int finished_par = 0, count = 0; 
    std::vector<size_t> doc_idx(LDA_CGS_VLEN);
    int vec_id[num_voc];       
    for(size_t w=0; w<num_voc; w++) vec_id[w]=-1; 
    std::vector<double> random_num;
    int rand_size = LDA_MH_STEP*7; 
    size_t total_rand_size = rand_size*GEN_RAND_FREQ;
    random_num.resize(total_rand_size); 
             
    int prio_idx = PRIO_MAX;
    std::vector<int> seq;
    std::vector<TC> token_left_tmp;
    if(this->explore && this->config.train){
        seq.resize(LDA_CGS_VLEN*prio_idx); token_left_tmp.resize(LDA_CGS_VLEN);
        prio_idx = -1;
        for(size_t i=0; i<LDA_CGS_VLEN; i++){
            this->token_left[i] = this->token_count[i];
        }
    }
    
#pragma _NEC ivdep
    for(size_t v=0; v<LDA_CGS_VLEN; v++){ 
        doc_idx[v] = v == 0? 0:this->doc_par_size[v-1];
    }
    
    if(this->itr_count==0 && config.train){
        for (size_t d=0; d<doc_par_size[LDA_CGS_VLEN];d++){    
#pragma _NEC ivdep 
            for(size_t v=0; v<LDA_CGS_VLEN; v++){ 
                size_t doc_id = doc_idx[v] + d;
                size_t doc_last = this->doc_par_size[v]-1;
                if(doc_id <= doc_last){              
                    size_t word_idx  = corpus[doc_id].get_start_word_index_by_par_index(v);
                    if(word_idx ==corpus[doc_id].num_word()) word_idx=0;  // no word in current vocabulary partition
                    size_t token_idx = corpus[doc_id].get_start_token_index_by_word_index(word_idx);
                    corpus[doc_id].word_index = word_idx;
                    corpus[doc_id].token_index = token_idx;
                }
            }
        }
    } 
    
//=================================================== SAMPLING LOOP ==========================================================//
    while(finished_par != LDA_CGS_VLEN){            
        finished_par = 0;      
        
// ---------------------- RNG ----------------------- // 
        int gen_rand = count % GEN_RAND_FREQ;
        if(gen_rand==0) {
            size_t gen_rand_size = total_rand_size;
            if(!this->explore && min_loop_count-count<GEN_RAND_FREQ){
                gen_rand_size = (min_loop_count-count)*rand_size;
            }
#if (defined(_SX) || defined(__ve__))
            asl_random_generate_d(rng,gen_rand_size,&random_num[0]);
#else  
            for(size_t v=0; v<gen_rand_size; v++) {
                random_num[v] = rand() / static_cast<double>(RAND_MAX);
                random_num[v] = random_num[v]==1? random_num[v]-0.000001:random_num[v];
            }
#endif 
        } 
// ------------------ doc priority -------------------- //    
        if (this->config.train) {
            if (this->explore){
                int tmp = (this->itr_count+1)*PRIO_STEP;
                if (tmp > this->min_loop_count && this->itr_count!=0) this->explore=false;
                else {
                    if(count % tmp == 0 && prio_idx<PRIO_MAX-1) { 
                        prio_idx++;
                        for(size_t v=0; v<LDA_CGS_VLEN; v++) seq[prio_idx*LDA_CGS_VLEN+v]=v; 
                        for(size_t v=0; v<LDA_CGS_VLEN; v++) token_left_tmp[v]=this->token_left[v];      
#if !(defined(_SX) || defined(__ve__))
                        radix_sort_desc<TC,int>(&token_left_tmp[0], &seq[prio_idx*LDA_CGS_VLEN], LDA_CGS_VLEN);
#else
                        radix_sort<TC,int>(&token_left_tmp[0], &seq[prio_idx*LDA_CGS_VLEN], LDA_CGS_VLEN);
#endif
                    } 
                }
            }
            else {
                prio_idx = count/this->opt_prio_step;
                prio_idx = prio_idx<PRIO_MAX? prio_idx:PRIO_MAX-1;
            }
        }
        count ++;   

#if !(defined(_SX) || defined(__ve__))
    for(auto& i:vec_id) i=-1;
#endif  
// ------------------- mh-based cgs sampling -------------------- //
#pragma _NEC ivdep
        for(size_t v1=0; v1<LDA_CGS_VLEN; v1++) {
            size_t v = v1, seq_idx = prio_idx*LDA_CGS_VLEN;
            int sample = 0, finish_ = 0;
            if(config.train) v = this->explore? seq[seq_idx+v1]:opt_seq[seq_idx+v1];
            size_t doc_idx_ = doc_idx[v], doc_last = this->doc_par_size[v]-1;
            TW word_id = -1; TK topic_id = -1; 
            if(doc_idx_ > doc_last) finish_ = 1;
            if(!finish_) {                    
                word_id  = corpus[doc_idx_].word_id.idx[corpus[doc_idx_].word_index];
                topic_id = corpus[doc_idx_].topic_id[corpus[doc_idx_].token_index];
#if (defined(_SX) || defined(__ve__))
                vec_id[word_id]=v;
#else
                if(vec_id[word_id]==-1) vec_id[word_id]=v;
#endif
            }                    
            if(!finish_) {
                if(!config.train || vec_id[word_id]==v) {sample = 1;}
            }  else {
                finished_par++;
            }
            if(sample){                               
                //  -------------------- sampling ------------------------ //
                TK old_topic = topic_id, new_topic = -1;
                TD doc_token_count = corpus[doc_idx_].num_token();
                size_t offset2 = word_id*num_topics, offset1 = v*num_topics+k_x_v, rand_idx = gen_rand * rand_size;        
#pragma _NEC unroll(LDA_MH_STEP)  
                for(int s=0; s<LDA_MH_STEP; s++){
                    double nwo,nto,ndo,nwn,ntn,ndn,accept,which_component;
                    // sample from document proposal  
                    if(config.algorithm == lda_config::dp_cgs || config.algorithm==lda_config::cp_cgs) {
                        which_component = random_num[rand_idx++]*(alphaK+doc_token_count);
                        if(which_component < doc_token_count) {
                            size_t rand_token_idx = random_num[rand_idx++] * doc_token_count;
                            new_topic = corpus[doc_idx_].topic_id[rand_token_idx];
                        } else {
                            new_topic = random_num[rand_idx++] * num_topics;
                        }        
                        if(new_topic != old_topic){
                            nwo = c2[offset2+old_topic] + beta; 
                            nwn = c2[offset2+new_topic] + beta; 
                            nto = c1[offset1+old_topic] + betaW;
                            ntn = c1[offset1+new_topic] + betaW;        
                            accept = 1.0;
                            if(new_topic==topic_id) {
                                ntn -= this->subtractor; nwn -= this->subtractor;
                            } else if(old_topic==topic_id) {    
                                nto -= this->subtractor; nwo -= this->subtractor;
                            }
                            accept = (nwn * nto) / (nwo * ntn);   
                            if(random_num[rand_idx++] < accept) old_topic = new_topic; 
                        }  
                    }
                    // sample from word proposal
                    if(config.algorithm == lda_config::wp_cgs || config.algorithm ==lda_config::cp_cgs) {
                        which_component = random_num[rand_idx++] * (table_wp.get_norm(word_id)+betaK);  
                        if(which_component < betaK) {                          
                            new_topic = random_num[rand_idx++] * num_topics;
                        } else {
                            size_t idx_s = word_id==0? 0:table_wp.get_num_nonzeros(word_id-1);
                            size_t idx_e = table_wp.get_num_nonzeros(word_id);
                            size_t index = random_num[rand_idx++] * (idx_e - idx_s) + idx_s; 
                            if(random_num[rand_idx++] < table_wp.get_split_val(index)) {
                                new_topic = table_wp.get_bottom_topic(index);
                            } else {
                                new_topic = table_wp.get_top_topic(index);
                            }  
                        }
                        if(new_topic != old_topic){
                            ndo = corpus[doc_idx_].doc_topic_count.val[old_topic] + alpha;
                            ndn = corpus[doc_idx_].doc_topic_count.val[new_topic] + alpha;  
                            nto = c1[offset1+old_topic] + betaW;
                            ntn = c1[offset1+new_topic] + betaW; 
                            accept = 1.0;
                            if(old_topic==topic_id) { 
                                ndo -= 1; nto -= this->subtractor;
                            } else if(new_topic==topic_id) {
                                ndn -= 1; ntn -= this->subtractor;
                            }  
                            accept = (ndn*nto) / (ndo*ntn);
                            if(random_num[rand_idx++] < accept) old_topic = new_topic;                        
                        } 
                    }
                }
                new_topic = old_topic;
                //  ----------------- model update ---------------------- // 
                size_t idx2 = offset2 + new_topic;
                if(new_topic != topic_id){
                    c1[offset1+new_topic] += this->subtractor; c1[offset1+topic_id] -= this->subtractor;
                    c2[idx2] += this->subtractor; c2[offset2+topic_id] -= this->subtractor;
                }
                c1[v*num_topics+new_topic] += this->subtractor; 
                c3[idx2] += this->subtractor;
                if(new_topic != topic_id){
                    corpus[doc_idx_].doc_topic_count.val[topic_id]  --;  
                    corpus[doc_idx_].doc_topic_count.val[new_topic] ++;    
                    corpus[doc_idx_].topic_id[corpus[doc_idx_].token_index] = new_topic;
                }   
                size_t next_idx = corpus[doc_idx_].token_index+1;
                if(next_idx == doc_token_count) {
                    next_idx -= doc_token_count;
                    corpus[doc_idx_].word_index = 0;
                } else if (next_idx > corpus[doc_idx_].get_end_token_index_by_word_index(corpus[doc_idx_].word_index)){
                    corpus[doc_idx_].word_index = corpus[doc_idx_].word_index + 1;
                }                
                corpus[doc_idx_].token_index = next_idx;    
                corpus[doc_idx_].finished_token ++;
                this->token_left[v]--;
                if(corpus[doc_idx_].finished_token==doc_token_count){
                    corpus[doc_idx_].finished_token = 0;
                    doc_idx[v] ++;                               
                }      
            }   
        }
    }
    if(config.train) {
        if((this->explore && count<this->min_loop_count) || this->itr_count==0){
            this->min_loop_count = count;
            this->opt_prio_step = (this->itr_count+1)*PRIO_STEP;
            for(size_t i=0; i<LDA_CGS_VLEN * (prio_idx+1); i++) {
                this->opt_seq[i] = seq[i];
            }
        }
        if(this->itr_count==config.exp_iter-1){
            this->explore = 0;
            size_t size = (this->min_loop_count-1) / this->opt_prio_step + 1;
            this->opt_seq.resize(LDA_CGS_VLEN*size);
        }   
    } else if (this->explore){
        this->explore = false;
        this->min_loop_count = count;
    }
}

template <typename TC, typename TD, typename TW, typename TK, typename TA>
void lda_sampler<TC,TD,TW,TK,TA>::sample_for_doc_par_cgs(lda_corpus<TD,TW,TK>& corpus, std::vector<TC>& c1, std::vector<TC>& c2, std::vector<TC>& c3){
    double alpha = config.alpha, beta = config.beta, betaW = config.betaW;
    size_t num_topics = config.num_topics, num_voc = config.num_voc;
    size_t topicsV = num_topics * LDA_CGS_VLEN;    
    int finished_par = 0, count = 0;
    std::vector<size_t> doc_idx(LDA_CGS_VLEN);
    std::vector<int> vec_id(num_voc);
    std::vector<double> random_num(GEN_RAND_FREQ);  
    std::vector<double> topic_distribution(topicsV);  
    double normalization[LDA_CGS_VLEN];   
    TK new_topic[LDA_CGS_VLEN]; 
    TW word_id[LDA_CGS_VLEN];
    TK topic_id[LDA_CGS_VLEN];
    int sample[LDA_CGS_VLEN];
#if defined(LDA_CGS_USE_VREG)
#pragma _NEC vreg(normalization)
#pragma _NEC vreg(new_topic)
#pragma _NEC vreg(topic_id)
#pragma _NEC vreg(word_id)
#pragma _NEC vreg(sample)
#endif
    for(size_t w=0; w<num_voc; w++) vec_id[w]=-1; 

    int prio_idx = PRIO_MAX;
    std::vector<int> seq;
    std::vector<TC> token_left_tmp;
    if(this->explore && config.train){
        seq.resize(LDA_CGS_VLEN*prio_idx); token_left_tmp.resize(LDA_CGS_VLEN);
        prio_idx = -1;
        for(size_t i=0; i<LDA_CGS_VLEN; i++){
            this->token_left[i] = this->token_count[i];
        }
    }
    
#pragma _NEC ivdep
    for(size_t v=0; v<LDA_CGS_VLEN; v++){ 
        doc_idx[v] = v == 0? 0:this->doc_par_size[v-1];
    }
    
    if(this->itr_count==0 && config.train){
        for (size_t d=0; d<doc_par_size[LDA_CGS_VLEN];d++){    
#pragma _NEC ivdep 
            for(size_t v=0; v<LDA_CGS_VLEN; v++){ 
                size_t doc_id = doc_idx[v] + d;
                size_t doc_last = this->doc_par_size[v]-1;
                if(doc_id <= doc_last){
                    size_t word_idx = corpus[doc_id].get_start_word_index_by_par_index(v);
                    if(word_idx==corpus[doc_id].num_word()) word_idx=0;  // no word in current vocabulary partition
                    size_t token_idx = corpus[doc_id].get_start_token_index_by_word_index(word_idx);
                    corpus[doc_id].token_index = token_idx;
                    corpus[doc_id].word_index = word_idx;
                }
            }
        }
    }    
//=================================================== SAMPLING LOOP ==========================================================//
    while(finished_par != LDA_CGS_VLEN){            
        finished_par = 0;      
        
// ---------------------- RNG ----------------------- //        
        int gen_rand = count % GEN_RAND_FREQ;
        if(gen_rand==0) {
            size_t gen_rand_size = GEN_RAND_FREQ;
            if(!this->explore && min_loop_count-count<GEN_RAND_FREQ){
                gen_rand_size = min_loop_count-count;
            }
#if (defined(_SX) || defined(__ve__))
            asl_random_generate_d(rng,gen_rand_size,&random_num[0]);
#else  
            for(size_t v=0; v<gen_rand_size; v++) {
                random_num[v] = rand() / static_cast<double>(RAND_MAX);
                random_num[v] = random_num[v]==1? random_num[v]-0.000001:random_num[v];
            }
#endif 
        }
        double random_num_ = random_num[gen_rand];
// ------------------ doc priority -------------------- //    
        if(config.train) {
            if(this->explore){
                int tmp = (this->itr_count+1)*PRIO_STEP;
                if (tmp > this->min_loop_count && this->itr_count!=0) this->explore=false;
                else {
                    if(count % tmp == 0 && prio_idx<PRIO_MAX-1) { 
                        prio_idx++;
                        for(size_t v=0; v<LDA_CGS_VLEN; v++) seq[prio_idx*LDA_CGS_VLEN+v]=v; 
                        for(size_t v=0; v<LDA_CGS_VLEN; v++) token_left_tmp[v]=this->token_left[v];      
#if !(defined(_SX) || defined(__ve__))
                        radix_sort_desc<TC,int>(&token_left_tmp[0], &seq[prio_idx*LDA_CGS_VLEN], LDA_CGS_VLEN);
#else
                        radix_sort<TC,int>(&token_left_tmp[0], &seq[prio_idx*LDA_CGS_VLEN], LDA_CGS_VLEN);
#endif
                    }
                }
            }
            else {
                prio_idx = count/this->opt_prio_step;
                prio_idx = prio_idx<PRIO_MAX? prio_idx:PRIO_MAX-1;
            }
        }
        count ++;    
#if !(defined(_SX) || defined(__ve__))
    for(auto& i:vec_id) i=-1;
#endif   
#pragma _NEC ivdep
        for(size_t v1=0; v1<LDA_CGS_VLEN; v1++) {
            size_t seq_idx = prio_idx*LDA_CGS_VLEN + v1;
            size_t v = v1;
            if(config.train) v=this->explore? seq[seq_idx]:opt_seq[seq_idx];
            size_t doc_idx_ = doc_idx[v], doc_last = this->doc_par_size[v]-1;
            TW word_id_ = -1;  
            if(doc_idx_ <= doc_last){
                word_id_ = corpus[doc_idx_].word_id.idx[corpus[doc_idx_].word_index];
#if (defined(_SX) || defined(__ve__))
                vec_id[word_id_]=v;
#else
                if(vec_id[word_id_]==-1) vec_id[word_id_]=v;
#endif
            }  else finished_par ++;       
        }
        for(size_t v=0; v<LDA_CGS_VLEN; v++) {
            sample[v] = 0;
            size_t doc_idx_ = doc_idx[v], doc_last = this->doc_par_size[v]-1;  
            if (doc_idx_ <= doc_last) {                
                word_id[v] = corpus[doc_idx_].word_id.idx[corpus[doc_idx_].word_index];
                topic_id[v]= corpus[doc_idx_].topic_id[corpus[doc_idx_].token_index];
                if(!config.train || vec_id[word_id[v]]==v) {
                    sample[v] = 1;
                }
            } 
            normalization[v] = 0; 
            new_topic[v] = num_topics;
        }
        for (size_t t=0; t<config.num_topics; t++){
            for(size_t v=0; v<LDA_CGS_VLEN; v++) {                
                TK topic_id_ = topic_id[v];
                if(sample[v]){                                
                //  -------------------- sampling ------------------------ //  
                    size_t idx1 = v*num_topics+t;           
                    double word_topic_count = c2[word_id[v]*num_topics+t] + beta; 
                    double topic_count      = c1[idx1 + topicsV] + betaW;
                    double doc_topic_count  = corpus[doc_idx[v]].doc_topic_count.val[t] + alpha;
                    if(t == topic_id_) {
                        word_topic_count -= this->subtractor;
                        topic_count -= this->subtractor;
                        doc_topic_count -= this->subtractor;
                    }
                    topic_distribution[idx1] = word_topic_count * doc_topic_count / topic_count;                    
                    normalization[v] += topic_distribution[idx1];
                } 
            }
        }                
        for(size_t v=0; v<LDA_CGS_VLEN; v++) {
            normalization[v] *= random_num_;
        }
        for (size_t t=0; t<num_topics; t++){  
            for(size_t v=0; v<LDA_CGS_VLEN; v++) {
                if(sample[v]){         
                    normalization[v] -= topic_distribution[v*num_topics+t];   
                    if(normalization[v]<0 && t<new_topic[v]) new_topic[v] = t;                   
                }
            }
        }
#pragma _NEC ivdep
        for(size_t v=0; v<LDA_CGS_VLEN; v++) {
            if(sample[v]){  
                if(new_topic[v]==num_topics) new_topic[v]-=1;  
                TK new_topic_ = new_topic[v], topic_id_ = topic_id[v];  
                size_t doc_idx_= doc_idx[v];
                size_t next_idx_ = corpus[doc_idx_].token_index;
                //  ----------------- model update ---------------------- //
                size_t offset1 = v*num_topics, offset2 = word_id[v]*num_topics;
                size_t idxn1 = offset1+new_topic_, idxn2 = offset2+new_topic_;
                size_t idxo1 = offset1+topic_id_,  idxo2 = offset2+topic_id_;
                c1[idxn1] += this->subtractor; 
                c3[idxn2] += this->subtractor;
                if(new_topic_ != topic_id_){
                    c1[idxn1+topicsV] += this->subtractor; c1[idxo1+topicsV] -= this->subtractor;
                    c2[idxn2] += this->subtractor; c2[idxo2] -= this->subtractor;
                    corpus[doc_idx_].doc_topic_count.val[topic_id_]  --;  
                    corpus[doc_idx_].doc_topic_count.val[new_topic_] ++;    
                    corpus[doc_idx_].topic_id[next_idx_] = new_topic_;
                }
                TD doc_token_count = corpus[doc_idx_].num_token();
                next_idx_ = next_idx_ + 1;
                if(next_idx_ == doc_token_count){
                    next_idx_ = 0;
                    corpus[doc_idx_].word_index = 0;
                } else if (next_idx_ > corpus[doc_idx_].get_end_token_index_by_word_index(corpus[doc_idx_].word_index)){
                    corpus[doc_idx_].word_index ++;
                }
                corpus[doc_idx_].token_index = next_idx_;    
                corpus[doc_idx_].finished_token ++;
                if(corpus[doc_idx_].finished_token == doc_token_count) {
                    corpus[doc_idx_].finished_token = 0;
                    doc_idx[v] ++;
                }
                this->token_left[v]--;                  
            }   
        }       
    }
    if(config.train){
        if((this->explore && count<this->min_loop_count) || this->itr_count==0){
            this->min_loop_count = count;
            this->opt_prio_step = (this->itr_count+1)*PRIO_STEP;
            for(size_t i=0; i<LDA_CGS_VLEN * (prio_idx+1); i++) {
                this->opt_seq[i] = seq[i];
            }
        }
        if(this->itr_count==config.exp_iter-1){
            this->explore = 0;
            size_t size = (this->min_loop_count-1) / this->opt_prio_step + 1;
            this->opt_seq.resize(LDA_CGS_VLEN*size);
        } 
    } else if (this->explore){
        this->explore = false;
        this->min_loop_count = count;
    }
}

template <typename TC, typename TD, typename TW, typename TK, typename TA>
void lda_sampler<TC,TD,TW,TK,TA>::sample_for_doc_par_mh_delay_update(lda_corpus<TD,TW,TK>& corpus, lda_model<TC>& model, std::vector<TC>& c1, std::vector<TC>& c2){
    size_t num_topics = config.num_topics, num_voc = config.num_voc, w_x_k = num_topics * num_voc;    
    double alpha = config.alpha, alphaK = config.alphaK, beta = config.beta, betaW = config.betaW;
    double betaK = num_topics * beta;
    int finished_par = 0, count = 0;
    std::vector<size_t> doc_idx(LDA_CGS_VLEN);
    size_t model_idx[LDA_CGS_VLEN];
    TW word_ids[LDA_CGS_VLEN];
#if defined(LDA_CGS_USE_VREG)
#pragma _NEC vreg(model_idx)
#pragma _NEC vreg(word_ids)
#endif
    for(auto& i:model_idx) i = w_x_k;
    int vec_id[num_voc];
    for(size_t w=0; w<num_voc; w++)  vec_id[w]=-1;     
    std::vector<double> random_num;
    int rand_size = LDA_MH_STEP*7;  // 7 random numbers required in each cycle-proposal sampling step
    size_t total_rand_size = rand_size*GEN_RAND_FREQ;
    random_num.resize(total_rand_size); 
          
#pragma _NEC ivdep
    for(size_t v=0; v<LDA_CGS_VLEN; v++){ 
        doc_idx[v] = v == 0? 0:this->doc_par_size[v-1];
    }   
        
    if(this->itr_count==0 && config.train){
        for (size_t d=0; d<doc_par_size[LDA_CGS_VLEN];d++){    
#pragma _NEC ivdep 
            for(size_t v=0; v<LDA_CGS_VLEN; v++){ 
                size_t doc_id = doc_idx[v] + d;
                if(doc_id < this->doc_par_size[v]){
                    size_t word_idx = corpus[doc_id].get_start_word_index_by_par_index(v);
                    if(word_idx ==corpus[doc_id].num_word()) word_idx=0;  // no word in current vocabulary partition
                    size_t token_idx = corpus[doc_id].get_start_token_index_by_word_index(word_idx);
                    corpus[doc_id].word_index = word_idx;
                    corpus[doc_id].token_index = token_idx;                   
                }
            }
        } 
    }
     
    while(finished_par != LDA_CGS_VLEN){    
        finished_par = 0;   
//=============================== RNG ===============================//         
        int gen_rand = count % GEN_RAND_FREQ;
        if(gen_rand==0) {
            size_t gen_rand_size = random_num.size();
            if(this->itr_count!=0) {
                int left_loop = min_loop_count-count;
                if (left_loop < GEN_RAND_FREQ) gen_rand_size = left_loop * rand_size;
            }
#if (defined(_SX) || defined(__ve__))
            asl_random_generate_d(rng,gen_rand_size,&random_num[0]);
#else  
            for(size_t v=0; v<gen_rand_size; v++) {
                random_num[v] = rand() / static_cast<double>(RAND_MAX);
                random_num[v] = random_num[v]==1? random_num[v]-0.000001:random_num[v];
            }
#endif 
        }                    
        count ++;  
//=============================== SAMPLING ===============================//  
#pragma _NEC ivdep
        for(size_t v=0; v<LDA_CGS_VLEN; v++) { 
            size_t doc_idx_ = doc_idx[v];  
            if(doc_idx_ >= this->doc_par_size[v]) {
                finished_par ++; 
                model_idx[v] = w_x_k;
            } else {    
                size_t token_idx  = corpus[doc_idx_].token_index;                              
                size_t word_idx  = corpus[doc_idx_].word_index;
                TW word_id   = corpus[doc_idx_].word_id.idx[word_idx];
                TK topic_id  = corpus[doc_idx_].topic_id[token_idx];
                TK old_topic = topic_id;
                TK new_topic = -1; 
                size_t wtc_offset= word_id*num_topics;   
                TD doc_token_count = corpus[doc_idx_].num_token();                   
                //  -------------------- sampling ------------------------ //      
                size_t rand_idx = gen_rand*rand_size;        
#pragma _NEC unroll(LDA_MH_STEP)                            //!!!config.mhstep
                for(int s=0; s<LDA_MH_STEP; s++){           //!!!config.mhstep
                    double nwo,nto,ndo,nwn,ntn,ndn,accept,which_component;
                    // sample from document proposal  
                    if(config.algorithm == lda_config::dp_cgs || config.algorithm==lda_config::cp_cgs) {
                        which_component = random_num[rand_idx++]*(alphaK+doc_token_count);
                        if(which_component < doc_token_count) {
                            size_t rand_token_idx = random_num[rand_idx++] * doc_token_count;
                            new_topic = corpus[doc_idx_].topic_id[rand_token_idx];
                        } else {
                            new_topic = random_num[rand_idx++] * num_topics;
                        }        
                        if(new_topic != old_topic){
                            nwo = model.word_topic_count.val[wtc_offset+old_topic] + beta; 
                            nwn = model.word_topic_count.val[wtc_offset+new_topic] + beta; 
                            nto = model.corpus_topic_count.val[old_topic] + betaW;
                            ntn = model.corpus_topic_count.val[new_topic] + betaW;        
                            accept = 1.0;
                            if(new_topic==topic_id) {
                                ntn -= this->subtractor; nwn -= this->subtractor;
                            } else if(old_topic==topic_id) {    
                                nto -= this->subtractor; nwo -= this->subtractor;
                            }
                            accept = (nwn * nto) / (nwo * ntn);                        
                            if(random_num[rand_idx++] < accept) old_topic = new_topic; 
                        }  
                    }
                    // sample from word proposal
                    if(config.algorithm == lda_config::wp_cgs || config.algorithm ==lda_config::cp_cgs) {
                        which_component = random_num[rand_idx++] * (table_wp.get_norm(word_id)+betaK);  
                        if(which_component < betaK) {                          
                            new_topic = random_num[rand_idx++] * num_topics;
                        } else {
                            size_t idx_s = word_id==0? 0:table_wp.get_num_nonzeros(word_id-1);
                            size_t idx_e = table_wp.get_num_nonzeros(word_id);
                            size_t index = random_num[rand_idx++] * (idx_e - idx_s) + idx_s; 
                            if(random_num[rand_idx++] < table_wp.get_split_val(index)) {
                                new_topic = table_wp.get_bottom_topic(index);
                            } else {
                                new_topic = table_wp.get_top_topic(index);
                            }  
                        }
                        if(new_topic != old_topic){
                            ndo = corpus[doc_idx_].doc_topic_count.val[old_topic] + alpha;
                            ndn = corpus[doc_idx_].doc_topic_count.val[new_topic] + alpha;  
                            nto = model.corpus_topic_count.val[old_topic] + betaW;
                            ntn = model.corpus_topic_count.val[new_topic] + betaW; 
                            accept = 1.0;
                            if(old_topic==topic_id) { 
                                ndo -= 1; nto -= this->subtractor;
                            } else if(new_topic==topic_id) {
                                ndn -= 1; ntn -= this->subtractor;
                            }  
                            accept = (ndn*nto) / (ndo*ntn);
                            if(random_num[rand_idx++] < accept) old_topic = new_topic;                        
                        } 
                    }
                }
                new_topic = old_topic;
                //  ----------------- model update ---------------------- // 
                c1[v*num_topics+new_topic] += this->subtractor;
                if(new_topic != topic_id){
                    corpus[doc_idx_].doc_topic_count.val[topic_id]  --;  
                    corpus[doc_idx_].doc_topic_count.val[new_topic] ++;    
                    corpus[doc_idx_].topic_id[token_idx] = new_topic;
                }
                
                size_t next_idx = token_idx+1;
                if(next_idx == corpus[doc_idx_].num_token()) {
                    next_idx = 0;
                    corpus[doc_idx_].word_index = 0;
                } else if (next_idx > corpus[doc_idx_].get_end_token_index_by_word_index(word_idx)){
                    corpus[doc_idx_].word_index = word_idx + 1;
                }
                corpus[doc_idx_].token_index = next_idx;    
                corpus[doc_idx_].finished_token ++;
                if(corpus[doc_idx_].finished_token==doc_token_count) {     // one doc finish
                    corpus[doc_idx_].finished_token = 0;
                    doc_idx[v] ++; 
                }    
                model_idx[v] = word_id * num_topics + new_topic;
                word_ids[v] = word_id;
            }
        }
// ========================== update model ============================== //        
        if (config.train){
            int model_finish = 0;
            while(model_finish < LDA_CGS_VLEN) {
#if !(defined(_SX) || defined(__ve__))
                for(auto& i:vec_id) i=-1;
#endif  
                model_finish = 0;
#pragma _NEC ivdep
                for(size_t v=0; v<LDA_CGS_VLEN; v++) { 
                    if (model_idx[v]==w_x_k) model_finish ++;
                    else {
#if (defined(_SX) || defined(__ve__))
                        vec_id[word_ids[v]]=v;
#else
                        if(vec_id[word_ids[v]]==-1) vec_id[word_ids[v]]=v;
#endif
                    }
                    if (model_idx[v]!=w_x_k && vec_id[word_ids[v]] == v) {
                        c2[model_idx[v]] ++;
                        model_idx[v] = w_x_k;
                    }
                }
            } 
        }
    }    
    this->min_loop_count = count;
}

template <typename TC, typename TD, typename TW, typename TK, typename TA>
void lda_sampler<TC,TD,TW,TK,TA>::sample_for_doc_par_cgs_delay_update(lda_corpus<TD,TW,TK>& corpus, lda_model<TC>& model, std::vector<TC>& c3, std::vector<TC>& c4){
    double alpha = config.alpha, beta = config.beta, betaW = config.betaW;
    const size_t num_topics = config.num_topics, num_voc = config.num_voc, w_x_k = num_topics * num_voc;
    int finished_par = 0, count = 0;
    std::vector<size_t> doc_idx(LDA_CGS_VLEN);
    std::vector<int> vec_id(num_voc);
    std::vector<double> random_num(GEN_RAND_FREQ);  
    std::vector<double> topic_distribution(num_topics*LDA_CGS_VLEN);  
    double normalization[LDA_CGS_VLEN];   
    TK new_topic[LDA_CGS_VLEN]; 
    TK topic_id[LDA_CGS_VLEN];
    TW word_id[LDA_CGS_VLEN];
    size_t model_idx[LDA_CGS_VLEN];
    size_t offset_1[LDA_CGS_VLEN];
    size_t offset_2[LDA_CGS_VLEN];
#if defined(LDA_CGS_USE_VREG)
#pragma _NEC vreg(normalization)
#pragma _NEC vreg(new_topic)
#pragma _NEC vreg(topic_id)
#pragma _NEC vreg(word_id)
#pragma _NEC vreg(model_idx)
#pragma _NEC vreg(offset_1)
#pragma _NEC vreg(offset_2)
#endif
    for(size_t w=0; w<num_voc; w++) vec_id[w]=-1; 
    
#pragma _NEC ivdep
    for(size_t v=0; v<LDA_CGS_VLEN; v++){ 
        doc_idx[v] = v == 0? 0:this->doc_par_size[v-1];
    }
    
    if(this->itr_count==0 && config.train){
        for (size_t d=0; d<doc_par_size[LDA_CGS_VLEN];d++){    
#pragma _NEC ivdep 
            for(size_t v=0; v<LDA_CGS_VLEN; v++){ 
                size_t doc_id = doc_idx[v] + d;
                size_t doc_last = this->doc_par_size[v];
                if(doc_id < doc_last){
                    size_t word_idx = corpus[doc_id].get_start_word_index_by_par_index(v);
                    if(word_idx==corpus[doc_id].num_word()) word_idx=0;  // no word in current vocabulary partition
                    size_t token_idx = corpus[doc_id].get_start_token_index_by_word_index(word_idx);
                    corpus[doc_id].token_index = token_idx;
                    corpus[doc_id].word_index = word_idx;
                }
            }
        }
    }
        
    while(finished_par != LDA_CGS_VLEN){            
        finished_par = 0;              
//=============================== RNG ===============================//        
        int gen_rand = count % GEN_RAND_FREQ;
        if(gen_rand==0) {
            size_t gen_rand_size = GEN_RAND_FREQ;
            if(this->itr_count!=0) {
                int left_loop = min_loop_count-count;
                if (left_loop < GEN_RAND_FREQ) gen_rand_size = left_loop;
            }
#if (defined(_SX) || defined(__ve__))
            asl_random_generate_d(rng,gen_rand_size,&random_num[0]);
#else  
            for(size_t v=0; v<gen_rand_size; v++) {
                random_num[v] = rand() / static_cast<double>(RAND_MAX);
                random_num[v] = random_num[v]==1? random_num[v]-0.000001:random_num[v];
            }
#endif 
        }
        double random_num_ = random_num[gen_rand];
        count ++;         
       
//=============================== SAMPLING ===============================//  
#pragma _NEC ivdep
        for(size_t v=0; v<LDA_CGS_VLEN; v++) {
            size_t doc_idx_ = doc_idx[v], doc_last = this->doc_par_size[v]; 
            if (doc_idx_ < doc_last) {                
                word_id[v] = corpus[doc_idx_].word_id.idx[corpus[doc_idx_].word_index];
                topic_id[v]= corpus[doc_idx_].topic_id[corpus[doc_idx_].token_index];
                offset_1[v]= v*num_topics;
                offset_2[v]= word_id[v]*num_topics;
                model_idx[v] = 0; 
            } else {
                finished_par ++;
                model_idx[v] = w_x_k;
            }
            normalization[v] = 0; 
            new_topic[v] = num_topics;
        }
        for (size_t t=0; t<num_topics; t++){
            for(size_t v=0; v<LDA_CGS_VLEN; v++) {       
                int not_finish = model_idx[v]!=w_x_k;
                if(not_finish){     
                //  -------------------- sampling ------------------------ //  
                    size_t idx1 = offset_1[v]+t;           
                    double word_topic_count = model.word_topic_count.val[offset_2[v]+t] + beta; 
                    double topic_count = model.corpus_topic_count.val[t] + betaW;
                    double doc_topic_count  = corpus[doc_idx[v]].doc_topic_count.val[t] + alpha;
                    if(t == topic_id[v]) {
                        word_topic_count -= this->subtractor;
                        topic_count -= this->subtractor;
                        doc_topic_count -= this->subtractor;
                    }
                    topic_distribution[idx1] = word_topic_count * doc_topic_count / topic_count;                    
                    normalization[v] += topic_distribution[idx1];
                } 
            }
        }                
        for(size_t v=0; v<LDA_CGS_VLEN; v++) {
            normalization[v] *= random_num_;
        }
        for (size_t t=0; t<num_topics; t++){  
            for(size_t v=0; v<LDA_CGS_VLEN; v++) {
                if(model_idx[v] != w_x_k){         
                    normalization[v] -= topic_distribution[offset_1[v]+t];   
                    if(normalization[v]<0 && t<new_topic[v]) new_topic[v] = t;                   
                }
            }
        }
#pragma _NEC ivdep
        for(size_t v=0; v<LDA_CGS_VLEN; v++) {
            int not_finish = model_idx[v]!=w_x_k;
            if(not_finish){  
                if(new_topic[v]==num_topics) new_topic[v]-=1;       
                size_t doc_idx_= doc_idx[v];
                size_t next_idx_ = corpus[doc_idx_].token_index; 
                c3[offset_1[v] + new_topic[v]] += this->subtractor;
                model_idx[v] = offset_2[v] + new_topic[v];
                if(new_topic[v] != topic_id[v]){
                    corpus[doc_idx_].doc_topic_count.val[topic_id[v]]  --;  
                    corpus[doc_idx_].doc_topic_count.val[new_topic[v]] ++;    
                    corpus[doc_idx_].topic_id[next_idx_] = new_topic[v];
                }
                TD doc_token_count = corpus[doc_idx_].num_token();
                next_idx_ = next_idx_ + 1;
                if(next_idx_ == doc_token_count){
                    next_idx_ = 0;
                    corpus[doc_idx_].word_index = 0;
                } else if (next_idx_ > corpus[doc_idx_].get_end_token_index_by_word_index(corpus[doc_idx_].word_index)){
                    corpus[doc_idx_].word_index ++;
                }
                corpus[doc_idx_].token_index = next_idx_;    
                corpus[doc_idx_].finished_token ++;
                if(corpus[doc_idx_].finished_token == doc_token_count) {
                    corpus[doc_idx_].finished_token = 0;
                    doc_idx[v] ++;
                }
                this->token_left[v]--;          
            }   
        } 
// ========================== update model ============================== // 
        if (config.train){
            int model_finish = 0;
            while(model_finish < LDA_CGS_VLEN) {   
#if !(defined(_SX) || defined(__ve__))
                for(auto& i:vec_id) i=-1;
#endif  
                model_finish = 0;
#pragma _NEC ivdep
                for(size_t v=0; v<LDA_CGS_VLEN; v++) { 
                    if (model_idx[v]==w_x_k) model_finish ++;
                    else {
#if (defined(_SX) || defined(__ve__))
                        vec_id[word_id[v]]=v;
#else
                        if(vec_id[word_id[v]]==-1) vec_id[word_id[v]]=v;
#endif
                    }
                    if (model_idx[v]!=w_x_k && vec_id[word_id[v]] == v) {
                        c4[model_idx[v]] ++;
                        model_idx[v] = w_x_k;
                    }
                }
            } 
        }    
    }    
    this->min_loop_count = count;
}

template <typename TC, typename TD, typename TW, typename TK, typename TA>
void lda_sampler<TC,TD,TW,TK,TA>::sample_for_corpus_unvec(lda_corpus<TD,TW,TK>& corpus, lda_model<TC>& model_g, lda_model<TC>& model_l){
    if(config.algorithm==lda_config::wp_cgs||config.algorithm==lda_config::cp_cgs) {
        if(config.train || itr_count==0) {
            this->table_wp.gen_wp_table_sparse(model_g,config);            
            if(this->itr_count % SHRINK_FREQ == 0) {
                this->table_wp.shrink_table(config.num_voc);
            }
        }
    }    
    if(config.algorithm==lda_config::sparse_cgs) this->cal_distribution_1(model_g);    
    for(size_t i=0; i<corpus.size(); i++) {
        sample_for_doc(corpus,model_g,model_l,i);
    }      
    itr_count++;   
}

template <typename TC, typename TD, typename TW, typename TK, typename TA>
void lda_sampler<TC,TD,TW,TK,TA>::sample_for_doc(lda_corpus<TD,TW,TK>& corpus, lda_model<TC>& model_g, lda_model<TC>& model_l, size_t& doc_id){
    TK new_topic = config.num_topics, topic_id = -1;
    TW word_id = -1;    
    if(config.algorithm==lda_config::sparse_cgs) this->cal_distribution_2(corpus[doc_id]);    
    size_t word_idx = 0;
    for (size_t i=0; i<corpus[doc_id].num_token(); i++){
        if(i > corpus[doc_id].get_end_token_index_by_word_index(word_idx)) word_idx ++;
        TW curr_word = corpus[doc_id].word_id.idx[word_idx];
        if(curr_word!=word_id){
            word_id = curr_word;
            if(config.algorithm==lda_config::sparse_cgs) this->cal_distribution_3(corpus[doc_id],model_g,word_id);
        }
        topic_id = corpus[doc_id].topic_id[i];
        
        // minus one from previous entry
        if (config.train) {   
            model_g.corpus_topic_count.val[topic_id]-=1;
            model_g.word_topic_count.val[word_id*config.num_topics+topic_id]-=1;
            if(config.algorithm==lda_config::sparse_cgs){
                this->coefficient[topic_id] = 1/(model_g.corpus_topic_count.val[topic_id]+config.betaW);
                this->dist1[config.num_topics] -= this->dist1[topic_id];
                this->dist1[topic_id] = config.alpha*config.beta*coefficient[topic_id];
                this->dist1[config.num_topics] += this->dist1[topic_id];
            }
        }   
        corpus[doc_id].doc_topic_count.val[topic_id] -= 1;
        if(config.algorithm==lda_config::sparse_cgs){
            this->dist2[config.num_topics] -= this->dist2[topic_id];
            this->dist2[topic_id] = config.beta * corpus[doc_id].doc_topic_count.val[topic_id] * coefficient[topic_id];
            this->dist2[config.num_topics] += this->dist2[topic_id];
            for (size_t t = 0; t < dist3.size(); ++t) {
                if (topic_id == dist3[t].first) {
                    if(model_g.word_topic_count.val[word_id*config.num_topics+topic_id]!=0)
                        dist3[t].second = model_g.word_topic_count.val[word_id*config.num_topics+topic_id] 
                            * (config.alpha + corpus[doc_id].doc_topic_count.val[topic_id]) * coefficient[topic_id];
                    else {
                        dist3[t].second = 0;
                        dist3[t].swap(dist3.back());
                        dist3.pop_back();
                    }
                }
            } dist3.shrink_to_fit();  
        }        
        // sampling using different algorithms
        if(config.algorithm==lda_config::original_cgs) new_topic = sample_for_word_cgs(corpus[doc_id],model_g,word_id);
        else if(config.algorithm==lda_config::sparse_cgs) new_topic = sample_for_word_sparse();
        else new_topic = sample_for_word_mh(corpus[doc_id],model_g,word_id,topic_id);    
        
        // plus one to the new entry 
        if (config.train) {   
            model_g.corpus_topic_count.val[new_topic]+=1;
            model_g.word_topic_count.val[word_id*config.num_topics+new_topic]+=1;
            model_l.corpus_topic_count.val[new_topic]+=1;
            model_l.word_topic_count.val[word_id*config.num_topics+new_topic]+=1;
            if(config.algorithm==lda_config::sparse_cgs){
                this->coefficient[new_topic] = 1/(model_g.corpus_topic_count.val[new_topic]+config.betaW);
                this->dist1[config.num_topics] -= this->dist1[new_topic];
                this->dist1[new_topic] = config.alpha*config.beta*coefficient[new_topic];
                this->dist1[config.num_topics] += this->dist1[new_topic];
            }
        }  
        corpus[doc_id].doc_topic_count.val[new_topic] ++;
        corpus[doc_id].topic_id[i] = new_topic;
        if(config.algorithm==lda_config::sparse_cgs){
            this->dist2[config.num_topics] -= this->dist2[new_topic];
            this->dist2[new_topic] = config.beta * corpus[doc_id].doc_topic_count.val[new_topic] * coefficient[new_topic];
            this->dist2[config.num_topics] += this->dist2[new_topic];
            for (size_t t = 0; t < dist3.size(); ++t) {
                if (topic_id == dist3[t].first) {
                    if(model_g.word_topic_count.val[word_id*config.num_topics+topic_id]!=0)
                        dist3[t].second = model_g.word_topic_count.val[word_id*config.num_topics+topic_id] 
                            * (config.alpha + corpus[doc_id].doc_topic_count.val[topic_id]) * coefficient[topic_id];
                    else {
                        dist3[t].second = 0;
                        dist3[t].swap(dist3.back());
                        dist3.pop_back();
                    }
                }
            } dist3.shrink_to_fit();
        }
    }    
}

template <typename TC, typename TD, typename TW, typename TK, typename TA>
TK lda_sampler<TC,TD,TW,TK,TA>::sample_for_word_cgs(lda_document<TD,TW,TK>& doc, lda_model<TC>& model, TW& word_id){
    TK new_topic; double normalization = 0;  
    double random_num = rand() / (static_cast<double>(RAND_MAX) + 0.000001);
    std::vector<double> topic_distribution(config.num_topics);    
#pragma _NEC ivdep 
    for (size_t t=0; t<config.num_topics; t++){
        auto word_topic_count = model.word_topic_count.val[word_id*config.num_topics+t]; 
        auto topic_count = model.corpus_topic_count.val[t];
        auto doc_topic_count = doc.doc_topic_count.val[t];
        topic_distribution[t] = (word_topic_count + config.beta) * (doc_topic_count + config.alpha) / (topic_count + config.betaW);
    }  
    for (size_t t=0; t<config.num_topics; t++){
        normalization += topic_distribution[t];
    }  
    random_num *= normalization;    
    for(new_topic = -1; random_num > 0; random_num-=topic_distribution[++new_topic]);
    return new_topic;
}

template <typename TC, typename TD, typename TW, typename TK, typename TA>
TK lda_sampler<TC,TD,TW,TK,TA>::sample_for_word_mh(lda_document<TD,TW,TK>& doc, lda_model<TC>& model, TW& word_id, TK& topic_id){
    TK new_topic = 0, old_topic = topic_id, num_topics = config.num_topics;        
    TD token_count = doc.num_token();
    int mhstep_ = LDA_MH_STEP;
    double betaK = config.beta * num_topics;
    if(config.algorithm == lda_config::cp_cgs) mhstep_ *= 2;
    for(int s=0; s<mhstep_; s++){         
        double rand_num_0 = rand() / (static_cast<double>(RAND_MAX) + 0.000001);
        double rand_num_1 = rand() / (static_cast<double>(RAND_MAX) + 0.000001);
        double rand_num_2 = rand() / (static_cast<double>(RAND_MAX) + 0.000001);
        double rand_num_3 = rand() / (static_cast<double>(RAND_MAX) + 0.000001);
        if(config.algorithm==lda_config::dp_cgs || (config.algorithm==lda_config::cp_cgs && s%2==0)){
            // sample from document proposal
            if(rand_num_1*(token_count+config.alphaK) < token_count) {
                size_t rand_idx = rand_num_2 * token_count;
                new_topic = doc.topic_id[rand_idx];
            } else {
                new_topic = rand_num_2 * num_topics;
            } 
            //MH algorithm
            if(new_topic!=old_topic){                   
                double nto,ntn,nwo,nwn,accept;
                ntn = model.corpus_topic_count.val[new_topic] + config.betaW;
                nto = model.corpus_topic_count.val[old_topic] + config.betaW;
                nwo = model.word_topic_count.val[word_id*num_topics+old_topic]+config.beta; 
                nwn = model.word_topic_count.val[word_id*num_topics+new_topic]+config.beta; 
                accept = (nwn*nto) / (nwo*ntn);  
                if(rand_num_3 < accept) old_topic = new_topic;   
            } 
        } else {            
            //sample from word proposal                       
            double which_component = rand_num_0 * (table_wp.get_norm(word_id)+betaK);  
            if(which_component < betaK) {                          
                new_topic = rand_num_1 * num_topics;
            } else {
                size_t idx_s = word_id==0? 0:table_wp.get_num_nonzeros(word_id-1);
                size_t idx_e = table_wp.get_num_nonzeros(word_id);
                size_t index = rand_num_1 * (idx_e - idx_s) + idx_s; 
                if(rand_num_2 < table_wp.get_split_val(index)) {
                    new_topic = table_wp.get_bottom_topic(index);
                } else {
                    new_topic = table_wp.get_top_topic(index);
                }  
            }
            //MH algorithm
            if(new_topic!=old_topic){
                double nto,ndo,ntn,ndn,accept;
                ndo = doc.doc_topic_count.val[old_topic] + config.alpha;   
                ndn = doc.doc_topic_count.val[new_topic] + config.alpha;; 
                ntn = model.corpus_topic_count.val[new_topic] + config.betaW;
                nto = model.corpus_topic_count.val[old_topic] + config.betaW;
                accept = ndn * nto / (ndo * ntn);   
                if(rand_num_3 < accept) old_topic = new_topic;     
            }     
        }
    }
    new_topic = old_topic;
    return new_topic;
}

template <typename TC, typename TD, typename TW, typename TK, typename TA>
TK lda_sampler<TC,TD,TW,TK,TA>::sample_for_word_sparse(){
    TK new_topic = config.num_topics;    
    double rand_num = rand() / (static_cast<double>(RAND_MAX) + 0.000001);        
    double n1 = this->dist1[config.num_topics];
    double n2 = this->dist2[config.num_topics];
    double n3 = 0;     
    for(size_t t=0; t<dist3.size(); t++)  n3 += dist3[t].second;   
    rand_num *= (n1+n2+n3);            
    if(rand_num<n3) {
        for(new_topic = -1; rand_num > 0; rand_num -= dist3[++new_topic].second);
        new_topic = dist3[new_topic].first;
    } else if(rand_num<(n3+n2)) {        
        rand_num -= n3;
        for (new_topic = -1; rand_num > 0; rand_num -= dist2[++new_topic]);        
    } else {  
        rand_num -= n3 + n2;
        for (new_topic = -1; rand_num > 0; rand_num -= dist1[++new_topic]);  
    }
    return new_topic;
}
   
template <typename TC, typename TD, typename TW, typename TK, typename TA>
void lda_sampler<TC,TD,TW,TK,TA>::cal_distribution_1(lda_model<TC>& model){
    for(auto& i:coefficient) i=0;
    for(auto& i:dist1) i=0;    
    for(size_t t=0; t<config.num_topics; t++){
        coefficient[t] = 1/(model.corpus_topic_count.val[t]+config.betaW);
        dist1[t] = config.alpha*config.beta*coefficient[t];
        dist1[config.num_topics] += dist1[t];
    } 
}

template <typename TC, typename TD, typename TW, typename TK, typename TA>
void lda_sampler<TC,TD,TW,TK,TA>::cal_distribution_2(lda_document<TD,TW,TK>& doc){
    for(auto& i:dist2) i=0;
    for(size_t t=0; t<config.num_topics; t++){
        if(doc.doc_topic_count.val[t]!=0) {
            dist2[t] = config.beta * doc.doc_topic_count.val[t] * coefficient[t];
            dist2[config.num_topics] += dist2[t];
        }
    }
}

template <typename TC, typename TD, typename TW, typename TK, typename TA>
void lda_sampler<TC,TD,TW,TK,TA>::cal_distribution_3(lda_document<TD,TW,TK>& doc, lda_model<TC>& model, TW& word_id){
    dist3.clear();
    for(size_t t=0; t<config.num_topics; t++){
        if(model.word_topic_count.val[word_id*config.num_topics+t]!=0) {
            double prob = model.word_topic_count.val[word_id*config.num_topics+t] * (config.alpha + doc.doc_topic_count.val[t]) * coefficient[t];
            dist3.push_back(std::make_pair(t,prob));
        }
    }
}

inline double log_gamma(double xx){
    double cof[6] = 
    {   76.18009172947146, -86.50532032941677,
        24.01409824083091, -1.231739572450155,
        0.1208650973866179e-2, -0.5395239384953e-5};
    double x, y, tmp1, ser;
    y = xx;
    x = xx;
    tmp1 = x + 5.5;
    tmp1 -= (x + 0.5)*log(tmp1);
    ser = 1.000000000190015;
#pragma _NEC unroll(6)
    for (int j = 0; j < 6; j++) ser += cof[j] / ++y;
    return -tmp1 + log(2.5066282746310005*ser / x);
}

template <typename TD = int32_t, typename TW = int32_t, typename TK = int32_t>
double cal_doc_likelihood(lda_corpus<TD,TW,TK>& corpus, lda_config& config){
    double tmp_lld = 0; 
    double lg_alpha = log_gamma(config.alpha);
    double lg_alphaK = log_gamma(config.alphaK);
    for (size_t d=0; d<corpus.size(); d++){
        for(size_t t=0; t<config.num_topics; t++){
            tmp_lld += log_gamma(config.alpha+corpus[d].doc_topic_count.val[t]) - lg_alpha;
        }
    }
    for (size_t d=0; d<corpus.size(); d++){
        tmp_lld += lg_alphaK - log_gamma(config.alphaK+corpus[d].num_token());   
    }  
    return tmp_lld;
}

template <typename TC = int32_t>
double cal_word_likelihood(lda_model<TC>& model, lda_config& config){
    int rank = get_selfid();
    size_t size = get_nodesize();           
    size_t num_topic_local = (config.num_topics-1)/size+1;    
    size_t ts = rank*num_topic_local;
    size_t te = rank*num_topic_local+num_topic_local-1;
    if(rank==size-1) te = config.num_topics-1;      
    double tmp_lld = 0;
    double lg_beta = log_gamma(config.beta);
    double lg_betaW= log_gamma(config.betaW);
    for(size_t t=ts; t<=te; t++){
        for(size_t w=0; w<config.num_voc; w++){
            tmp_lld += log_gamma(config.beta+model.word_topic_count.val[w*config.num_topics+t]) - lg_beta;
        }
    }
    for(size_t t=ts; t<=te; t++){
        tmp_lld += lg_betaW - log_gamma(config.betaW+model.corpus_topic_count.val[t]);
    }
    return tmp_lld;
}

template <typename TC, typename TD = int32_t, typename TW = int32_t, typename TK = int32_t>
double cal_perplexity(lda_corpus<TD,TW,TK>& corpus, lda_model<TC>& model, lda_config& config){
    double likelihood = 0;
    size_t num_topics = config.num_topics;
    std::vector<double> prob; prob.resize(config.max_token); 
    std::vector<double> prob_doc; prob_doc.resize(num_topics); 
    
    for(size_t d=0; d<corpus.size(); d++){
        for(size_t t=0 ; t<num_topics; t++) {
            prob_doc[t] = (corpus[d].doc_topic_count.val[t]+config.alpha) 
                        / (corpus[d].num_token()+config.alphaK)
                        / (model.corpus_topic_count.val[t]+config.betaW);
        }  
        TW word_id_ = -1;
        size_t word_idx = 0;
        for (size_t i=0; i<corpus[d].num_token(); i++){
            if(i > corpus[d].get_end_token_index_by_word_index(word_idx)) word_idx++;
            TW curr_word = corpus[d].word_id.idx[word_idx];       
            if (curr_word == word_id_) prob[i] = prob[i-1];     
            else {
                prob[i]  = 0;
                word_id_ = curr_word;
                for(size_t t=0 ; t<num_topics; t++){
                    TC word_topic_count_ = model.word_topic_count.val[word_id_*num_topics+t];
                    double prob_ = (word_topic_count_+config.beta) * prob_doc[t];
                    prob[i] += prob_;
                }                
            }
        }
        for(size_t i=0; i<corpus[d].num_token(); i++){
            double prob_ = log(prob[i]);
            likelihood += prob_;
        }
    }   
    return likelihood;
}

template <typename TC, typename TD, typename TW, typename TK, typename TA>
void lda_sampler<TC,TD,TW,TK,TA>::initialize(lda_corpus<TD,TW,TK>& corpus, std::vector<TC>& local_token_count, lda_config& config){
    this->config = config;
    size_t num_topics = this->config.num_topics;
#if (defined(_SX) || defined(__ve__))
    this->set_voc_par_size(corpus,local_token_count[get_selfid()]);
    this->set_doc_par_size(corpus,local_token_count[get_selfid()]); 
    this->opt_seq.resize(LDA_CGS_VLEN*PRIO_MAX);
    for(size_t i=0; i<LDA_CGS_VLEN; i++) this->opt_seq[i] = i;
    min_loop_count = 0;
    explore = true;
    opt_prio_step = PRIO_STEP;
    if(this->config.delay_update) ct_count_v.assign(LDA_CGS_VLEN*num_topics,0);
    else ct_count_v.assign(LDA_CGS_VLEN*num_topics*2,0); 
    this->initialize_asl_rng();
#endif
    this->itr_count = 0;
    if(this->config.algorithm==lda_config::sparse_cgs){
        this->coefficient.resize(num_topics);
        this->dist1.resize(num_topics+1);
        this->dist2.resize(num_topics+1);
        this->dist3.clear();
    }
    this->table_wp.initialize(this->config);
    subtractor = this->config.train? 1:0;
}

template <typename TC, typename TD, typename TW, typename TK, typename TA>
void lda_sampler<TC,TD,TW,TK,TA>::set_doc_par_size(lda_corpus<TD,TW,TK>& corpus, TC& token_count){   
    size_t doc_count = corpus.size(); 
    doc_par_size.clear();
    this->token_left.assign(LDA_CGS_VLEN,0);
    this->token_count.assign(LDA_CGS_VLEN,0);
            
    if (LDA_CGS_VLEN==1) {
        doc_par_size.push_back(doc_count);
        doc_par_size.push_back(doc_count);
    }       
    else {        
        TC num_tok_per_vec = token_count/LDA_CGS_VLEN;
        TD num_tok_doc = 0;
        TC num_tok_now = 0; 
        size_t max_doc = 0;
        int32_t par_id  = 1;
        for(int i=0; i<doc_count; i++){
            TC diff_old = num_tok_now - num_tok_per_vec*par_id;
            diff_old = diff_old<0? -diff_old:diff_old;
            num_tok_doc =  corpus[i].num_token();
            num_tok_now += num_tok_doc;
            TC diff_new = num_tok_now - num_tok_per_vec*par_id;
            diff_new = diff_new<0? -diff_new:diff_new;
            if(diff_new>diff_old && doc_par_size.size()<LDA_CGS_VLEN-1){
                doc_par_size.push_back(i);
                size_t doc_count = doc_par_size.size()>1? (doc_par_size.back()-doc_par_size.at(doc_par_size.size()-2)):doc_par_size.back();
                if(doc_count>max_doc) max_doc = doc_count;      
                par_id++;
            } 
            if (doc_par_size.size()==LDA_CGS_VLEN-1) break;
        }  
        while(doc_par_size.size()<LDA_CGS_VLEN){
            doc_par_size.push_back(doc_count);            
            size_t doc_count = doc_par_size.size()>1? (doc_par_size.back()-doc_par_size.at(doc_par_size.size()-2)):doc_par_size.back();
            if(doc_count>max_doc) max_doc = doc_count;
        }
        doc_par_size.push_back(max_doc);             
        for(size_t d=0; d<doc_par_size[LDA_CGS_VLEN]; d++){
#pragma _NEC ivdep 
            for(size_t v=0; v<LDA_CGS_VLEN; v++){
                size_t ds = 0? 0:this->doc_par_size[v-1];
                size_t de = this->doc_par_size[v]-1;
                if(ds+d<=de) this->token_count[v] += corpus[ds+d].num_token();
            }
        }                        
    }    
    for(size_t i=0; i<LDA_CGS_VLEN; i++){
        this->token_left[i] = this->token_count[i];
    } 
}

template <typename TC, typename TD, typename TW, typename TK, typename TA>
void lda_sampler<TC,TD,TW,TK,TA>::set_voc_par_size(lda_corpus<TD,TW,TK>& corpus, TC& token_count){ 
    size_t voc_size = this->config.num_voc;
    voc_par_size.clear();
    if (LDA_CGS_VLEN == 1) {
        voc_par_size.push_back(voc_size);
    }
    std::vector<TC> count_matrix; count_matrix.assign(voc_size,0);
    for(size_t d=0; d<corpus.size(); d++) {        
        for(size_t i=0; i<corpus[d].num_word(); i++){
            count_matrix[corpus[d].word_id.idx[i]] += corpus[d].get_token_count_by_word_index(i);
        }
    }
    TC num_tok_per_vec = token_count/LDA_CGS_VLEN;
    TC num_tok_voc = 0;
    TC num_tok_now = 0; 
    int par_id = 1;
    for(size_t i=0; i<voc_size; i++){
        TC diff_old = num_tok_now - num_tok_per_vec*par_id;
        diff_old = diff_old<0? -diff_old:diff_old;
        num_tok_voc =  count_matrix[i];
        num_tok_now += num_tok_voc;
        TC diff_new = num_tok_now - num_tok_per_vec*par_id;
        diff_new = diff_new<0? -diff_new:diff_new;
        if (diff_new>diff_old && voc_par_size.size()<LDA_CGS_VLEN-1){
            voc_par_size.push_back(i);
            par_id ++;  
        } 
        if (voc_par_size.size()==LDA_CGS_VLEN-1) break;
    }  
    while(voc_par_size.size()<LDA_CGS_VLEN){
        voc_par_size.push_back(voc_size);            
    }       
    for(size_t d=0; d<corpus.size(); d++){
        corpus[d].set_voc_par_word_idx(this->voc_par_size);
    }    
}

template <typename TC, typename TD, typename TW, typename TK, typename TA>
void lda_sampler<TC,TD,TW,TK,TA>::cal_doc_par_efficiency(lda_corpus<TD,TW,TK>& corpus){    
    std::vector<TC> num_token_par;
    TC max_token_par = 0;
    num_token_par.assign(LDA_CGS_VLEN,0);
    for(size_t dp=0; dp<LDA_CGS_VLEN; dp++){
        size_t ds = dp == 0? 0:this->doc_par_size[dp-1];
        size_t de = this->doc_par_size[dp]-1;
        for (size_t d=ds; d<=de; d++){
            num_token_par[dp]+=corpus[d].num_token();         
        }
    }
    for(size_t v=0; v<LDA_CGS_VLEN; v++){
        if(num_token_par[v]>max_token_par) max_token_par = num_token_par[v];
    }
    RLOG(DEBUG)<<"rank-"<<get_selfid()<<": "<<max_token_par<<std::endl;
}

template <typename TC, typename TD, typename TW, typename TK, typename TA>
void lda_sampler<TC,TD,TW,TK,TA>::cal_doc_voc_par_efficiency(lda_corpus<TD,TW,TK>& corpus){    
    std::vector<TC> num_token_par;
    std::vector<TC> max_token_par;
    num_token_par.assign(LDA_CGS_VLEN*LDA_CGS_VLEN,0);
    max_token_par.assign(LDA_CGS_VLEN,0);
    TC cost = 0;
    for(size_t dp=0; dp<LDA_CGS_VLEN; dp++){
        size_t ds = dp == 0? 0:this->doc_par_size[dp-1];
        size_t de = this->doc_par_size[dp]-1;
        for (size_t d=ds; d<=de; d++){
            for(size_t vp=0; vp<LDA_CGS_VLEN; vp++){
                num_token_par[dp*LDA_CGS_VLEN+vp]+=corpus[d].get_token_count_by_par_index(vp);
            }            
        }
    }
    for(size_t v=0; v<LDA_CGS_VLEN; v++){
        for(size_t dp=0; dp<LDA_CGS_VLEN; dp++){
            size_t vp = (v + dp % LDA_CGS_VLEN) % LDA_CGS_VLEN;
            if(num_token_par[dp*LDA_CGS_VLEN+vp]>max_token_par[v]) max_token_par[v] = num_token_par[dp*LDA_CGS_VLEN+vp];
        }
    }
    for(size_t v=0; v<LDA_CGS_VLEN; v++){
        cost += max_token_par[v];
    }
    RLOG(DEBUG)<<"rank-"<<get_selfid()<<": "<<cost<<std::endl;
}
}
}
#endif /* _LDA_CGS_SAMPLER_HPP_ */

