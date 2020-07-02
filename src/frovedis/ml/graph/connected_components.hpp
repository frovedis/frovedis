
#ifndef GRAPH_CONNECTED_COMPONENTS_HPP
#define GRAPH_CONNECTED_COMPONENTS_HPP

// #include "sparse_vector.hpp"
#include <frovedis/matrix/crs_matrix.hpp>
// #include "spMspV.hpp"

#define HYB_TH_NUM_ELEM_TOTAL 1000000
#define HYB_TH_NUM_VEC 100000

namespace frovedis {
//Check ifVisited
template <class T>
std::vector<T>
check_ifVisited_bitmap(std::vector<T>& nodes_next_temp, std::vector<T>& nodes_ifVisited){
    auto vsize = nodes_next_temp.size();
    std::vector<T> nodes_next(vsize,0);
    auto* nodes_next_temp_ptr = &nodes_next_temp[0];
    auto* nodes_ifVisited_ptr = &nodes_ifVisited[0];
    auto* nodes_next_ptr = &nodes_next[0];
    
    for(size_t i=0;i<vsize;i++){
        nodes_next_ptr[i] = ((bool)nodes_next_temp_ptr[i])* (1 - nodes_ifVisited_ptr[i]) ;
    }
    return nodes_next;
}

template <class T, class B>
sparse_vector<B>
check_ifVisited_spV(std::vector<T>& nodes_next_temp_idx, std::vector<B>& nodes_ifVisited){

    size_t v_idx_size = nodes_next_temp_idx.size();
    sparse_vector<B> nodes_next_sp_temp(v_idx_size);
    auto* nodes_next_temp_idx_ptr = &nodes_next_temp_idx[0];
    auto* nodes_ifVisited_ptr = &nodes_ifVisited[0];
    auto* nodes_next_sp_temp_idx_ptr = &nodes_next_sp_temp.idx[0];
    auto* nodes_next_sp_temp_val_ptr = &nodes_next_sp_temp.val[0];    
    size_t idx_current = 0;
    
    for(size_t i=0;i<v_idx_size;i++){
        if(nodes_ifVisited_ptr[nodes_next_temp_idx_ptr[i]] == 0){
            nodes_next_sp_temp_idx_ptr[idx_current] = nodes_next_temp_idx_ptr[i];
            nodes_next_sp_temp_val_ptr[idx_current] = 1;
            idx_current++;
        }
    }
    sparse_vector<B> nodes_next_sp(idx_current);
    auto* nodes_next_sp_idx_ptr = &nodes_next_sp.idx[0];
    auto* nodes_next_sp_val_ptr = &nodes_next_sp.val[0];
    
    for(size_t i=0; i<idx_current; i++){
        nodes_next_sp_idx_ptr[i] = nodes_next_sp_temp_idx_ptr[i];
        nodes_next_sp_val_ptr[i] = nodes_next_sp_temp_val_ptr[i];
    }
    return nodes_next_sp;
}


template <class T, class B>
std::vector<T>
check_ifVisited_idx(std::vector<T>& nodes_next_temp_idx, std::vector<B>& nodes_ifVisited){
    //Only idx vector rather than sparse_vector(idx,val), enough for BFS. Reduce communication overhead.
    size_t v_idx_size = nodes_next_temp_idx.size();

    std::vector<T> nodes_next_temp(v_idx_size, 0);
    auto* nodes_next_temp_idx_ptr = &nodes_next_temp_idx[0];
    auto* nodes_ifVisited_ptr = &nodes_ifVisited[0];
    auto* nodes_next_temp_ptr = &nodes_next_temp[0];  
    size_t idx_current = 0;
    
    for(size_t i=0;i<v_idx_size;i++){
        if(nodes_ifVisited_ptr[nodes_next_temp_idx_ptr[i]] == 0){
            nodes_next_temp_ptr[idx_current] = nodes_next_temp_idx_ptr[i];
            idx_current++;
        }
    }
    std::vector<T> nodes_next(idx_current,0);
    auto* nodes_next_idx_ptr = &nodes_next[0];
    
    for(size_t i=0; i<idx_current; i++){
        nodes_next_idx_ptr[i] = nodes_next_temp_ptr[i];
    }
    return nodes_next;
}



//sparse_vector<T>
//check_ifVisited(sparse_vector<T>& nodes_next_temp_sp, std::vector<T>& nodes_ifVisited){
//    //Generalized function: Input and output are both sparse vectors. 
//    //Push_back is not friendly to SX.
//    auto vsize_sp = nodes_next_temp_sp.idx.size();
//    sparse_vector<T> nodes_next_sp;
//    auto* nodes_next_temp_sp_idx_ptr = &nodes_next_temp_sp.idx[0];
//    auto* nodes_next_temp_sp_val_ptr = &nodes_next_temp_sp.val[0];    
//    auto* nodes_ifVisited_ptr = &nodes_ifVisited[0];
//    
//    for(size_t i=0;i<vsize_sp;i++){
//        if(nodes_ifVisited_ptr[nodes_next_temp_sp_idx_ptr[i]] == 0){
//            nodes_next_sp.idx.push_back(nodes_next_temp_sp_idx_ptr[i]);
//            nodes_next_sp.val.push_back(nodes_next_temp_sp_val_ptr[i]);      
//        }
//    }
//    return nodes_next_sp;
//}



template <class T, class B>
//Update ifVisited List
void update_ifVisited(std::vector<T>& nodes_current, std::vector<B>& nodes_ifVisited){
    auto vsize = nodes_current.size();
    auto* nodes_current_ptr = &nodes_current[0];
    auto* nodes_ifVisited_ptr = &nodes_ifVisited[0];
    
    for(size_t i=0;i<vsize;i++){
        nodes_ifVisited_ptr[i] = nodes_current_ptr[i] + nodes_ifVisited_ptr[i];
    }
}


template <class T,class B>
void update_ifVisited_sp(sparse_vector<T>& nodes_current_sp, std::vector<B>& nodes_ifVisited){
    auto vsize_ifCurrent = nodes_current_sp.idx.size();
    auto* nodes_current_idx_ptr = &nodes_current_sp.idx[0];    
    auto* nodes_ifVisited_ptr = &nodes_ifVisited[0];
    
    for(size_t i=0;i<vsize_ifCurrent;i++){
        nodes_ifVisited_ptr[nodes_current_idx_ptr[i]] = 1;
    }
}

template <class T, class B>
void update_ifVisited_idx(std::vector<T>& nodes_current_idx, std::vector<B>& nodes_ifVisited){//Only idx rather than sparse_vector. Reduce comm overhead
    auto vsize_ifCurrent = nodes_current_idx.size();
    auto* nodes_current_idx_ptr = &nodes_current_idx[0];
    auto* nodes_ifVisited_ptr = &nodes_ifVisited[0];
    
    for(size_t i=0;i<vsize_ifCurrent;i++){
        nodes_ifVisited_ptr[nodes_current_idx_ptr[i]] = 1;
    }
}

//spMV 
template <class O, class T>
std::vector<T> spMV_crs(frovedis::crs_matrix_local<O>& m, std::vector<T>& nodes_current) {
    
    auto nodes_next =  m * nodes_current;
    return nodes_next;     
}

template <class O, class T>
std::vector<T> spMV_hyb(frovedis::jds_crs_hybrid_local<O>& m, std::vector<T>& v) {
  return m * v;
}
template <class O, class T>
std::vector<T> spMV_jds(frovedis::jds_matrix_local<O>& m, std::vector<T>& v) {
  return m * v;
}

//spMspV
template <class O, class T>
std::vector<T> spMspV_bitmap(frovedis::ccs_matrix_local<O>& m, sparse_vector<T>& nodes_current) {//Return a bitmap dense vector
    auto nodes_next =  m * nodes_current;
    return nodes_next;
}

//template <class O, class T>
//sparse_vector<T> spMspV_scatter(frovedis::ccs_matrix_local<O>& m, sparse_vector<O>& nodes_current, std::vector<size_t> nodes_offset) {
//    //Generalized version. 
//    //input: sparse_vector, output: sparse_vector
//    auto* vvalp = &nodes_current.val[0];  
//    auto* vidxp = &nodes_current.idx[0];
//    auto num_vnnz = nodes_current.idx.size(); 
//
//    std::vector<O> BufferVec(m.local_num_row,0);//upper bound of the output vector size
//
//    auto* BufferVec_p = &BufferVec[0];
//
//    auto* mvalp = &m.val[0];
//    auto* midxp = &m.idx[0];
//    auto* moffp = &m.off[0];
//  
//    for(size_t c=0; c<num_vnnz;c++){ 
//
//    #pragma cdir nodep
//    #pragma _NEC ivdep
//    #pragma cdir on_adb(vvalp)
//    #pragma cdir on_adb(vidxp)
//        for(auto r = moffp[ vidxp[c]]; r<moffp[ vidxp[c]+1];r++){ 
//            BufferVec_p[midxp[r]] = BufferVec_p[midxp[r]] + mvalp[r] * vvalp[c];
//        }
//    }
//    
//    size_t node_offset = nodes_offset[frovedis::get_selfid()];
//    size_t vsize_nodes_next = BufferVec.size();
//    size_t* output_idx_temp_ptr = new T[vsize_nodes_next]; //using new instead of vector to avoid initializing zero value to each element.
//    size_t* output_val_temp_ptr = new T[vsize_nodes_next];    
//    size_t idx_current = 0;
//
//    #pragma _NEC ivdep
//    for(size_t i=0;i<vsize_nodes_next;i++){
//        if(BufferVec_p[i] != 0){          
//            output_idx_temp_ptr[idx_current] = i + node_offset;
//            output_val_temp_ptr[idx_current] = BufferVec_p[i];
//            idx_current++;
//        }
//    }
//
//    sparse_vector<T> output(idx_current,0);    
//    size_t* output_idx_ptr = &output.idx[0];
//    size_t* output_val_ptr = &output.val[0];
//        
//    for(size_t i = 0;i < idx_current; i ++){
//        output_idx_ptr[i] = output_idx_temp_ptr[i];
//        output_val_ptr[i] = output_idx_temp_ptr[i];
//    }
//    
//    delete output_idx_temp_ptr;
//    delete output_val_temp_ptr;
//
//    return output;
//     
//}


template <class O, class T>
std::vector<T> spMspV_scatter_spv_idx(frovedis::ccs_matrix_local<O>& m, sparse_vector<O>& nodes_current, std::vector<size_t> nodes_offset) {
    //input sparse_vector, output idx vector
    auto* vvalp = &nodes_current.val[0];  
    auto* vidxp = &nodes_current.idx[0];
    auto num_vnnz = nodes_current.idx.size(); 

    std::vector<O> BufferVec(m.local_num_row,0);//upper bound of the output vector size

    auto* BufferVec_p = &BufferVec[0];

    auto* mvalp = &m.val[0];
    auto* midxp = &m.idx[0];
    auto* moffp = &m.off[0];
  
    for(size_t c=0; c<num_vnnz;c++){ 

    #pragma cdir nodep
    #pragma _NEC ivdep
    #pragma cdir on_adb(vvalp)
    #pragma cdir on_adb(vidxp)
        for(auto r = moffp[ vidxp[c]]; r<moffp[vidxp[c]+1];r++){ 
            BufferVec_p[midxp[r]] = BufferVec_p[midxp[r]] + mvalp[r] * vvalp[c];
        }
    }
    
    size_t node_offset = nodes_offset[frovedis::get_selfid()];
    size_t vsize_nodes_next = BufferVec.size();
    size_t* nodes_next_idx_temp_ptr = new T[vsize_nodes_next]; //using new instead of vector to avoid initializing zero value to each element.   
    size_t idx_current = 0;

    #pragma _NEC ivdep
    for(size_t i=0;i<vsize_nodes_next;i++){
        if(BufferVec_p[i] != 0){          
            nodes_next_idx_temp_ptr[idx_current++] = i + node_offset;
        }
    }
    std::vector<size_t> nodes_next_idx(idx_current,0);
    size_t* nodes_next_idx_ptr = &nodes_next_idx[0];
    
    for(size_t i = 0;i < idx_current; i ++){
        nodes_next_idx_ptr[i] = nodes_next_idx_temp_ptr[i];
    }
    
    delete nodes_next_idx_temp_ptr;
    return nodes_next_idx;
     
}
template <class O, class T>
std::vector<T> spMspV_scatter_idx(frovedis::ccs_matrix_local<O>& m, std::vector<T>& nodes_current_idx, std::vector<size_t> nodes_offset) {
   //input is only idx vector instead of sparse_vector. Enough for BFS
  auto* vidxp = &nodes_current_idx[0];
  auto num_vnnz = nodes_current_idx.size(); 
  
  std::vector<T> nodes_next(m.local_num_row,0);//upper bound of the output vector size
  auto* nodes_nextp = &nodes_next[0];
  auto* mvalp = &m.val[0];
  auto* midxp = &m.idx[0];
  auto* moffp = &m.off[0];


    for(size_t c=0; c<num_vnnz;c++){ 
    #pragma cdir nodep
    #pragma _NEC ivdep
    #pragma cdir on_adb(vvalp)
    #pragma cdir on_adb(vidxp)
        for(size_t r = moffp[ vidxp[c]]; r<moffp[ vidxp[c]+1];r++){ 
            nodes_nextp[midxp[r]] = nodes_nextp[midxp[r]] + mvalp[r];
        }
    }

    size_t node_offset = nodes_offset[frovedis::get_selfid()];
    size_t vsize_nodes_next = nodes_next.size();
    T* nodes_next_idx_temp_ptr = new T[vsize_nodes_next]; //using new instead of vector to avoid initializing zero value to each element.    
    T* nodes_next_ptr = &nodes_next[0];
    
    size_t idx_current = 0;

    #pragma _NEC ivdep
    for(size_t i=0;i<vsize_nodes_next;i++){
        if(nodes_next_ptr[i] != 0){          
            nodes_next_idx_temp_ptr[idx_current++] = i + node_offset;
        }
    }
    std::vector<T> nodes_next_idx(idx_current,0);
    auto* nodes_next_idx_ptr = &nodes_next_idx[0];   
    for(size_t i = 0;i < idx_current; i ++){
        nodes_next_idx_ptr[i] = nodes_next_idx_temp_ptr[i];
    }
    
    delete nodes_next_idx_temp_ptr;
    return nodes_next_idx;
     
}
template <class O, class T>
std::vector<T> spMspV_scatter_unvisited(frovedis::ccs_matrix_local<O>& m, 
        std::vector<T>& vidx, 
        std::vector<size_t>& nodes_offset, 
        std::vector<T>& lnodes_ifVisited) {
    //Check ifVisited locally
    //without push_back, friendly to SX
    std::vector<T> vval(vidx.size());
    auto* vvalp = &vval[0];
    for(size_t i=0; i<vidx.size(); i++){
        vvalp[i] = 1;
    }
    sparse_vector<T> v_sparse(vidx,vval);
            
    auto nodes_next =  spMspV_bitmap(m,v_sparse);
    size_t node_offset = nodes_offset[frovedis::get_selfid()];
    auto m_local_num_row = m.local_num_row;
    size_t vsize_nodes_next = nodes_next.size();
    
    T* nodes_next_idx_temp_ptr = new T[vsize_nodes_next]; //using new instead of vector to avoid initializing zero value to each element.    
    T* nodes_next_ptr = &nodes_next[0];    
    std::vector<T> nodes_next_idx;

    size_t idx_current = 0;

    for(size_t i=0;i<vsize_nodes_next;i++){
        if(nodes_next_ptr[i] != 0 ){          
            nodes_next_idx_temp_ptr[idx_current++] = i + node_offset;
        }
    }
    nodes_next_idx.resize(idx_current);
    T* nodes_next_idx_ptr = &nodes_next_idx[0];
    
    for(size_t i = 0;i < idx_current; i ++){
        nodes_next_idx_ptr[i] = nodes_next_idx_temp_ptr[i];
    }

    delete nodes_next_idx_temp_ptr;

    //update and check ifVisited
    auto* vidxp = &vidx[0];
    auto* lnodes_ifVisited_ptr = &lnodes_ifVisited[0];
    for(size_t i=0; i<vidx.size();i++){
        if(vidxp[i] >= node_offset && vidxp[i] < node_offset + m_local_num_row){
            lnodes_ifVisited_ptr[vidxp[i]-node_offset] = 1;
        }
    }
    auto* ret_idx_unvisited_temp_ptr = new size_t[nodes_next_idx.size()];
    size_t ret_idx_count = 0;
    for(size_t i=0; i<nodes_next_idx.size();i++){
        if(lnodes_ifVisited_ptr[nodes_next_idx_ptr[i]-node_offset] == 0){
            ret_idx_unvisited_temp_ptr[ret_idx_count++] = nodes_next_idx_ptr[i];
        }
    }
    std::vector<T> ret_idx_unvisited(ret_idx_count);
    auto* ret_idx_unvisited_ptr = &ret_idx_unvisited[0];
    for(size_t i=0; i<ret_idx_count;i++){
        ret_idx_unvisited_ptr[i] = ret_idx_unvisited_temp_ptr[i];

    }    
    
    delete ret_idx_unvisited_temp_ptr;
    return ret_idx_unvisited;
     
}

template <class O, class T>
std::vector<T> 
spMspV_idxSort(frovedis::ccs_matrix_local<O>& m, sparse_vector<O>& nodes_current, std::vector<size_t> nodes_offset) {

  size_t node_offset = nodes_offset[frovedis::get_selfid()];
  auto* vvalp = &nodes_current.val[0];  
  auto* vidxp = &nodes_current.idx[0];
  auto num_vnnz = nodes_current.idx.size();
  auto m_local_num_row = m.local_num_row;
    
  auto* mvalp = &m.val[0];
  auto* midxp = &m.idx[0];
  auto* moffp = &m.off[0];
  
  auto* SPAp = new O[m_local_num_row]; 
 
  size_t max_num_elem = 0;
    #pragma cdir nodep 
    #pragma _NEC ivdep
  for(size_t c=0; c<num_vnnz;c++){ 
        max_num_elem += moffp[vidxp[c]+1] - moffp[vidxp[c]];
    }
  
   std::vector<T> idxSPA(max_num_elem,0); //FIXME: Reserve for m_local_num_row may not be eonough for denser vector, but m_local_num_row*vnnz is too large and exceed the maximum size for a vector
  auto* idxSPAp = &idxSPA[0];
  size_t idx_current = 0; 
    for(size_t c=0; c<num_vnnz;c++){ 
        auto col_idx = vidxp[c];
        #pragma cdir nodep 
        #pragma _NEC ivdep
        for(size_t r = moffp[col_idx]; r<moffp[col_idx+1];r++){ 
            SPAp[midxp[r]] = SPAp[midxp[r]] + mvalp[r] * vvalp[c];
            idxSPAp[idx_current] = midxp[r] + node_offset;
            idx_current++;

        }
    }

   std::vector<O> valSPA(idxSPA.size(),0);
   auto* valSPAp = &valSPA[0];
   for(size_t i=0; i<valSPA.size(); i++){
       valSPAp[i] = SPAp[idxSPAp[i] - node_offset]; //minus the offset to get value
   }

//#ifndef _SX   
#if !defined(_SX) && !defined(__ve__)  
    std::sort( idxSPA.begin(), idxSPA.end() );
#else
    frovedis::radix_sort(idxSPA,valSPA);
#endif
    
    auto idxSPA_sep = set_separate(idxSPA);
    auto* idxSPA_sep_p = &idxSPA_sep[0];
    size_t idx_neat_size = idxSPA_sep.size()-1;
    std::vector<T> idx_neat(idx_neat_size,0);

    auto* idx_neatp = &idx_neat[0];
    for(size_t i = 0; i<idx_neat_size;i++){
        idx_neatp[i] = idxSPA[idxSPA_sep_p[i]];
    }
    
    delete SPAp;
    return idx_neat;
}


template <class O, class T>
std::vector<T> 
spMspV_HybridMerge(frovedis::ccs_matrix_local<O>& m, 
        sparse_vector<T>& v, 
        std::vector<size_t> nodes_offset, 
        size_t merge_th) {//Horizontal + Vertical Merge
    
    size_t node_offset = nodes_offset[frovedis::get_selfid()];

    auto* vidxp = &v.idx[0];
    auto num_vnnz = v.idx.size(); 
    auto* midxp = &m.idx[0];
    auto* moffp = &m.off[0];


    std::vector<size_t> num_idx_per_col(num_vnnz,0);
    auto* num_idx_per_col_ptr = &num_idx_per_col[0];
    size_t num_idx = 0;

    #pragma cdir nodep
    #pragma cdir on_adb(num_idx_per_col_ptr)    
    for(size_t c=0; c<num_vnnz;c++){ 
        auto col_idx = vidxp[c]; 
        num_idx_per_col_ptr[c] = moffp[col_idx+1] - moffp[col_idx];
        num_idx += moffp[col_idx+1] - moffp[col_idx];
    }
    
    size_t vec_idx[num_vnnz];
    size_t vec_idx_stop[num_vnnz];

    vec_idx[0] = 0;
    vec_idx_stop[0] = num_idx_per_col_ptr[0];
    for(size_t i=1; i<num_vnnz; i++){
        vec_idx[i] = vec_idx_stop[i-1];
        vec_idx_stop[i] = vec_idx[i] + num_idx_per_col_ptr[i];
    }
    
    std::vector<T> idx_buff(num_idx,0);
    auto* idx_buff_ptr = &idx_buff[0];

    size_t col_idx; 
    size_t idx_per_col_current;
        
    for(size_t c=0; c<num_vnnz;c++){ 
        col_idx = vidxp[c]; 
        idx_per_col_current = 0;

    #pragma cdir nodep
    #pragma cdir on_adb(idx_buff_ptr)
    #pragma cdir on_adb(vec_idx)
        for(size_t r = moffp[col_idx]; r<moffp[col_idx+1];r++){
            idx_buff_ptr[vec_idx[c]+idx_per_col_current++] = midxp[r] + node_offset;
        }
    }

    std::vector<T> ret_idx;

    if(num_vnnz > merge_th){
        ret_idx = set_union_horizontal<T>(idx_buff,num_idx_per_col, vec_idx,merge_th);
    }

    else if(num_vnnz <= merge_th && num_vnnz > 1){
        std::vector<std::vector<T>> idx_per_col(num_vnnz);
        auto* idx_per_col_ptr = &idx_per_col[0];
        for(size_t i=0; i<num_vnnz;i++){
            idx_per_col_ptr[i].resize(num_idx_per_col[i]);
    #pragma cdir nodep 
    #pragma _NEC ivdep
            for(size_t j=0; j<num_idx_per_col_ptr[i];j++){
                idx_per_col_ptr[i][j] =  idx_buff_ptr[vec_idx[i] + j];
            }
        }
        ret_idx = set_union_multvec_hierarchical<T>(idx_per_col);    
    }
    else {        
        ret_idx = idx_buff;
    }
    return ret_idx; //Currently only return ret_idx without ret_valp;
}


//template <class O, class T>
//sparse_vector<T> spMspV_2DMerge(frovedis::ccs_matrix_local<O>& m, 
//        sparse_vector<T>& v, 
//        std::vector<size_t>& nodes_offset,
//        size_t& merge_th) {
//    //Generalized Version
//    //Input & Output vectors are sparse
//    size_t node_offset = nodes_offset[frovedis::get_selfid()];
//
//    auto* vidxp = &v.idx[0];
//    auto* vvalp = &v.val[0];
//    auto num_vnnz = v.idx.size(); 
//    auto* midxp = &m.idx[0];
//    auto* moffp = &m.off[0];
//    auto* mvalp = &m.val[0];
//
//    std::vector<size_t> num_elem_per_col(num_vnnz,0);
//    auto* num_elem_per_col_ptr = &num_elem_per_col[0];
//    size_t num_elem = 0;
//    
//    #pragma cdir nodep
//    #pragma cdir on_adb(num_elem_per_col_ptr)    
//    for(size_t c=0; c<num_vnnz;c++){ 
//        auto col_idx = vidxp[c]; 
//        num_elem_per_col_ptr[c] = moffp[col_idx+1] - moffp[col_idx];
//        num_elem += moffp[col_idx+1] - moffp[col_idx];
//    }
//    
//    sparse_vector<T> buffer(num_elem,0);
//
//    auto* buffer_idx_ptr = &buffer.idx[0];
//    auto* buffer_val_ptr = &buffer.val[0];
//    
//    std::vector<size_t> vec_idx(num_vnnz,0);
//    std::vector<size_t> vec_idx_stop(num_vnnz,0);
//    auto* vec_idx_ptr = &vec_idx[0];
//    auto* vec_idx_stop_ptr = &vec_idx_stop[0];
//    
//    vec_idx_ptr[0] = 0;
//    vec_idx_stop_ptr[0] = num_elem_per_col_ptr[0];
//    for(size_t i=1; i<num_vnnz; i++){
//        vec_idx_ptr[i] = vec_idx_stop_ptr[i-1];
//        vec_idx_stop_ptr[i] = vec_idx_ptr[i] + num_elem_per_col_ptr[i];     
//    }
//
//    size_t col_idx; 
//    size_t idx_per_col_current;
//        
//    for(size_t c=0; c<num_vnnz;c++){ 
//        col_idx = vidxp[c]; 
//        idx_per_col_current = 0;
//
//    #pragma cdir nodep
//    #pragma cdir on_adb(buffer_idx_ptr)
//    #pragma cdir on_adb(vec_idx)
//        for(size_t r = moffp[col_idx]; r<moffp[col_idx+1];r++){
//            buffer_idx_ptr[vec_idx_ptr[c]+idx_per_col_current] = midxp[r] + node_offset;
//            buffer_val_ptr[vec_idx_ptr[c]+idx_per_col_current] = mvalp[r] * vvalp[c]; 
//            idx_per_col_current++;
//        }
//    }
//
//    sparse_vector<T> ret_sp;
//    
//    if(num_vnnz > 1){
//        ret_sp = set_union_2D<T>(buffer,num_elem_per_col, vec_idx, merge_th);
//    }
//    else {
//        ret_sp = buffer;
//    }
//
//    return ret_sp; //Currently only return ret_idx without ret_valp;
//}


template <class O, class T>
std::vector<T> 
spMspV_2DMerge_idx(frovedis::ccs_matrix_local<O>& m, 
        std::vector<T>& v, 
        std::vector<size_t>& nodes_offset, 
        size_t& merge_th) {
    //Optimized for BFS
    //Input vector is only idx vector rather than sparse_vector to reduce comm overhead.
    size_t node_offset = nodes_offset[frovedis::get_selfid()];

    auto* vidxp = &v[0];
    auto num_vnnz = v.size(); 
    auto* midxp = &m.idx[0];
    auto* moffp = &m.off[0];


    std::vector<size_t> num_idx_per_col(num_vnnz,0);
    auto* num_idx_per_col_ptr = &num_idx_per_col[0];
    size_t num_idx = 0;
    
    #pragma cdir nodep
    #pragma cdir on_adb(num_idx_per_col_ptr)    
    for(size_t c=0; c<num_vnnz;c++){ 
      auto col_idx = vidxp[c]; 
      num_idx_per_col_ptr[c] = moffp[col_idx+1] - moffp[col_idx];
      num_idx += moffp[col_idx+1] - moffp[col_idx];
    }
    
    std::vector<T> idx_buff(num_idx,0);
    auto* idx_buff_ptr = &idx_buff[0];
    
    std::vector<size_t> vec_idx(num_vnnz,0);
    std::vector<size_t> vec_idx_stop(num_vnnz,0);
    auto* vec_idx_ptr = &vec_idx[0];
    auto* vec_idx_stop_ptr = &vec_idx_stop[0];
    
    vec_idx_ptr[0] = 0;
    vec_idx_stop_ptr[0] = num_idx_per_col_ptr[0];
    for(size_t i=1; i<num_vnnz; i++){
        vec_idx_ptr[i] = vec_idx_stop_ptr[i-1];
        vec_idx_stop_ptr[i] = vec_idx_ptr[i] + num_idx_per_col_ptr[i];       
    }

    size_t col_idx; 
    size_t idx_per_col_current;
        
    for(size_t c=0; c<num_vnnz;c++){ 
        col_idx = vidxp[c]; 
        idx_per_col_current = 0;

    #pragma cdir nodep
    #pragma cdir on_adb(idx_buff_ptr)
    #pragma cdir on_adb(vec_idx)
        for(size_t r = moffp[col_idx]; r<moffp[col_idx+1];r++){
            idx_buff_ptr[vec_idx_ptr[c]+idx_per_col_current++] = midxp[r] + node_offset;
        }
    }

    std::vector<T> ret_idx;
    if(num_vnnz > 1){

        ret_idx = set_union_2d_idx_highd<T>(idx_buff,num_idx_per_col, vec_idx,merge_th);
    }
    else {        
        ret_idx = idx_buff;
    }

    return ret_idx; //Currently only return ret_idx without ret_valp;
}

template <class O, class T>
std::vector<T> 
spMspV_2DMerge_idx_scalefree(frovedis::ccs_matrix_local<O>& m, 
        std::vector<T>& v, 
        std::vector<size_t>& nodes_offset, 
        size_t& merge_th) {
    //Optimized for low-diamater scale-free graphs. May be inefficient for high-diameter graphs.
    size_t node_offset = nodes_offset[frovedis::get_selfid()];

    auto* vidxp = &v[0];
    auto num_vnnz = v.size(); 

    auto* midxp = &m.idx[0];
    auto* moffp = &m.off[0];


    std::vector<size_t> num_idx_per_col(num_vnnz,0);
    auto* num_idx_per_col_ptr = &num_idx_per_col[0];
    size_t num_idx = 0;
    
    #pragma cdir nodep
    #pragma cdir on_adb(num_idx_per_col_ptr)    
    for(size_t c=0; c<num_vnnz;c++){
        auto col_idx = vidxp[c]; 
        num_idx_per_col_ptr[c] = moffp[col_idx+1] - moffp[col_idx];
        num_idx += moffp[col_idx+1] - moffp[col_idx];
    }
  
    std::vector<T> idx_buff(num_idx,0);
    auto* idx_buff_ptr = &idx_buff[0];
    

    std::vector<size_t> vec_idx(num_vnnz,0); //Pointers
    std::vector<size_t> vec_idx_stop(num_vnnz,0);
    auto* vec_idx_ptr = &vec_idx[0];
    auto* vec_idx_stop_ptr = &vec_idx_stop[0];
    vec_idx_ptr[0] = 0;
    vec_idx_stop_ptr[0] = num_idx_per_col_ptr[0];
    
    for(size_t i=1; i<num_vnnz; i++){
        vec_idx_ptr[i] = vec_idx_stop_ptr[i-1];
        vec_idx_stop_ptr[i] = vec_idx_ptr[i] + num_idx_per_col_ptr[i];  
    }

    size_t col_idx; 
    size_t idx_per_col_current;
        
    for(size_t c=0; c<num_vnnz;c++){ 
        col_idx = vidxp[c]; 
        idx_per_col_current = 0;
        #pragma cdir nodep
        #pragma cdir on_adb(idx_buff_ptr)
        #pragma cdir on_adb(vec_idx)
        for(size_t r = moffp[col_idx]; r<moffp[col_idx+1];r++){
            idx_buff_ptr[vec_idx_ptr[c]+idx_per_col_current++] = midxp[r] + node_offset;
        }
    }


    std::vector<T> ret_idx;

    if(num_vnnz > 1){
        ret_idx = set_union_2d_idx_lowd<T>(idx_buff,num_idx_per_col, vec_idx,merge_th);            
    }
    else {
        ret_idx = idx_buff;
    }

    return ret_idx; //Currently only return ret_idx without ret_valp;
}

template <class O, class T>
std::vector<T> 
spMspV_2DMerge_idx_Hyb(frovedis::ccs_matrix_local<O>& m, 
        std::vector<T>& v, 
        std::vector<size_t>& nodes_offset, 
        size_t& merge_th) {    // Using Hybrid Strategy: spMspV-Scatter + spMspV-2DMerge. Designed for Low-diameter Scale-free graphs


    size_t node_offset = nodes_offset[frovedis::get_selfid()];

    auto* vidxp = &v[0];

    auto num_vnnz = v.size(); 
    auto m_local_num_row = m.local_num_row;

    auto* mvalp = &m.val[0];
    auto* midxp = &m.idx[0];
    auto* moffp = &m.off[0];


    std::vector<size_t> num_idx_per_col(num_vnnz,0);
    auto* num_idx_per_col_ptr = &num_idx_per_col[0];

    size_t num_idx = 0;
    #pragma cdir nodep
    #pragma cdir on_adb(num_idx_per_col_ptr)    
    for(size_t c=0; c<num_vnnz;c++){ 
      auto col_idx = vidxp[c]; 
      num_idx_per_col_ptr[c] = moffp[col_idx+1] - moffp[col_idx];
      num_idx += moffp[col_idx+1] - moffp[col_idx];
    }

    std::vector<T> idx_buff(num_idx,0);
    auto* idx_buff_ptr = &idx_buff[0];
    
    if(num_idx > HYB_TH_NUM_ELEM_TOTAL || num_vnnz > HYB_TH_NUM_VEC){
        //Using spMspV-Scatter 
 
        std::vector<T> BufferVec(m.local_num_row,0);//upper bound of the output vector size

        auto* BufferVec_ptr = &BufferVec[0];        

        for(size_t c=0; c<num_vnnz;c++){ 

         #pragma cdir nodep
         #pragma _NEC ivdep
         #pragma cdir on_adb(vvalp)
         #pragma cdir on_adb(vidxp)
            for(size_t r = moffp[ vidxp[c]]; r<moffp[ vidxp[c]+1];r++){ 
                BufferVec_ptr[midxp[r]] = BufferVec_ptr[midxp[r]] + mvalp[r];              
            }
        }

        std::vector<T> nodes_next_idx_temp(num_idx,0); 
        auto* nodes_next_idx_temp_ptr = &nodes_next_idx_temp[0];
        size_t idx_current = 0;

        #pragma _NEC ivdep
         for(size_t i=0;i<m_local_num_row;i++){
             if(BufferVec_ptr[i] != 0){          
                 nodes_next_idx_temp_ptr[idx_current++] = i + node_offset;
             }
         }

         std::vector<T> nodes_next_idx(idx_current,0);
         auto* nodes_next_idx_ptr = &nodes_next_idx[0];
         for(size_t i = 0;i < idx_current; i ++){
             nodes_next_idx_ptr[i] = nodes_next_idx_temp_ptr[i];
         }
 
         return nodes_next_idx;        
    }

    std::vector<size_t> vec_idx(num_vnnz,0);
    std::vector<size_t> vec_idx_stop(num_vnnz,0);
    auto* vec_idx_ptr = &vec_idx[0];
    auto* vec_idx_stop_ptr = &vec_idx_stop[0];
    
    vec_idx_ptr[0] = 0;
    vec_idx_stop_ptr[0] = num_idx_per_col_ptr[0];

    for(size_t i=1; i<num_vnnz; i++){
        vec_idx_ptr[i] = vec_idx_stop_ptr[i-1];
        vec_idx_stop_ptr[i] = vec_idx_ptr[i] + num_idx_per_col_ptr[i];    
    }
    

    size_t col_idx; 
    size_t idx_per_col_current;
        
    for(size_t c=0; c<num_vnnz;c++){ 
        col_idx = vidxp[c]; 
        idx_per_col_current = 0;

    #pragma cdir nodep
    #pragma cdir on_adb(idx_buff_ptr)
    #pragma cdir on_adb(vec_idx)
        for(size_t r = moffp[col_idx]; r<moffp[col_idx+1];r++){
            idx_buff_ptr[vec_idx_ptr[c]+idx_per_col_current++] = midxp[r] + node_offset;
        }
    }

    std::vector<T> ret_idx;


    if(num_vnnz > 1){
        ret_idx = set_union_2d_idx_lowd<T>(idx_buff,num_idx_per_col, vec_idx,merge_th);
    }
    else {        
        ret_idx = idx_buff;
    }

    return ret_idx; //Currently only return ret_idx without ret_valp;
}


template <class O, class T, class B>
std::vector<T> 
spMspV_2DMerge_unvisited(frovedis::ccs_matrix_local<O>& m, 
                                        std::vector<T>& vidx, 
                                        std::vector<size_t>& nodes_offset, 
                                        size_t& merge_th, 
                                        std::vector<B>& lnodes_ifVisited) {
    //update and check ifVisited locally

    size_t node_offset = nodes_offset[frovedis::get_selfid()];

    auto* vidxp = &vidx[0];
    auto num_vnnz = vidx.size(); 
    auto m_local_num_row = m.local_num_row;
    auto* midxp = &m.idx[0];
    auto* moffp = &m.off[0];


    std::vector<size_t> num_idx_per_col(num_vnnz,0);
    auto* num_idx_per_col_ptr = &num_idx_per_col[0];
    size_t num_idx = 0;
    
    #pragma cdir nodep
    #pragma cdir on_adb(num_idx_per_col_ptr)  
    for(size_t c=0; c<num_vnnz;c++){ 
        auto col_idx = vidxp[c]; 
        num_idx_per_col_ptr[c] = moffp[col_idx+1] - moffp[col_idx];
        num_idx += moffp[col_idx+1] - moffp[col_idx];
    }
    std::vector<T> idx_buff(num_idx,0);
    auto* idx_buff_ptr = &idx_buff[0];
    std::vector<size_t> vec_idx(num_vnnz,0);
    std::vector<size_t> vec_idx_stop(num_vnnz,0);
    auto* vec_idx_ptr = &vec_idx[0];
    auto* vec_idx_stop_ptr = &vec_idx_stop[0];
    
    vec_idx_ptr[0] = 0;
    vec_idx_stop_ptr[0] = num_idx_per_col_ptr[0];
    for(size_t i=1; i<num_vnnz; i++){
        vec_idx_ptr[i] = vec_idx_stop_ptr[i-1];
        vec_idx_stop_ptr[i] = vec_idx_ptr[i] + num_idx_per_col_ptr[i];       
    }

    size_t col_idx; 
    size_t idx_per_col_current;
        
    for(size_t c=0; c<num_vnnz;c++){ 
        col_idx = vidxp[c]; 
        idx_per_col_current = 0;

    #pragma cdir nodep
    #pragma cdir on_adb(idx_buff_ptr)
    #pragma cdir on_adb(vec_idx)
        for(size_t r = moffp[col_idx]; r<moffp[col_idx+1];r++){
            idx_buff_ptr[vec_idx_ptr[c]+idx_per_col_current++] = midxp[r] + node_offset;
        }
    }

    std::vector<T> ret_idx;
    if(num_vnnz > 1){
        ret_idx = set_union_2d_idx_highd<T>(idx_buff,num_idx_per_col,vec_idx, merge_th);
    }
    else {
        ret_idx = idx_buff;
    } 
  
    //update and check ifVisited
    auto* ret_idx_ptr = &ret_idx[0];
    auto* lnodes_ifVisited_ptr = &lnodes_ifVisited[0];
    for(size_t i=0; i<vidx.size();i++){
        if(vidxp[i] >= node_offset && vidxp[i] < node_offset + m_local_num_row){
            lnodes_ifVisited_ptr[vidxp[i]-node_offset] = 1;
        }
    }
    auto* ret_idx_unvisited_temp_ptr = new size_t[ret_idx.size()];
    size_t ret_idx_count = 0;
    for(size_t i=0; i<ret_idx.size();i++){
        if(lnodes_ifVisited_ptr[ret_idx_ptr[i]-node_offset] == 0){
            ret_idx_unvisited_temp_ptr[ret_idx_count++] = ret_idx_ptr[i];
        }
    }
    std::vector<T> ret_idx_unvisited(ret_idx_count);
    auto* ret_idx_unvisited_ptr = &ret_idx_unvisited[0];
    for(size_t i=0; i<ret_idx_count;i++){
        ret_idx_unvisited_ptr[i] = ret_idx_unvisited_temp_ptr[i];

    }    
    delete ret_idx_unvisited_temp_ptr; 
    return ret_idx_unvisited; //Currently only return ret_idx without ret_valp;

}
template <class T>
std::vector<T>
Allgather_next_idx(std::vector<T>& ret_idx_unvisited){
       //allgather to exchange ret_idx_unvisited among workers.
    
    
    size_t num_worker = frovedis::get_nodesize();
    auto* ret_idx_unvisited_ptr = &ret_idx_unvisited[0];
    size_t send_count = ret_idx_unvisited.size();
    int send_data[1];
    send_data[0] = send_count;
    
    int recv_counts[num_worker];       

 
    MPI_Allgather(send_data, 1, MPI_INT, recv_counts, 1, MPI_INT, frovedis_comm_rpc);
    
    int displs[num_worker];   
    size_t num_ret_idx_unvisited_all = 0;
    for(size_t i=0; i<num_worker; i++){
        displs[i] = num_ret_idx_unvisited_all;
        num_ret_idx_unvisited_all += recv_counts[i];
    }
    
    std::vector<T> ret_idx_unvisited_all(num_ret_idx_unvisited_all);
    if(num_ret_idx_unvisited_all == 0){ 
        return ret_idx_unvisited_all;
    }
    else{
        auto* ret_idx_unvisited_all_ptr = &ret_idx_unvisited_all[0];

        MPI_Allgatherv(ret_idx_unvisited_ptr, 
                send_count, 
                MPI_UNSIGNED_LONG_LONG, 
                ret_idx_unvisited_all_ptr, 
                recv_counts,
                displs,
                MPI_UNSIGNED_LONG_LONG,
                frovedis_comm_rpc);

        return ret_idx_unvisited_all;
    }
}
}
#endif /* GRAPH_CONNECTED_COMPONENTS_HPP*/

