
#ifndef GRAPH_SSSP_HPP
#define	GRAPH_SSSP_HPP

#include "sparse_vector.hpp"
#include "global.hpp"

namespace frovedis {

template <class T, class I, class O>
crs_matrix_local<T, I, O>
construct_min_crs_matrix(crs_matrix_local<T, I, O>& mat,
                         std::vector<I>& rowid,
                         std::vector<T>& dist,
                         std::vector<I>& target_dest_ids,
                         size_t myst) {
  auto nrow = mat.local_num_row;
  auto nedges = mat.val.size();
  auto mvalp = mat.val.data();
  auto sidp = mat.idx.data();
  auto didp = rowid.data();
  auto distp = dist.data();
  auto Tmax = std::numeric_limits<T>::max();
  std::vector<T> minval(nedges); auto minvalp = minval.data();
  std::vector<I> minidx(nedges); auto minidxp = minidx.data();
  std::vector<O> tmpvec(nrow);   auto tmpvecp = tmpvec.data();
  size_t count = 0;
  for(size_t i = 0; i < nedges; ++i) {
    auto src = sidp[i];
    auto dst = didp[i];
    if(distp[src] != Tmax && ((distp[src] + mvalp[i]) < distp[dst + myst])) {
      minvalp[count] = distp[src] + mvalp[i];
      minidxp[count] = src;
      count++;
      tmpvecp[dst] = count;
    }
  }

  size_t k = 0;
  auto targetp = target_dest_ids.data();
  for(size_t dst = 0; dst < nrow; ++dst) if (tmpvecp[dst] != 0) targetp[k++] = dst;

  std::vector<O> countvec(k + 1); auto cntvecp = countvec.data();
  for(size_t i = 0; i < k; ++i) cntvecp[i+1] = tmpvecp[targetp[i]];
  O max_freq = 0;
  for(size_t i = 0; i < k; ++i) {
    auto freq = cntvecp[i+1] - cntvecp[i];
    if (freq > max_freq) max_freq = freq;
  }

  crs_matrix_local<T,I,O> ret;
  ret.val.swap(minval); // extra-sized (skipping extraction step to avoid copy of short loop-length)
  ret.idx.swap(minidx); // extra-sized (skipping here as well)
  ret.off.swap(countvec);
  ret.local_num_row = k;
  ret.local_num_col = max_freq;
  return ret;
}

template <class T, class I, class O>
std::vector<T>
compute_nextdist(crs_matrix_local<T, I, O>& mat,
                 std::vector<T>& dist,
                 std::vector<I>& pred,
                 std::vector<I>& target_dest_ids,
                 size_t myst) {
  auto nrow = pred.size();
  auto max_freq = mat.local_num_col;
  auto target_dest_cnt = mat.local_num_row;
  auto tmpvalp = mat.val.data();
  auto tmpidxp = mat.idx.data();
  auto tmpoffp = mat.off.data();
  std::vector<T> nextdist(nrow);
  auto nextdistp = nextdist.data();
  auto distp = dist.data();
  auto predp = pred.data();
  auto targetp = target_dest_ids.data();

  for(size_t i = 0; i < nrow; ++i) nextdistp[i] = distp[i + myst]; // initializing nextdist
#pragma _NEC nointerchange
  for(size_t j = 0; j < max_freq; ++j) {
#pragma _NEC ivdep
    for(size_t i = 0; i < target_dest_cnt; ++i) {
      int freq = tmpoffp[i+1] - tmpoffp[i] - j;
      if (freq > 0) { // guarding to check if jth column exist for ith row
        auto target_dest = targetp[i]; // 'target_dest' would be different across i-loop, hence 'ivdep' is applied
        auto src = tmpidxp[tmpoffp[i] + j];
        auto t_dist = tmpvalp[tmpoffp[i] + j];
        if(t_dist < nextdistp[target_dest]) {
          nextdistp[target_dest] = t_dist;
          predp[target_dest] = src;
        }
      }
    }
  }
  return nextdist;
}

template <class T, class I, class O>
std::vector<T>
calc_sssp_bf_impl(crs_matrix_local<T, I, O>& mat,
                  std::vector<I>& rowid,
                  std::vector<T>& dist,
                  std::vector<I>& pred,
                  size_t myst) {
  auto myrank = get_selfid();
  auto nrow = mat.local_num_row;
  std::vector<I> target_dest_ids(nrow);
  time_spent t_compute(TRACE), t_update(TRACE);

  t_compute.lap_start();
  auto tmp_crs_mat = construct_min_crs_matrix(mat, rowid, dist,
                     target_dest_ids, myst);
  t_compute.lap_stop();
  //std::cout << "[" << myrank << "] tmp_crs_mat: "; tmp_crs_mat.debug_print(10);

  t_update.lap_start();
  auto nextdist = compute_nextdist(tmp_crs_mat, dist, pred,
                  target_dest_ids, myst);
  t_update.lap_stop();
  //std::cout << "[" << myrank << "] nextdist: "; debug_print_vector(nextdist, 10);

  if(myrank == 0) {
    RLOG(TRACE) << "target_dest_cnt: " << tmp_crs_mat.local_num_row
                << "; max_freq: " << tmp_crs_mat.local_num_col << std::endl;
    t_compute.show_lap("  - bf compute time: ");
    t_update.show_lap("  - distance update time: ");
  }
  return nextdist;
}

template <class T, class I, class O>
std::vector<I> compute_rowid(crs_matrix_local<T,I,O>& mat) {
  auto nrow = mat.local_num_row;
  auto nelem = mat.val.size();
  std::vector<I> ret(nelem);
  auto offp = mat.off.data();
  auto retp = ret.data();
  for(size_t i = 0; i < nrow; ++i) {
    for(size_t j = offp[i]; j < offp[i+1]; ++j) retp[j] = i;
  }
  return ret;
}

template <class T, class I, class O>
int get_local_nrows_int(crs_matrix_local<T,I,O>& mat) {
  return static_cast<int>(mat.local_num_row);
}

template <class T, class I, class O>
void set_local_ncol(crs_matrix_local<T,I,O>& mat, size_t ncol) {
  mat.local_num_col = ncol;
}

template <class T, class I, class O>
crs_matrix<T,I,O>
check_input(crs_matrix<T,I,O>& mat) {
  auto nrow = mat.num_row;
  auto ncol = mat.num_col;
  if (nrow == ncol) return mat;
  else if(ncol < nrow) {
    auto ret = mat;
    ret.num_row = ret.num_col = nrow;
    ret.data.mapv(set_local_ncol<T,I,O>, broadcast(nrow));
    return ret;
  }
  else REPORT_ERROR(USER_ERROR, "input is not a square matrix!\n");
}

template <class T, class I, class O>
std::vector<T>
calc_sssp_bf(crs_matrix_local<T, I, O>& mat,
             std::vector<I>& rowid,
             std::vector<T>& dist,
             std::vector<I>& pred,
             std::vector<int>& nrows_per_process,
             std::vector<int>& stidx_per_process) {
  auto nrow = mat.local_num_row;
  auto nvert = mat.local_num_col;
  auto myrank = get_selfid();
  auto myst = stidx_per_process[myrank];
  auto step = 1;
  std::vector<T> all_nextdist(nvert);
  auto countp = nrows_per_process.data();
  auto displp = stidx_per_process.data();
  pred.resize(nrow); auto predp = pred.data();
  for(size_t i = 0; i < nrow; ++i) predp[i] = i + myst; // initializing predecessor
  while (step <= nvert) {
    time_spent t_iter(DEBUG), t_com(DEBUG);
    t_iter.lap_start();
    auto mynextdist = calc_sssp_bf_impl(mat, rowid, dist, pred, myst);
    t_com.lap_start();
    typed_allgatherv<T>(mynextdist.data(), nrow,
                        all_nextdist.data(), countp, displp,
                        frovedis_comm_rpc); // all_nextdist = gather all mynextdist process-by-process
    t_com.lap_stop();
    if(myrank == 0) t_com.show_lap("Communication time: ");

    if (all_nextdist == dist) {
      if(myrank == 0) {
        RLOG(INFO) << "Converged in " + std::to_string(step) + " steps!\n";
        t_iter.lap_stop();
        t_iter.show_lap("One iteration: ");
      }
      break;
    }
    dist.swap(all_nextdist); // exchange all_nextdist with dist
    step++;
    t_iter.lap_stop();
    if(myrank == 0) t_iter.show_lap("One iteration: ");
  }
  return dist;
}

template <class T, class I, class O>
void sssp_bf_impl(crs_matrix<T, I, O>& adj_mat, size_t srcid,
                  std::vector<T>& dist,
                  std::vector<I>& pred) {
  auto gr = check_input(adj_mat);
  //std::cout << "input adjacency matrix (src nodes in column): \n";
  //gr.debug_print(10);

  auto nvert = gr.num_col;
  if (srcid >= nvert)
    REPORT_ERROR(USER_ERROR, "source index should be in between 0 and nvert-1!\n");

  // --- initialization step ---
  auto Tmax = std::numeric_limits<T>::max();
  dist.clear(); dist.resize(nvert); auto distp = dist.data(); // output vector
  for(size_t i = 0; i < nvert; ++i) distp[i] = Tmax;
  dist[srcid] = 0;
  // --- initialization related to distributed design ---
  auto rowid = gr.data.map(compute_rowid<T,I,O>);
  auto nrows = gr.data.map(get_local_nrows_int<T,I,O>).gather();
  std::vector<int> sidx(nrows.size()); sidx[0] = 0;
  for(size_t i = 1; i < nrows.size(); ++i) sidx[i] = sidx[i-1] + nrows[i-1];
  auto pred_loc = make_node_local_allocate<std::vector<I>>();
  // --- distributed computation step ---
  dist = gr.data.map(calc_sssp_bf<T,I,O>,
                     rowid, broadcast(dist), pred_loc,
                     broadcast(nrows), broadcast(sidx)).get(0); // all processes have updated distance
  pred = pred_loc.template moveto_dvector<I>().gather();
}

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

