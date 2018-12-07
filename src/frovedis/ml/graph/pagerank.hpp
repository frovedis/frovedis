

#ifndef GRAPH_PAGERANK_HPP
#define GRAPH_PAGERANK_HPP

#include <complex>
#include <cmath>
#include "frovedis/core/dvector.hpp"
#include "global.hpp"
namespace frovedis {
    

template <class T, class I, class O>
frovedis::node_local<std::vector<size_t>> 
partition_shrink_column_pagerank(frovedis::crs_matrix<T,I,O>& m) {
    auto tmp = m.data.map(frovedis::crs_get_local_num_row<T>).gather();
    std::vector<size_t> ret(tmp.size()+1);
    for(size_t i = 1; i < ret.size(); i++){
        ret[i] = ret[i-1] + tmp[i-1];
    }
    return frovedis::make_node_local_broadcast(ret);
}

template <class T, class I, class O>
frovedis::node_local<frovedis::shrink_vector_info<I>> 
prepare_shrink_comm_pagerank(frovedis::node_local<std::vector<I>>& tbl, frovedis::crs_matrix<T,I,O>& m) {
  auto v = partition_shrink_column_pagerank(m);
  return frovedis::create_shrink_vector_info(tbl, v);
}



template<class T>
std::vector<T>
alpha_times_vec(T alpha, std::vector<T>& v) {
    size_t size = v.size();
    std::vector<T> res(size,0);
    T* vp = &v[0];
    T* resp = &res[0];
    for(size_t i = 0; i < size; i++) resp[i] = alpha * vp[i]; 
    
    return res;
}

template<class T>
T sumup(std::vector<T>& v){
    T ret = 0;
    size_t size = v.size();
    for(size_t i=0;i<size;i++) 
        ret += v[i];
    return ret;
}

template<class T>
T sum(T a, T b){return a+b;}


template<class T>
T abs_d(T a){return std::abs(a);}

template<class T>
T vec_L1(std::vector<T>& v){
    T ret=0;
    for(auto i:v) ret += std::abs(i);
    return ret;
}

template<class T> 
std::vector<T>
normalize_vec(std::vector<T>& v){
    T sum=sumup(v);
    size_t v_size = v.size();
    std::vector<T> v_norm(v_size);   
    T* vp = &v[0];
    T* v_normp = &v_norm[0];
    for(size_t i=0; i<v_size;i++){
        v_normp[i] = vp[i]/sum;
    }
    return v_norm;
}

template <class T>
T cal_diff_abs(T a, T b){
    T ret = std::abs(a-b);
    return ret;
}


template<class T>
std::vector<T>
matvec_rowmj(frovedis::rowmajor_matrix_local<T>& m, std::vector<T>& v) {
    std::vector<T> ret(m.local_num_row);
        for(size_t r = 0; r < m.local_num_row; r++) {
            for(size_t c = 0; c < m.local_num_col; c++) {
                ret[r] += m.val[m.local_num_col * r + c] * v[c];
            }
        }
    return ret;
}

template<class T>
std::vector<T>
PRmat_rowmj(frovedis::crs_matrix_local<T>& m, std::vector<T>& prank, std::vector<double>& b, double& dfactor) {
    frovedis::rowmajor_matrix_local<T> p(m.local_num_row,m.local_num_col); //use rowmajor matrix for google pagerank matrix
    
    for(size_t r=0; r<p.local_num_row;r++){
        for(size_t c=0;c<p.local_num_col;c++){           
            p.val[p.local_num_col*r + c] = dfactor*b[c];               
        }
    }
    size_t row_next = 0;
    for(size_t i=0; i<m.val.size();i++){ //
        
    while(i >= m.off[row_next]){
            row_next ++;
        }
            p.val[p.local_num_col*(row_next-1) + m.idx[i]] += (1-dfactor)*m.val[i];                
    }    
    return matvec_rowmj(p,prank); 
}

//Avoid using dense matrix. Avoid using b vector
template<class T> 
std::vector<T> 
PRmat_crs(frovedis::crs_matrix_local<T>& m, std::vector<T>& prank, double& b, double& dfactor) {

    auto temp_prank = m * prank;
    size_t prank_size = temp_prank.size();    
    std::vector<T> temp_prank_new(prank_size,0);
    auto* temp_prankp = &temp_prank[0];
    auto* temp_prank_newp = &temp_prank_new[0];
    for(size_t i=0;i<prank_size;i++){
        temp_prank_newp[i] = (1-dfactor) * temp_prankp[i] + dfactor * b;
    }
    return temp_prank_new;
}

//Avoid using dense matrix. Avoid using b vector
template<class T> 
std::vector<T> 
PRmat_hyb(frovedis::jds_crs_hybrid_local<T>& m, std::vector<T>& prank, double& b, double& dfactor) {

    auto temp_prank = m * prank;
    size_t prank_size = temp_prank.size();    
    std::vector<double> temp_prank_new(prank_size,0);
    auto* temp_prankp = &temp_prank[0];
    auto* temp_prank_newp = &temp_prank_new[0];
    for(size_t i=0;i<prank_size;i++){
        temp_prank_newp[i] = (1-dfactor) * temp_prankp[i] + dfactor * b;
    }
   
    return temp_prank_new;

}

template<class T> 
std::vector<T> 
PRmat_tempprank_crs(frovedis::crs_matrix_local<T>& m, std::vector<T>& prank, double dfactor){
    auto temp_prank = m * prank;
    auto new_temp_prank = alpha_times_vec<T>(1-dfactor, temp_prank);
    return new_temp_prank;
}

template<class T>
std::vector<T> 
PRmat_tempprank_hyb(frovedis::jds_crs_hybrid_local<T>& m, std::vector<T>& prank, T dfactor){
    auto temp_prank = m * prank;
    auto new_temp_prank = alpha_times_vec<T>(1-dfactor, temp_prank);
    return new_temp_prank;
}

template<class T>
T PRmat_diff(std::vector<T>& prank, std::vector<T>& prank_new) {
    auto diff = vec_L1(prank) - vec_L1(prank_new);  
    return diff;

}

template<class T>
std::vector<T> 
PRmat_newprank(std::vector<T>& temp_prank, T diff, T b){
    auto prank_size = temp_prank.size();
    std::vector<T> prank_new(prank_size,0);
    auto* prank_newp = &prank_new[0];
    auto* temp_prankp = &temp_prank[0];

    for(size_t i = 0; i< prank_size;i++){
        prank_newp[i] = temp_prankp[i] + diff * b;
    }
    return prank_new;
}

template<class T>
T PRmat_newprank_elem(T temp_prank_elem, T diff, T b){

    return temp_prank_elem + diff * b;
}

template<class T>
T cal_abs_diff_vec(std::vector<T>& v1,std::vector<T>& v2){
    T ret = 0;
    size_t v_size = v1.size();
    auto* v1p = &v1[0];
    auto* v2p = &v2[0];
    for(size_t i=0;i<v_size;i++){ 
        ret += std::abs(v1p[i]-v2p[i]);
    }
    return ret;
}

template<class T>
std::vector<T>
cal_rel_prank(std::vector<T>& abs_prank){
    size_t prank_size = abs_prank.size();
    std::vector<T> rel_prank(prank_size);
    auto* rel_prankp = &rel_prank[0];
    auto* abs_prankp = &abs_prank[0];

    T sum_prank = std::accumulate(abs_prank.begin(),abs_prank.end(),0.0);
    
    for(size_t i=0; i< prank_size;i++){
        rel_prankp[i] = abs_prankp[i]/sum_prank;
    }  
    return rel_prank;
}
template<class T>
bool by_rank_descend(std::pair<size_t, T> a, std::pair<size_t, T> b) { return  a.second> b.second; }

template<class T>
std::vector<std::pair<size_t, T>> 
sort_prank(std::vector<T>& prank){
    size_t prank_size = prank.size();
    std::vector<std::pair<size_t, T>> prank_sorted(prank_size);
    auto* prank_sortedp = &prank_sorted[0];

    // prank_sorted.resize(prank.size());
    for(size_t i=0; i<prank_size;i++){
        prank_sortedp[i].first = i;
        prank_sortedp[i].second = prank[i];
    }
    std::sort(prank_sorted.begin(),prank_sorted.end(),by_rank_descend<T>);
    return prank_sorted;
}

template<class T>
void print_sort_prank(std::vector<T> prank){
    RLOG(TIME_RECORD_LOG_LEVEL)<<std::endl;
    RLOG(TIME_RECORD_LOG_LEVEL)<<"-------------------------------------------------"<<std::endl;   
    RLOG(TIME_RECORD_LOG_LEVEL)<<"Printing the RELATIVE ranks for the top 20 pages"<<std::endl;
    auto prank_sorted = sort_prank(prank);
    size_t i = 0;
    size_t max_page_out = 20;
        RLOG(TIME_RECORD_LOG_LEVEL)<<"No.\t"<<"pageID\t"<<"Rank"<<std::endl;
    while(i < max_page_out && i < prank_sorted.size()){
        RLOG(TIME_RECORD_LOG_LEVEL)<<i<<"\t"<<prank_sorted[i].first<<"\t"<<prank_sorted[i].second<<std::endl;
        i++;
    }    
    RLOG(TIME_RECORD_LOG_LEVEL)<<"...."<<std::endl;
    RLOG(TIME_RECORD_LOG_LEVEL)<<"-------------------------------------------------"<<std::endl;
}



}
#endif /* GRAPH_PAGERANK_HPP*/

