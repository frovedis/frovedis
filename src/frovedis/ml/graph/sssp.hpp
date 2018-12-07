
#ifndef GRAPH_SSSP_HPP
#define	GRAPH_SSSP_HPP

#include "sparse_vector.hpp"
#include "global.hpp"
namespace frovedis {
    

template <class O>
std::vector<O> 
bellmanford_spmv(frovedis::ccs_matrix_local<O>& m, 
                    std::vector<O>& vdist, 
                    std::vector<size_t>& vpred,
                    std::vector<size_t>& nodes_offset, 
                    std::vector<int>& isConverged ){
//Bellman-Ford algorithm to find sssp
//CCS matrix, dense vector
   size_t node_offset = nodes_offset[frovedis::get_selfid()];
    auto* vdistp = &vdist[0];
    auto* vpredp = &vpred[0]; 
    auto m_local_num_row = m.local_num_row;
    auto m_local_num_col = m.local_num_col;
    auto* mvalp = &m.val[0];
    auto* midxp = &m.idx[0];
    auto* moffp = &m.off[0];
    std::vector <O> ret_vdist(m_local_num_row,0);
    auto* ret_vdist_ptr = &ret_vdist[0];    
    for(size_t i = 0; i< m_local_num_row; i++){
        ret_vdist_ptr[i] = vdistp[node_offset + i];
    }
    size_t num_dist_changed = 0;
    isConverged[0] = 1;
  
    int lt;
    int gt0;

    
    for(size_t c=0; c<m_local_num_col;c++){ //TODO: cannot be vectorized due to vpred and ret_vdist_ptr all have dependency (simultaneous write)
        #pragma cdir on_adb(ret_vdist_ptr)        
        #pragma cdir nodep
        #pragma _NEC ivdep
        for(size_t r = moffp[c]; r<moffp[c+1];r++){
            lt = vdistp[c] + mvalp[r] < ret_vdist_ptr[midxp[r]];          
            gt0 = vdistp[c] + mvalp[r] >= 0;              
            if(lt && gt0){
                ret_vdist_ptr[midxp[r]] = vdistp[c] + mvalp[r];
                vpredp[midxp[r]] = c;
                isConverged[0] = 0;                //set a flag to show whether there is a change in current iteration
                num_dist_changed ++;          
            }

        }
    }
  return ret_vdist;
}

template <class O>
sparse_vector_tri<O> 
bellmanford_spmspv_sort(frovedis::ccs_matrix_local<O>& m, 
                    sparse_vector_tri<O>& vdist_current_sp,
                    std::vector<O>& vdist, //local copy of the whole vector
                    std::vector<size_t>& vpred, //only part of the whole vector
                    std::vector<size_t>& nodes_offset
//                    std::vector<int>& isConverged 
                    ){
//Bellman-Ford algorithm to find sssp
//sparse vector_tri
    
    size_t node_offset = nodes_offset[frovedis::get_selfid()];
    size_t num_vnnz = vdist_current_sp.idx.size();
    
    auto* vdist_current_idxp = &vdist_current_sp.idx[0];
    auto* vdist_current_valp = &vdist_current_sp.val[0];
    auto* vdist_current_predp = &vdist_current_sp.pred[0];
    auto* vdistp = &vdist[0]; //whole vdist
    auto* vpredp = &vpred[0]; //local vpred
    auto m_local_num_row = m.local_num_row;
    auto* mvalp = &m.val[0];
    auto* midxp = &m.idx[0];
    auto* moffp = &m.off[0];

    //update local vdist and vpred before computation
    for(size_t i=0; i<num_vnnz;i++){
        vdistp[vdist_current_idxp[i]] = vdist_current_valp[i];
        if(vdist_current_idxp[i] >=node_offset && vdist_current_idxp[i] < node_offset + m_local_num_row){
            vpredp[vdist_current_idxp[i]-node_offset] = vdist_current_predp[i];
        }
    }
    std::vector <O> ret_vdist(m_local_num_row,0);
    auto* ret_vdist_ptr = &ret_vdist[0];
    for(size_t i = 0; i< m_local_num_row; i++){
        ret_vdist_ptr[i] = vdistp[node_offset + i];
    }
    size_t num_dist_changed = 0;
    size_t num_elem_temp = m_local_num_row*5; //estimate max possible num of elem
    std::vector <O> idx_changed_temp(num_elem_temp,0);
    auto* idx_changed_temp_ptr = &idx_changed_temp[0];
    size_t elem_count = 0;
    #pragma cdir nodep  
    for(size_t c=0; c<num_vnnz;c++){ 
        auto col_idx = vdist_current_idxp[c]; 

        #pragma _NEC ivdep    
        #pragma cdir nodep
        for(size_t r = moffp[col_idx]; r<moffp[col_idx+1];r++){
            
            int lt = vdist_current_valp[c] + mvalp[r] < ret_vdist_ptr[midxp[r]];         
            int gt0 = vdist_current_valp[c] + mvalp[r] >= 0; 

            if(lt && gt0){                
                ret_vdist_ptr[midxp[r]] = vdist_current_valp[c] + mvalp[r];
                vpredp[midxp[r]] = col_idx;
                idx_changed_temp_ptr[elem_count] = midxp[r] + node_offset;
                elem_count ++;          
            }
        }
    }
    
    std::vector<size_t> idx_changed_duplicate(elem_count,0);
    
    for(size_t i=0; i<elem_count;i++){
        idx_changed_duplicate[i] = idx_changed_temp_ptr[i];
    }
// #ifndef _SX   
#if !defined(_SX) && !defined(__ve__)
    std::sort(idx_changed_duplicate.begin(), idx_changed_duplicate.end());
#else
    frovedis::radix_sort(idx_changed_duplicate,idx_changed_duplicate);
#endif
    auto idx_changed_sep = set_separate(idx_changed_duplicate);  //TODO: CANNOT SIMPLY SEPARATE, NEED TO COMPARE THE DISTANCE 
    size_t num_elem_neat =idx_changed_sep.size()-1;
    num_dist_changed = num_elem_neat;

    if(num_elem_neat > 0){
        sparse_vector_tri<O> vdist_sp(num_elem_neat,0);
        for(size_t i=0; i<num_elem_neat;i++){
            vdist_sp.idx[i] = idx_changed_duplicate[idx_changed_sep[i]];
            vdist_sp.val[i] = ret_vdist_ptr[idx_changed_duplicate[idx_changed_sep[i]]-node_offset]; //already the shortest distance stored in vdist, no need to find shortest in set_separate 
            vdist_sp.pred[i] = vpredp[idx_changed_duplicate[idx_changed_sep[i]]-node_offset];
        }    
    
        num_dist_changed = vdist_sp.idx.size();

     RLOG(TIME_RECORD_LOG_LEVEL)<<"worker "<<frovedis::get_selfid()
            <<", num_dist_changed = "<<num_dist_changed
            <<std::endl;
    return vdist_sp;
    }else{
    sparse_vector_tri<O> vdist_sp;
    RLOG(TIME_RECORD_LOG_LEVEL)<<"worker "<<frovedis::get_selfid()
            <<", num_dist_changed = "<<num_dist_changed
            <<std::endl;
    return vdist_sp;
    }

}

template <class O>
sparse_vector_tri<O> 
bellmanford_spmspv_merge(frovedis::ccs_matrix_local<O>& m, 
                    sparse_vector_tri<O>& vdist_current_sp,
                    std::vector<O>& vdist,
                    std::vector<size_t>& vpred,
                    std::vector<size_t>& nodes_offset,
                    size_t& upperbound
                    ){
    //Bellman-Ford algorithm to find sssp
    //sparse vector_tri
    
    size_t node_offset = nodes_offset[frovedis::get_selfid()];
    size_t num_vnnz = vdist_current_sp.idx.size();
    
    auto* vdist_current_idxp = &vdist_current_sp.idx[0];
    auto* vdist_current_valp = &vdist_current_sp.val[0];
    auto* vdist_current_predp = &vdist_current_sp.pred[0];
    
    auto* vdistp = &vdist[0]; //whole vdist
    auto* vpredp = &vpred[0]; //local vpred

    auto m_local_num_row = m.local_num_row;
    auto* mvalp = &m.val[0];
    auto* midxp = &m.idx[0];
    auto* moffp = &m.off[0];
    size_t num_elem_max = 0;    
    
    #pragma cdir nodep 
    for(size_t c=0; c<num_vnnz;c++){ 
      auto col_idx = vdist_current_idxp[c]; 
      num_elem_max += moffp[col_idx+1] - moffp[col_idx];
    }
    
    std::vector<size_t> vec_idx(num_vnnz,0);

    auto* vec_idx_ptr = &vec_idx[0];
    sparse_vector <O> ret_vdist_multiVec_buff(num_elem_max,0); //first initial a buffer that is big enough
    auto* ret_vdist_singlebuff_temp_idx_ptr = &ret_vdist_multiVec_buff.idx[0];
    auto* ret_vdist_singlebuff_temp_val_ptr = &ret_vdist_multiVec_buff.val[0];
    
    //update local vdist and vpred before computation
    for(size_t i=0; i<num_vnnz;i++){
        vdistp[vdist_current_idxp[i]] = vdist_current_valp[i];
        if(vdist_current_idxp[i] >=node_offset && vdist_current_idxp[i] < node_offset + m_local_num_row){
            vpredp[vdist_current_idxp[i]-node_offset] = vdist_current_predp[i];
        }
    }

    size_t buff_idx = 0;
    
    #pragma cdir nodep  
    for(size_t c=0; c<num_vnnz;c++){ 
        auto col_idx = vdist_current_idxp[c]; 
        vec_idx_ptr[c] = buff_idx;
        #pragma _NEC ivdep        
        #pragma cdir nodep
        for(size_t r = moffp[col_idx]; r<moffp[col_idx+1];r++){
            
            int lt = vdist_current_valp[c] + mvalp[r] < vdistp[node_offset + midxp[r]];         
            int gt0 = vdist_current_valp[c] + mvalp[r] >= 0;   //prevent overflow
        
            if(lt && gt0){                  
                ret_vdist_singlebuff_temp_idx_ptr[buff_idx] = midxp[r] + node_offset;
                ret_vdist_singlebuff_temp_val_ptr[buff_idx] = vdistp[vdist_current_idxp[c]] + mvalp[r];
                buff_idx ++;          
            }
        }
    }
    
    size_t num_dist_changed = 0;
    sparse_vector_tri<O> ret;
    if(buff_idx > 0){
        sparse_vector <O> ret_vdist_singlebuff(buff_idx,0); //Copy result to the final buffer with exact size
        auto* ret_vdist_singlebuff_idx_ptr = &ret_vdist_singlebuff.idx[0];
        auto* ret_vdist_SingleBuff_val_ptr = &ret_vdist_singlebuff.val[0];

        for(size_t i=0; i<buff_idx;i++){
            ret_vdist_singlebuff_idx_ptr[i] = ret_vdist_singlebuff_temp_idx_ptr[i];
            ret_vdist_SingleBuff_val_ptr[i] = ret_vdist_singlebuff_temp_val_ptr[i];
        }
    

//    auto ret_vdist = set_union_horizontal_relax<O>(ret_vdist_singlebuff,vec_idx, vdist_current_sp.idx, merge_th);

        ret = set_union_2d_sssp<O>(ret_vdist_singlebuff,vec_idx, vdist_current_sp.idx, upperbound);
        num_dist_changed = ret.idx.size();
    }
    
        std::cout<<"worker "<<frovedis::get_selfid()
        <<", num_dist_changed = "<<num_dist_changed
        <<std::endl;  

    return ret;

}
}
#endif	/* GRAPH_SSSP_HPP*/

