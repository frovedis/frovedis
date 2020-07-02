
#ifndef _LDA_CGS_ALIAS_TABLE_HPP_
#define _LDA_CGS_ALIAS_TABLE_HPP_

#include <vector>

#include "lda_model.hpp"

namespace frovedis {
namespace lda {

template<typename TC, typename TK = int32_t, typename TA = int32_t>
class alias_table {
    
private:
    std::vector<float> norms;
    std::vector<float> topic_split;
    std::vector<TC> num_nonzeros;
    std::vector<TK> topic_top;
    std::vector<TK> topic_bot;
    SERIALIZE_NONE
    
public:
    alias_table(){
        this->norms.clear();
        this->num_nonzeros.clear();
        this->topic_top.clear();
        this->topic_bot.clear();
        this->topic_split.clear();
    }
    
    ~alias_table() {}

    void initialize (lda_config& config);
    void gen_wp_table_sparse(lda_model<TC>& model, lda_config& config);
    void gather_sparse_table(size_t local_size);
    void shrink_table(size_t num_voc);          
    TK get_top_topic(size_t idx) {return this->topic_top[idx];}    
    TK get_bottom_topic(size_t idx) {return this->topic_bot[idx];} 
    float get_split_val(size_t idx) {return this->topic_split[idx];} 
    float get_norm(size_t idx) {return this->norms[idx];} 
    float get_num_nonzeros(size_t idx) {return this->num_nonzeros[idx];} 
    
};

// when the number of topics do not exceed short int
// used compressed format of alias tables
// bottom and top values stored in the lower and upper 16-bits
template<typename TC>
class alias_table<TC,int32_t,int16_t> {
private:
    std::vector<float> norms;
    std::vector<float> topic_split;
    std::vector<TC> num_nonzeros;
    std::vector<int32_t> topics;
    SERIALIZE_NONE
    
public:   
    alias_table(){
        this->norms.clear();
        this->num_nonzeros.clear();
        this->topics.clear();
        this->topic_split.clear();
    }
    
    ~alias_table() {}
    
    void initialize (lda_config& config);
    void gen_wp_table_sparse(lda_model<TC>& model, lda_config& config);
    void gather_sparse_table(size_t local_size);
    void shrink_table(size_t num_voc);          
    int32_t get_top_topic(size_t idx) {return this->topics[idx] >> 16;}
    int32_t get_bottom_topic(size_t idx) {return this->topics[idx] & 0x0000FFFF;} 
    float get_split_val(size_t idx) {return this->topic_split[idx];} 
    float get_norm(size_t idx) {return this->norms[idx];} 
    float get_num_nonzeros(size_t idx) {return this->num_nonzeros[idx];} 
    
};

template <typename TC, typename TK, typename TA>
void alias_table<TC,TK,TA>::initialize(lda_config& config){   
    size_t num_voc_local = (config.num_voc-1)/get_nodesize()+1;
    size_t size1 = num_voc_local*get_nodesize();
    size_t size2 = num_voc_local*config.num_topics;
    this->norms.resize(size1);
    this->num_nonzeros.resize(size1);
    this->topic_split.resize(size2);
    this->topic_top.resize(size2);
    this->topic_bot.resize(size2);
}

template <typename TC, typename TK, typename TA>
void alias_table<TC,TK,TA>::shrink_table(size_t num_voc){ 
    size_t new_size = this->num_nonzeros[num_voc-1];
    topic_top.resize(new_size);
    topic_bot.resize(new_size);
    topic_split.resize(new_size);
    topic_top.shrink_to_fit();
    topic_bot.shrink_to_fit();
    topic_split.shrink_to_fit();
}

template <typename TC, typename TK, typename TA>
void alias_table<TC,TK,TA>::gather_sparse_table(size_t local_size){
    size_t size = get_nodesize(), total_nonzero = 0;    
    size_t local_voc = norms.size() / size;
    int rank = get_selfid();
    std::vector<int> rev_sizes(size), displ(size); 
    MPI_Allgather(&local_size, 1, MPI_INT, rev_sizes.data(), 1, MPI_INT, frovedis_comm_rpc);  
    displ[0] = 0; total_nonzero += rev_sizes[0];
    for(size_t r = 1; r < size; r++){
        total_nonzero += rev_sizes[r];
        displ[r] = displ[r-1] + rev_sizes[r-1];
    }
    if(rank != 0){
        for(size_t i=0; i<local_voc; i++) num_nonzeros[i] += displ[rank];
    } 
    if(total_nonzero > topic_bot.size()){
        topic_top.resize(total_nonzero);
        topic_bot.resize(total_nonzero);
        topic_split.resize(total_nonzero);
    }
    
    typed_allgather<TC>(num_nonzeros.data(), local_voc, num_nonzeros.data(), local_voc, frovedis_comm_rpc);  
    typed_allgatherv<TK>(topic_top.data(), local_size, topic_top.data(),rev_sizes.data(), displ.data(), frovedis_comm_rpc); 
    typed_allgatherv<TK>(topic_bot.data(), local_size, topic_bot.data(),rev_sizes.data(), displ.data(), frovedis_comm_rpc); 
    MPI_Allgather(norms.data(), local_voc, MPI_FLOAT, norms.data(), local_voc, MPI_FLOAT, frovedis_comm_rpc);
    MPI_Allgatherv(topic_split.data(), local_size, MPI_FLOAT,topic_split.data(),rev_sizes.data(), displ.data(),MPI_FLOAT, frovedis_comm_rpc);
}

template <typename TC, typename TK, typename TA>
void alias_table<TC,TK,TA>::gen_wp_table_sparse(lda_model<TC>& model, lda_config& config){   
    size_t rank = get_selfid(), size = get_nodesize();    
    size_t num_topics = config.num_topics, num_voc = config.num_voc;    
    size_t num_voc_local = (num_voc-1)/size+1;  
    size_t ws = rank * num_voc_local;
    size_t we = ws + num_voc_local - 1;
    if(rank == size-1)  {
        we = num_voc - 1;   
    }
    
#if !(defined(_SX) || defined(__ve__)) 
    for(size_t w=ws; w<=we; w++){
        std::vector<TK> l_val;
        std::vector<TK> h_val;
        std::vector<float> l_pro;
        std::vector<float> h_pro;
        std::vector<float> distribution(num_topics);
        distribution.assign(num_topics,0);
        std::vector<TK> nonzeros_topic(num_topics);
        nonzeros_topic.assign(num_topics,-1);
        size_t nonzeros = 0;
        float normalization = 0.0;
        for(size_t t=0; t<num_topics; t++){ 
            auto word_topic_count = model.word_topic_count.val[w*num_topics+t]; 
            if(word_topic_count!=0) {
                distribution[nonzeros] = word_topic_count;
                nonzeros_topic[nonzeros] = t;
                normalization += distribution[nonzeros];
                nonzeros ++;
            }
        }            
        float one_over_topic = 1/(float)nonzeros;            
        this->norms[w-ws] = normalization;
        for(size_t t=0; t<nonzeros; t++){
            distribution[t] /= normalization;
            if(distribution[t] <= one_over_topic){
                l_val.push_back(nonzeros_topic[t]);
                l_pro.push_back(distribution[t]);
            } else {
                h_val.push_back(nonzeros_topic[t]);
                h_pro.push_back(distribution[t]);            
            }
        }
        size_t curr_idx = 0;
        this->num_nonzeros[w-ws] = nonzeros; 
        if(w != ws) {
            curr_idx = this->num_nonzeros[w-ws-1];
            this->num_nonzeros[w-ws] += curr_idx;
        }            
        size_t required_size = num_nonzeros[w-ws];
        if(required_size > topic_top.size()){
            topic_top.resize(required_size);
            topic_bot.resize(required_size);
            topic_split.resize(required_size);
        }
        while (!l_val.empty()){   
            TK l=l_val.back(); l_val.pop_back();
            float lp=l_pro.back(); l_pro.pop_back();
            if(!h_val.empty()){
                TK h=h_val.back(); h_val.pop_back();
                float hp=h_pro.back(); h_pro.pop_back();
                this->topic_bot[curr_idx] = l;
                this->topic_top[curr_idx] = h;
                float hpn = hp-(one_over_topic-lp);
                if(hpn < one_over_topic || l_val.empty()) {
                    l_val.push_back(h);
                    l_pro.push_back(hpn);
                } else {
                    h_val.push_back(h);
                    h_pro.push_back(hpn);            
                } 
            } else {
                this->topic_bot[curr_idx] = l;
                this->topic_top[curr_idx] = l;
            }
            this->topic_split[curr_idx] = lp*nonzeros;
            curr_idx ++;
        }
    }    
#else      
    size_t k_x_v = num_topics*LDA_CGS_VLEN ,i_x_v = 0;
    size_t loop_count = LDA_CGS_VLEN;  
    size_t sub_count = (num_voc_local-1)/LDA_CGS_VLEN + 1;   // avoid too much temporary memory requirement  
    std::vector<TK> l_val(k_x_v);
    std::vector<TK> h_val(k_x_v);
    std::vector<float> l_pro(k_x_v);
    std::vector<float> h_pro(k_x_v);
    std::vector<float> distribution(k_x_v);
    std::vector<TK> nonzeros_topic(k_x_v);
    size_t idx_off[LDA_CGS_VLEN];
    TK nonzeros[LDA_CGS_VLEN];
    TK l_last[LDA_CGS_VLEN];
    TK h_last[LDA_CGS_VLEN];
    float one_over_topic[LDA_CGS_VLEN];
    float normalization[LDA_CGS_VLEN];
#if defined(LDA_CGS_USE_VREG)
#pragma _NEC vreg(idx_off)
#pragma _NEC vreg(nonzeros)
#pragma _NEC vreg(l_last)
#pragma _NEC vreg(h_last)
#pragma _NEC vreg(one_over_topic)
#pragma _NEC vreg(normalization)
#endif
    for(size_t w=0; w<LDA_CGS_VLEN; w++){
        l_last[w] = -1;
        h_last[w] = -1;
        idx_off[w]= w*num_topics;
    }    

    for(size_t i=0;i<sub_count;i++){
        if (i == sub_count - 1) loop_count = (we-ws+1) % LDA_CGS_VLEN;
        if (loop_count == 0) loop_count = LDA_CGS_VLEN;
#pragma _NEC shortloop
        for(size_t w=0; w<loop_count; w++){  
            nonzeros[w] = 0;
            normalization[w] = 0;
        }  
        for(size_t w=0; w<loop_count*num_topics; w++){  
            distribution[w] = 0;
            nonzeros_topic[w] = -1;
        }           
        TK max_nonzero = -1;     
        for(size_t t=0; t<num_topics; t++){
#pragma _NEC ivdep
#pragma _NEC shortloop
            for(size_t w=0; w<loop_count; w++){
                size_t word_id = ws+i_x_v+w;
                auto word_topic_count = model.word_topic_count.val[word_id*num_topics+t]; 
                if(word_topic_count!=0) {
                    size_t idx = idx_off[w] + nonzeros[w];
                    nonzeros_topic[idx] = t;
                    nonzeros[w]++;
                    distribution[idx] = word_topic_count;
                    normalization[w] += distribution[idx];
                } 
            }
        }    
#pragma _NEC shortloop
        for(size_t w=0; w<loop_count; w++){
            if(max_nonzero<nonzeros[w]) max_nonzero=nonzeros[w];
            size_t idx = i_x_v+w;
            norms[idx] = normalization[w];
            num_nonzeros[idx] = nonzeros[w]; 
            one_over_topic[w] = 1/(float)nonzeros[w];
            nonzeros[w]=0;
        }     
        if(i!=0) num_nonzeros[i_x_v] += num_nonzeros[i_x_v-1];
        for(size_t w=i_x_v+1; w<i_x_v+loop_count; w++){
            num_nonzeros[w] += num_nonzeros[w-1];
        }   
        size_t required_size = num_nonzeros[i_x_v+loop_count-1];
        if(required_size > topic_top.size()){
            topic_top.resize(required_size);
            topic_bot.resize(required_size);
            topic_split.resize(required_size);
        }
        for(size_t t=0; t<max_nonzero; t++){
#pragma _NEC ivdep
#pragma _NEC shortloop
            for(size_t w=0; w<loop_count; w++){
                size_t idx1 = idx_off[w] + t, idx2; 
                TK topic_id = nonzeros_topic[idx1];
                float one_over_topic_ = one_over_topic[w];
                if(topic_id!=-1){
                    distribution[idx1] /= normalization[w];
                    if(distribution[idx1] <= one_over_topic_){
                        l_last[w]++; idx2 = idx_off[w] + l_last[w]; 
                        l_val[idx2] = topic_id;
                        l_pro[idx2] = distribution[idx1];
                    } else {
                        h_last[w]++; idx2 = idx_off[w] + h_last[w]; 
                        h_val[idx2] = topic_id;
                        h_pro[idx2] = distribution[idx1];        
                    }
                }
            }    
        }       
        for(size_t t=0; t<max_nonzero; t++){
#pragma _NEC ivdep
#pragma _NEC shortloop
            for(size_t w=0; w<loop_count; w++){
                size_t word_idx = i_x_v+w;
                size_t curr_nonzero = word_idx==0? 0:num_nonzeros[word_idx-1];
                int empty_l = l_last[w]==-1? 1:0;
                int empty_h = h_last[w]==-1? 1:0;
                size_t idx1 = curr_nonzero + nonzeros[w], idx2;
                if(!empty_l){
                    float one_over_topic_ = one_over_topic[w];
                    idx2 = idx_off[w] + l_last[w];
                    TK l=l_val[idx2]; float lp=l_pro[idx2]; l_last[w]--;
                    if(!empty_h){
                        idx2 = idx_off[w] + h_last[w];
                        TK h=h_val[idx2]; float hp=h_pro[idx2]; h_last[w]--;
                        this->topic_bot[idx1] = l;
                        this->topic_top[idx1] = h;
                        float hpn = hp-(one_over_topic_-lp);
                        if(hpn <= one_over_topic_ || l_last[w]==-1) {
                            l_last[w]++; idx2 = idx_off[w] + l_last[w];
                            l_val[idx2]=h; l_pro[idx2]=hpn;
                        } else {
                            h_last[w]++; idx2 = idx_off[w] + h_last[w];
                            h_val[idx2]=h; h_pro[idx2]=hpn;   
                        } 
                    } else {
                        this->topic_bot[idx1] = l;
                        this->topic_top[idx1] = l;
                    }
                    this->topic_split[idx1] = lp/one_over_topic_;
                    nonzeros[w]++;
                }
            } 
        } 
        i_x_v += LDA_CGS_VLEN;
    }   
#endif
    
    size_t local_nonzero = num_nonzeros[we-ws];
    if(size>1) {
        this->gather_sparse_table(local_nonzero); 
    }
}

template <typename TC>
void alias_table<TC,int32_t,int16_t>::initialize(lda_config& config){   
    size_t num_voc_local = (config.num_voc-1)/get_nodesize()+1;
    size_t size1 = num_voc_local*get_nodesize();
    size_t size2 = num_voc_local*config.num_topics;
    this->norms.resize(size1);
    this->num_nonzeros.resize(size1);
    this->topic_split.resize(size2);
    this->topics.resize(size2);
}

template <typename TC>
void alias_table<TC,int32_t,int16_t>::shrink_table(size_t num_voc){ 
    size_t new_size = this->num_nonzeros[num_voc-1];
    topics.resize(new_size);
    topic_split.resize(new_size);
    topics.shrink_to_fit();
    topic_split.shrink_to_fit();
}

template <typename TC>
void alias_table<TC,int32_t,int16_t>::gather_sparse_table(size_t local_size){
    size_t size = get_nodesize(), total_nonzero = 0;    
    size_t local_voc = norms.size() / size;
    int rank = get_selfid();
    std::vector<int> rev_sizes(size), displ(size); 
    MPI_Allgather(&local_size, 1, MPI_INT, rev_sizes.data(), 1, MPI_INT, frovedis_comm_rpc);  
    displ[0] = 0; total_nonzero += rev_sizes[0];
    for(size_t r = 1; r < size; r++){
        total_nonzero += rev_sizes[r];
        displ[r] = displ[r-1] + rev_sizes[r-1];
    }
    if(rank != 0){
        for(size_t i=0; i<local_voc; i++) num_nonzeros[i] += displ[rank];
    } 
    if(total_nonzero > topics.size()){
        topics.resize(total_nonzero);
        topic_split.resize(total_nonzero);
    }
    
    typed_allgather<TC>(num_nonzeros.data(), local_voc, num_nonzeros.data(), local_voc, frovedis_comm_rpc);  
    typed_allgatherv<int32_t>(topics.data(), local_size, topics.data(),rev_sizes.data(), displ.data(), frovedis_comm_rpc); 
    MPI_Allgather(norms.data(), local_voc, MPI_FLOAT, norms.data(), local_voc, MPI_FLOAT, frovedis_comm_rpc);
    MPI_Allgatherv(topic_split.data(), local_size, MPI_FLOAT,topic_split.data(),rev_sizes.data(), displ.data(),MPI_FLOAT, frovedis_comm_rpc);
}

template <typename TC>
void alias_table<TC,int32_t,int16_t>::gen_wp_table_sparse(lda_model<TC>& model, lda_config& config){   
    size_t rank = get_selfid(), size = get_nodesize();    
    size_t num_topics = config.num_topics, num_voc = config.num_voc;    
    size_t num_voc_local = (num_voc-1)/size+1;  
    size_t ws = rank * num_voc_local;
    size_t we = ws + num_voc_local - 1;
    if(rank == size-1)  {
        we = num_voc - 1;   
    }
    
#if !(defined(_SX) || defined(__ve__)) 
    for(size_t w=ws; w<=we; w++){
        std::vector<int32_t> l_val;
        std::vector<int32_t> h_val;
        std::vector<float> l_pro;
        std::vector<float> h_pro;
        std::vector<float> distribution(num_topics);
        distribution.assign(num_topics,0);
        std::vector<int32_t> nonzeros_topic(num_topics);
        nonzeros_topic.assign(num_topics,-1);
        size_t nonzeros = 0;
        float normalization = 0.0;
        for(size_t t=0; t<num_topics; t++){ 
            auto word_topic_count = model.word_topic_count.val[w*num_topics+t]; 
            if(word_topic_count!=0) {
                distribution[nonzeros] = word_topic_count;
                nonzeros_topic[nonzeros] = t;
                normalization += distribution[nonzeros];
                nonzeros ++;
            }
        }            
        float one_over_topic = 1/(float)nonzeros;            
        this->norms[w-ws] = normalization;
        for(size_t t=0; t<nonzeros; t++){
            distribution[t] /= normalization;
            if(distribution[t] <= one_over_topic){
                l_val.push_back(nonzeros_topic[t]);
                l_pro.push_back(distribution[t]);
            } else {
                h_val.push_back(nonzeros_topic[t]);
                h_pro.push_back(distribution[t]);            
            }
        }
        size_t curr_idx = 0;
        this->num_nonzeros[w-ws] = nonzeros; 
        if(w != ws) {
            curr_idx = this->num_nonzeros[w-ws-1];
            this->num_nonzeros[w-ws] += curr_idx;
        }            
        size_t required_size = num_nonzeros[w-ws];
        if(required_size > topics.size()){
            topics.resize(required_size);
            topic_split.resize(required_size);
        }
        while (!l_val.empty()){   
            int32_t l=l_val.back(); l_val.pop_back();
            float lp=l_pro.back(); l_pro.pop_back();
            if(!h_val.empty()){
                int32_t h=h_val.back(); h_val.pop_back();
                float hp=h_pro.back(); h_pro.pop_back();
                this->topics[curr_idx] = ( h << 16 ) | l;
                float hpn = hp-(one_over_topic-lp);
                if(hpn < one_over_topic || l_val.empty()) {
                    l_val.push_back(h);
                    l_pro.push_back(hpn);
                } else {
                    h_val.push_back(h);
                    h_pro.push_back(hpn);            
                } 
            } else {
                this->topics[curr_idx] = ( l << 16 ) | l;
            }
            this->topic_split[curr_idx] = lp*nonzeros;
            curr_idx ++;
        }
    }    
#else      
    size_t k_x_v = num_topics*LDA_CGS_VLEN ,i_x_v = 0;
    size_t loop_count = LDA_CGS_VLEN;  
    size_t sub_count = (num_voc_local-1)/LDA_CGS_VLEN + 1;   // avoid too much temporary memory requirement  
    std::vector<int32_t> l_val(k_x_v);
    std::vector<int32_t> h_val(k_x_v);
    std::vector<float> l_pro(k_x_v);
    std::vector<float> h_pro(k_x_v);
    std::vector<float> distribution(k_x_v);
    std::vector<int32_t> nonzeros_topic(k_x_v);
    size_t idx_off[LDA_CGS_VLEN];
    int32_t nonzeros[LDA_CGS_VLEN];
    int32_t l_last[LDA_CGS_VLEN];
    int32_t h_last[LDA_CGS_VLEN];
    float one_over_topic[LDA_CGS_VLEN];
    float normalization[LDA_CGS_VLEN];
#if defined(LDA_CGS_USE_VREG)
#pragma _NEC vreg(idx_off)
#pragma _NEC vreg(nonzeros)
#pragma _NEC vreg(l_last)
#pragma _NEC vreg(h_last)
#pragma _NEC vreg(one_over_topic)
#pragma _NEC vreg(normalization)
#endif
    for(size_t w=0; w<LDA_CGS_VLEN; w++){
        l_last[w] = -1;
        h_last[w] = -1;
        idx_off[w]= w*num_topics;
    }    

    for(size_t i=0;i<sub_count;i++){
        if (i == sub_count - 1) loop_count = (we-ws+1) % LDA_CGS_VLEN;
        if (loop_count == 0) loop_count = LDA_CGS_VLEN;
#pragma _NEC shortloop
        for(size_t w=0; w<loop_count; w++){  
            nonzeros[w] = 0;
            normalization[w] = 0;
        }  
        for(size_t w=0; w<loop_count*num_topics; w++){  
            distribution[w] = 0;
            nonzeros_topic[w] = -1;
        }           
        int32_t max_nonzero = -1;     
        for(size_t t=0; t<num_topics; t++){
#pragma _NEC ivdep
#pragma _NEC shortloop
            for(size_t w=0; w<loop_count; w++){
                size_t word_id = ws+i_x_v+w;
                auto word_topic_count = model.word_topic_count.val[word_id*num_topics+t]; 
                if(word_topic_count!=0) {
                    size_t idx = idx_off[w] + nonzeros[w];
                    nonzeros_topic[idx] = t;
                    nonzeros[w]++;
                    distribution[idx] = word_topic_count;
                    normalization[w] += distribution[idx];
                } 
            }
        }    
#pragma _NEC shortloop
        for(size_t w=0; w<loop_count; w++){
            if(max_nonzero<nonzeros[w]) max_nonzero=nonzeros[w];
            size_t idx = i_x_v+w;
            norms[idx] = normalization[w];
            num_nonzeros[idx] = nonzeros[w]; 
            one_over_topic[w] = 1/(float)nonzeros[w];
            nonzeros[w]=0;
        }     
        if(i!=0) num_nonzeros[i_x_v] += num_nonzeros[i_x_v-1];
        for(size_t w=i_x_v+1; w<i_x_v+loop_count; w++){
            num_nonzeros[w] += num_nonzeros[w-1];
        }   
        size_t required_size = num_nonzeros[i_x_v+loop_count-1];
        if(required_size > topics.size()){
            topics.resize(required_size);
            topic_split.resize(required_size);
        }
        for(size_t t=0; t<max_nonzero; t++){
#pragma _NEC ivdep
#pragma _NEC shortloop
            for(size_t w=0; w<loop_count; w++){
                size_t idx1 = idx_off[w] + t, idx2; 
                int32_t topic_id = nonzeros_topic[idx1];
                float one_over_topic_ = one_over_topic[w];
                if(topic_id!=-1){
                    distribution[idx1] /= normalization[w];
                    if(distribution[idx1] <= one_over_topic_){
                        l_last[w]++; idx2 = idx_off[w] + l_last[w]; 
                        l_val[idx2] = topic_id;
                        l_pro[idx2] = distribution[idx1];
                    } else {
                        h_last[w]++; idx2 = idx_off[w] + h_last[w]; 
                        h_val[idx2] = topic_id;
                        h_pro[idx2] = distribution[idx1];        
                    }
                }
            }    
        }       
        for(size_t t=0; t<max_nonzero; t++){
#pragma _NEC ivdep
#pragma _NEC shortloop
            for(size_t w=0; w<loop_count; w++){
                size_t word_idx = i_x_v+w;
                size_t curr_nonzero = word_idx==0? 0:num_nonzeros[word_idx-1];
                int empty_l = l_last[w]==-1? 1:0;
                int empty_h = h_last[w]==-1? 1:0;
                size_t idx1 = curr_nonzero + nonzeros[w], idx2;
                if(!empty_l){
                    float one_over_topic_ = one_over_topic[w];
                    idx2 = idx_off[w] + l_last[w];
                    int32_t l=l_val[idx2]; float lp=l_pro[idx2]; l_last[w]--;
                    if(!empty_h){
                        idx2 = idx_off[w] + h_last[w];
                        int32_t h=h_val[idx2]; float hp=h_pro[idx2]; h_last[w]--;
                        this->topics[idx1] = ( h << 16 ) | l;
                        float hpn = hp-(one_over_topic_-lp);
                        if(hpn <= one_over_topic_ || l_last[w]==-1) {
                            l_last[w]++; idx2 = idx_off[w] + l_last[w];
                            l_val[idx2]=h; l_pro[idx2]=hpn;
                        } else {
                            h_last[w]++; idx2 = idx_off[w] + h_last[w];
                            h_val[idx2]=h; h_pro[idx2]=hpn;   
                        } 
                    } else {
                        this->topics[idx1] = ( l << 16 ) | l;
                    }
                    this->topic_split[idx1] = lp/one_over_topic_;
                    nonzeros[w]++;
                }
            } 
        } 
        i_x_v += LDA_CGS_VLEN;
    }   
#endif
    
    size_t local_nonzero = num_nonzeros[we-ws];
    if(size>1) {
        this->gather_sparse_table(local_nonzero); 
    }
}

}
}

#endif /* _LDA_CGS_ALIAS_TABLE_HPP_ */

