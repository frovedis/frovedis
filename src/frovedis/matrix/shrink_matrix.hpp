#ifndef SHRINK_MATRIX_HPP
#define SHRINK_MATRIX_HPP

#include "crs_matrix.hpp"
#include "rowmajor_matrix.hpp"

#define SMAT_VLEN 256

namespace frovedis {

template <class T, class I, class O>
std::vector<I> shrink_column_local(crs_matrix_local<T,I,O>& m) {
  std::vector<I> checker(m.local_num_col);
  I* checkerp = &checker[0];
  I* idxp = &m.idx[0];
  for(size_t row = 0; row < m.local_num_row; row++) {
#pragma cdir nodep
#pragma _NEC ivdep
    for(O col = m.off[row]; col < m.off[row + 1]; col++) {
      checkerp[idxp[col]] = 1;
    }
  }
  std::vector<I> worktbl(m.local_num_col);
  I* worktblp = &worktbl[0];
  size_t crnt = 0;
  for(size_t i = 0; i < m.local_num_col; i++) {
    if(checkerp[i] == 1) worktblp[crnt++] = i;
  }
  size_t shrink_num_col = crnt;
  std::vector<I> revtbl(m.local_num_col);
  I* revtblp = &revtbl[0];
  for(size_t i = 0; i < shrink_num_col; i++) {
    revtblp[worktbl[i]] = i;
  }
  for(size_t row = 0; row < m.local_num_row; row++) {
    for(O col = m.off[row]; col < m.off[row + 1]; col++) {
      idxp[col] = revtblp[idxp[col]];
    }
  }
  RLOG(DEBUG) << "shrink_column_local: column is shrinked from "
              << m.local_num_col << " to " << shrink_num_col << std::endl;
  m.local_num_col = shrink_num_col;
  std::vector<I> rettbl(shrink_num_col);
  auto rettblp = rettbl.data();
  for(size_t i = 0; i < shrink_num_col; i++) {
    rettblp[i] = worktblp[i];
  }
  return rettbl;
}

template <class T, class I, class O>
node_local<std::vector<I>> shrink_column(crs_matrix<T,I,O>& m) {
  return m.data.map(shrink_column_local<T,I,O>);
}

template <class I>
node_local<std::vector<size_t>> 
partition_shrink_column(node_local<std::vector<I>>& shrink_table, size_t num_col) {
  auto shrink_tables = shrink_table.gather();
  std::vector<size_t> cols(num_col);
  size_t* colsp = &cols[0];
  for(size_t i = 0; i < shrink_tables.size(); i++) {
    auto& each_table = shrink_tables[i];
    I* each_tablep = &each_table[0];
#pragma cdir nodep
#pragma _NEC ivdep
    for(size_t j = 0; j < shrink_tables[i].size(); j++) {
      colsp[each_tablep[j]]++;
    }
  }
  std::vector<size_t> cols_sum(num_col);
  size_t* cols_sump = &cols_sum[0];
  cols_sum[0] = cols[0];
  for(size_t i = 1; i < cols.size(); i++) {
    cols_sump[i] = cols_sump[i-1] + colsp[i];
  }
  auto total = cols_sum[cols_sum.size() - 1];
  auto each = ceil_div(total, static_cast<size_t>(frovedis::get_nodesize()));
  std::vector<size_t> ret_vec(frovedis::get_nodesize()+1);
  ret_vec[0] = 0;
  auto prev_it = cols_sum.begin();
  for(size_t i = 1; i < ret_vec.size(); i++) {
    auto it = std::lower_bound(prev_it, cols_sum.end(), each * (i+1));
    ret_vec[i] = it - cols_sum.begin();
    prev_it = it;
  }  
  return make_node_local_broadcast(ret_vec);
}

template <class I>
struct shrink_vector_info {
  shrink_vector_info(){}
  shrink_vector_info(size_t node_size) : 
    send_size(node_size), recv_size(node_size), 
    send_displ(node_size), recv_displ(node_size) {}
  shrink_vector_info(const shrink_vector_info& a) {
    send_size = a.send_size;
    recv_size = a.recv_size;
    send_displ = a.send_displ;
    recv_displ = a.recv_displ;
    exchange_map = a.exchange_map;
    reduce_size = a.reduce_size;
    bcast_size = a.bcast_size;
  }
  shrink_vector_info(shrink_vector_info&& a) {
    send_size.swap(a.send_size);
    recv_size.swap(a.recv_size);
    send_displ.swap(a.send_displ);
    recv_displ.swap(a.recv_displ);
    exchange_map.swap(a.exchange_map);
    reduce_size = a.reduce_size;
    bcast_size = a.bcast_size;
  }
  shrink_vector_info<I>& operator=(const shrink_vector_info& a) {
    send_size = a.send_size;
    recv_size = a.recv_size;
    send_displ = a.send_displ;
    recv_displ = a.recv_displ;
    exchange_map = a.exchange_map;
    reduce_size = a.reduce_size;
    bcast_size = a.bcast_size;
    return *this;
  }
  shrink_vector_info<I>& operator=(shrink_vector_info&& a) {
    send_size.swap(a.send_size);
    recv_size.swap(a.recv_size);
    send_displ.swap(a.send_displ);
    recv_displ.swap(a.recv_displ);
    exchange_map.swap(a.exchange_map);
    reduce_size = a.reduce_size;
    bcast_size = a.bcast_size;
    return *this;
  }
  
  // send/recv is in the case of reduce; opposite in the case of bcast
  std::vector<size_t> send_size;
  std::vector<size_t> recv_size;
  std::vector<size_t> send_displ;
  std::vector<size_t> recv_displ;
  std::vector<I> exchange_map;
  size_t reduce_size;
  size_t bcast_size;
  void debug_print() {
    std::cout << "send_size = ";
    for(auto i: send_size) std::cout << i << " ";
    std::cout << std::endl;
    std::cout << "recv_size = ";
    for(auto i: recv_size) std::cout << i << " ";
    std::cout << std::endl;
    std::cout << "send_displ = ";
    for(auto i: send_displ) std::cout << i << " ";
    std::cout << std::endl;
    std::cout << "recv_displ = ";
    for(auto i: recv_displ) std::cout << i << " ";
    std::cout << std::endl;
    std::cout << "exchange_map = ";
    for(auto i: exchange_map) std::cout << i << " ";
    std::cout << std::endl;
    std::cout << "reduce_size = " << reduce_size << std::endl;
  }
  SERIALIZE(send_size, recv_size, send_displ, recv_displ, exchange_map,
            reduce_size, bcast_size)
};

template <class I>
shrink_vector_info<I> 
create_shrink_vector_info_local(std::vector<I>& shrink_table,
                                std::vector<I>& column_partition) {
  size_t node_size = frovedis::get_nodesize();
  shrink_vector_info<I> ret(node_size);
  auto prev_it = shrink_table.begin();
  for(size_t i = 0; i < node_size; i++) {
    auto it = std::lower_bound(shrink_table.begin(), shrink_table.end(), 
                               column_partition[i+1]);
    ret.send_size[i] = it - prev_it;
    prev_it = it;
  }
  MPI_Alltoall(&ret.send_size[0], sizeof(size_t), MPI_CHAR,
               &ret.recv_size[0], sizeof(size_t), MPI_CHAR, MPI_COMM_WORLD);
  size_t total_size = 0;
  auto retrecv_sizep = ret.recv_size.data();
  for(size_t i = 0; i < node_size; i++) {
    total_size += retrecv_sizep[i];
  }
  auto retsend_displp = ret.send_displ.data();
  auto retrecv_displp = ret.recv_displ.data();
  auto retsend_sizep = ret.send_size.data();
  retsend_displp[0] = 0; retrecv_displp[0] = 0;
  for(size_t i = 1; i < node_size; i++) {
    retsend_displp[i] = retsend_displp[i-1] + retsend_sizep[i-1];
    retrecv_displp[i] = retrecv_displp[i-1] + retrecv_sizep[i-1];
  }
  ret.exchange_map.resize(total_size);
  large_alltoallv(sizeof(I),
                  reinterpret_cast<char*>(const_cast<I*>(&shrink_table[0])),
                  ret.send_size, ret.send_displ, 
                  reinterpret_cast<char*>
                  (const_cast<I*>(&ret.exchange_map[0])),
                  ret.recv_size, ret.recv_displ, 
                  MPI_COMM_WORLD);
  size_t self = frovedis::get_selfid();
  auto retexchange_mapp = ret.exchange_map.data();
  auto column_partitionp = column_partition.data();
  for(size_t i = 0; i < total_size; i++) {
    retexchange_mapp[i] -= column_partitionp[self];
  }
  ret.reduce_size = column_partitionp[self + 1] - column_partitionp[self];
  ret.bcast_size = shrink_table.size();
  return ret;
}

template <class I>
node_local<shrink_vector_info<I>>
create_shrink_vector_info(node_local<std::vector<I>>& shrink_table,
                          node_local<std::vector<I>>& column_partition) {
  return shrink_table.map(create_shrink_vector_info_local<I>, 
                          column_partition);
}

template <class T, class I, class O>
node_local<shrink_vector_info<I>> 
prepare_shrink_comm(node_local<std::vector<I>>& tbl, crs_matrix<T,I,O>& m) {
  auto v = partition_shrink_column(tbl, m.num_col);
  return create_shrink_vector_info(tbl, v);
}

template <class I>
size_t get_scatter_size_helper(shrink_vector_info<I>& i) {
  return i.reduce_size;
}

template <class I>
std::vector<I> get_scatter_size(node_local<shrink_vector_info<I>>& info){
  return info.map(get_scatter_size_helper<I>).gather();
}

template <class T, class I>
std::vector<T> shrink_vector_sum_local(std::vector<T>& v,
                                       shrink_vector_info<I>& vm) {
  std::vector<T> ret(vm.reduce_size);
  std::vector<T> work(vm.exchange_map.size());
  large_alltoallv(sizeof(T),
                  reinterpret_cast<char*>(const_cast<T*>(&v[0])),
                  vm.send_size, vm.send_displ, 
                  reinterpret_cast<char*>(const_cast<T*>(&work[0])),
                  vm.recv_size, vm.recv_displ, 
                  MPI_COMM_WORLD);
  T* retp = &ret[0];
  size_t node_size = frovedis::get_nodesize();
  std::vector<T*> current_work_vec(node_size);
  std::vector<I*> current_ex_vec(node_size);
  std::vector<ssize_t> current_size(node_size); // make it signed for fail safe
  auto current_work_vecp = current_work_vec.data();
  auto vmrecv_displp = vm.recv_displ.data();
  auto current_ex_vecp = current_ex_vec.data();
  auto current_sizep = current_size.data();
  auto vmrecv_sizep = vm.recv_size.data();
  for(size_t i = 0; i < node_size; i++) {
    current_work_vecp[i] = &work[0] + vmrecv_displp[i];
    current_ex_vecp[i] = &vm.exchange_map[0] + vmrecv_displp[i];
    current_sizep[i] = vmrecv_sizep[i];
  }
  ssize_t max_len = 0;
  for(size_t i = 0; i < node_size; i++)
    if(max_len < vmrecv_sizep[i]) max_len = vmrecv_sizep[i];
  ssize_t num_block = ceil_div<ssize_t>(max_len, SMAT_VLEN);
  for(size_t b = 0; b < num_block; b++) {
    for(size_t i = 0; i < node_size; i++) {
      T* current_workp = current_work_vecp[i];
      I* current_exp = current_ex_vecp[i];
      int work_len =
        current_sizep[i] > SMAT_VLEN ? SMAT_VLEN : current_sizep[i];
#pragma cdir nodep
#pragma _NEC ivdep
      for(int j = 0; j < work_len; j++) {
        retp[current_exp[j]] += current_workp[j];
      }
      current_work_vecp[i] += work_len;
      current_ex_vecp[i] += work_len;
      current_sizep[i] -= work_len;
    }
  }
  return ret;
}

template <class T, class I>
dvector<T> shrink_vector_sum(node_local<std::vector<T>>& v,
                             node_local<shrink_vector_info<I>>& vm) {
  return v.map(shrink_vector_sum_local<T,I>, vm).template moveto_dvector<T>();
}

template <class T, class I>
std::vector<T> shrink_vector_bcast_local(std::vector<T>& v,
                                         shrink_vector_info<I>& vm) {
  std::vector<T> ret(vm.bcast_size);
  std::vector<T> work(vm.exchange_map.size());
  T* vp = &v[0];
  size_t node_size = frovedis::get_nodesize();
  std::vector<T*> current_work_vec(node_size);
  std::vector<I*> current_ex_vec(node_size);
  std::vector<ssize_t> current_size(node_size);
  auto current_work_vecp = current_work_vec.data();
  auto current_ex_vecp = current_ex_vec.data();
  auto current_sizep = current_size.data();
  auto vmrecv_displp = vm.recv_displ.data();
  auto vmrecv_sizep = vm.recv_size.data();
  for(size_t i = 0; i < node_size; i++) {
    current_work_vecp[i] = &work[0] + vmrecv_displp[i];
    current_ex_vecp[i] = &vm.exchange_map[0] + vmrecv_displp[i];
    current_sizep[i] = vmrecv_sizep[i];
  }
  ssize_t max_len = 0;
  for(size_t i = 0; i < node_size; i++)
    if(max_len < vmrecv_sizep[i]) max_len = vmrecv_sizep[i];
  ssize_t num_block = ceil_div<ssize_t>(max_len, SMAT_VLEN);
  for(size_t b = 0; b < num_block; b++) {
    for(size_t i = 0; i < node_size; i++) {
      T* current_workp = current_work_vecp[i];
      I* current_exp = current_ex_vecp[i];
      int work_len =
        current_sizep[i] > SMAT_VLEN ? SMAT_VLEN : current_sizep[i];
#pragma cdir nodep
#pragma _NEC ivdep
      for(int j = 0; j < work_len; j++) {
        current_workp[j] = vp[current_exp[j]];
      }
      current_work_vecp[i] += work_len;
      current_ex_vecp[i] += work_len;
      current_sizep[i] -= work_len;
    }
  }
  // note that send/recv is opposite
  large_alltoallv(sizeof(T),
                  reinterpret_cast<char*>(const_cast<T*>(&work[0])),
                  vm.recv_size, vm.recv_displ, 
                  reinterpret_cast<char*>(const_cast<T*>(&ret[0])),
                  vm.send_size, vm.send_displ, 
                  MPI_COMM_WORLD);
  return ret;
}

template <class T, class I>
node_local<std::vector<T>> shrink_vector_bcast(dvector<T>& v,
                                         node_local<shrink_vector_info<I>>& vm) {
  return v.map_partitions(shrink_vector_bcast_local<T,I>, vm).moveto_node_local();
}

template <class T, class I>
std::vector<T> shrink_vector_merge_local(std::vector<T>& v,
                                         shrink_vector_info<I>& vm) {
  std::vector<T> ret(vm.reduce_size);
  std::vector<T> work(vm.exchange_map.size());
  large_alltoallv(sizeof(T),
                  reinterpret_cast<char*>(const_cast<T*>(&v[0])),
                  vm.send_size, vm.send_displ, 
                  reinterpret_cast<char*>(const_cast<T*>(&work[0])),
                  vm.recv_size, vm.recv_displ, 
                  MPI_COMM_WORLD);
  T* retp = &ret[0];
  size_t node_size = frovedis::get_nodesize();
  for(size_t i = 0; i < node_size; i++) {
    T* current_workp = &work[0] + vm.recv_displ[i];
    I* current_exp = &vm.exchange_map[0] + vm.recv_displ[i];
#pragma cdir nodep
#pragma _NEC ivdep
    for(size_t j = 0; j < vm.recv_size[i]; j++) {
      // only difference from shrink_dvector_sum
      retp[current_exp[j]] = current_workp[j]; 
    }
  }
  return ret;
}

template <class T, class I>
dvector<T> shrink_vector_merge(node_local<std::vector<T>>& v,
                               node_local<shrink_vector_info<I>>& vm) {
  return v.map(shrink_vector_merge_local<T,I>, vm).
    template moveto_dvector<T>();
}

template <class T, class I>
std::vector<T> shrink_vector_local(std::vector<T>& v,
                                   std::vector<I>& tbl) {
  std::vector<T> ret(tbl.size());
  T* vp = &v[0];
  T* retp = &ret[0];
  I* tblp = &tbl[0];
  for(size_t i = 0; i < tbl.size(); i++) {
    retp[i] = vp[tblp[i]];
  }
  return ret;
}

template <class T, class I>
node_local<std::vector<T>> shrink_vector(node_local<std::vector<T>>& v,
                                   node_local<std::vector<I>>& tbl) {
  return v.map(shrink_vector_local<T,I>, tbl);
}

template <class T, class I>
rowmajor_matrix_local<T>
shrink_rowmajor_matrix_sum_local(rowmajor_matrix_local<T>& m,
                                 shrink_vector_info<I>& vm) {
  size_t num_col = m.local_num_col;
  rowmajor_matrix_local<T> ret(vm.reduce_size, num_col);
  std::vector<T> work(vm.exchange_map.size() * num_col);
  large_alltoallv(sizeof(T) * num_col,
                  reinterpret_cast<char*>(const_cast<T*>(&m.val[0])),
                  vm.send_size, vm.send_displ, 
                  reinterpret_cast<char*>(const_cast<T*>(&work[0])),
                  vm.recv_size, vm.recv_displ, 
                  MPI_COMM_WORLD);
  T* retp = &ret.val[0];
  size_t node_size = frovedis::get_nodesize();
  std::vector<T*> current_work_vec(node_size);
  std::vector<I*> current_ex_vec(node_size);
  std::vector<ssize_t> current_size(node_size); // make it signed for fail safe
  auto current_work_vecp = current_work_vec.data();
  auto current_ex_vecp = current_ex_vec.data();
  auto current_sizep = current_size.data();
  auto vmrecv_displp = vm.recv_displ.data();
  auto vmrecv_sizep = vm.recv_size.data();
  for(size_t i = 0; i < node_size; i++) {
    current_work_vecp[i] = &work[0] + vmrecv_displp[i] * num_col;
    current_ex_vecp[i] = &vm.exchange_map[0] + vmrecv_displp[i];
    current_sizep[i] = vmrecv_sizep[i];
  }
  ssize_t max_len = 0;
  for(size_t i = 0; i < node_size; i++)
    if(max_len < vmrecv_sizep[i]) max_len = vmrecv_sizep[i];
  ssize_t num_block = ceil_div<ssize_t>(max_len, SMAT_VLEN);
  for(size_t b = 0; b < num_block; b++) {
    for(size_t i = 0; i < node_size; i++) {
      T* current_workp = current_work_vecp[i];
      I* current_exp = current_ex_vecp[i];
      int work_len =
        current_sizep[i] > SMAT_VLEN ? SMAT_VLEN : current_sizep[i];
      for(size_t k = 0; k < num_col; k++) {
#pragma cdir nodep
#pragma _NEC ivdep
        for(int j = 0; j < work_len; j++) {
          retp[num_col * current_exp[j] + k] += current_workp[num_col * j + k];
        }
      }
      current_work_vecp[i] += work_len * num_col;
      current_ex_vecp[i] += work_len;
      current_sizep[i] -= work_len;
    }
  }
  return ret;
}

template <class T>
std::pair<size_t, size_t> rowmajor_get_local_num_row_col(rowmajor_matrix_local<T>& lm) {
  return std::make_pair(lm.local_num_row, lm.local_num_col);
}

template <class T, class I>
rowmajor_matrix<T>
shrink_rowmajor_matrix_sum(node_local<rowmajor_matrix_local<T>>& m,
                           node_local<shrink_vector_info<I>>& vm) {
  rowmajor_matrix<T> ret;
  ret.data = m.map(shrink_rowmajor_matrix_sum_local<T,I>, vm);
  auto rows_cols = ret.data.map(rowmajor_get_local_num_row_col<T>).gather();
  ret.num_col = rows_cols[0].second;
  size_t total = 0;
  for(size_t i = 0; i < rows_cols.size(); i++) total += rows_cols[i].first;
  ret.num_row = total;
  return ret;
}

template <class T, class I>
rowmajor_matrix_local<T>
shrink_rowmajor_matrix_bcast_local(rowmajor_matrix_local<T>& m,
                                   shrink_vector_info<I>& vm) {
  size_t num_col = m.local_num_col;
  rowmajor_matrix_local<T> ret(vm.bcast_size, num_col);
  std::vector<T> work(vm.exchange_map.size() * num_col);
  T* vp = &m.val[0];
  size_t node_size = frovedis::get_nodesize();
  std::vector<T*> current_work_vec(node_size);
  std::vector<I*> current_ex_vec(node_size);
  std::vector<ssize_t> current_size(node_size); // make it signed for fail safe
  auto current_work_vecp = current_work_vec.data();
  auto current_ex_vecp = current_ex_vec.data();
  auto current_sizep = current_size.data();
  auto vmrecv_displp = vm.recv_displ.data();
  auto vmrecv_sizep = vm.recv_size.data();
  for(size_t i = 0; i < node_size; i++) {
    current_work_vecp[i] = &work[0] + vmrecv_displp[i] * num_col;
    current_ex_vecp[i] = &vm.exchange_map[0] + vmrecv_displp[i];
    current_sizep[i] = vmrecv_sizep[i];
  }
  ssize_t max_len = 0;
  for(size_t i = 0; i < node_size; i++)
    if(max_len < vmrecv_sizep[i]) max_len = vmrecv_sizep[i];
  ssize_t num_block = ceil_div<ssize_t>(max_len, SMAT_VLEN);
  for(size_t b = 0; b < num_block; b++) {
    for(size_t i = 0; i < node_size; i++) {
      T* current_workp = current_work_vecp[i];
      I* current_exp = current_ex_vecp[i];
      int work_len =
        current_sizep[i] > SMAT_VLEN ? SMAT_VLEN : current_sizep[i];
#pragma cdir nodep
#pragma _NEC ivdep
      for(int j = 0; j < work_len; j++) {
        for(size_t k = 0; k < num_col; k++) {
          current_workp[num_col * j + k] = vp[num_col * current_exp[j] + k];
        }
      }
      current_work_vecp[i] += work_len * num_col;
      current_ex_vecp[i] += work_len;
      current_sizep[i] -= work_len;
    }
  }
  // note that send/recv is opposite
  large_alltoallv(sizeof(T) * num_col,
                  reinterpret_cast<char*>(const_cast<T*>(&work[0])),
                  vm.recv_size, vm.recv_displ, 
                  reinterpret_cast<char*>(const_cast<T*>(&ret.val[0])),
                  vm.send_size, vm.send_displ, 
                  MPI_COMM_WORLD);
  return ret;
}

template <class T, class I>
node_local<rowmajor_matrix_local<T>>
shrink_rowmajor_matrix_bcast(rowmajor_matrix<T>& m,
                             node_local<shrink_vector_info<I>>& vm) {
  return m.data.map(shrink_rowmajor_matrix_bcast_local<T,I>, vm);
}

template <class T, class I>
rowmajor_matrix_local<T>
shrink_rowmajor_matrix_merge_local(rowmajor_matrix_local<T>& m,
                                   shrink_vector_info<I>& vm) {
  size_t num_col = m.local_num_col;
  rowmajor_matrix_local<T> ret(vm.reduce_size, num_col);
  std::vector<T> work(vm.exchange_map.size() * num_col);
  large_alltoallv(sizeof(T) * num_col,
                  reinterpret_cast<char*>(const_cast<T*>(&m.val[0])),
                  vm.send_size, vm.send_displ, 
                  reinterpret_cast<char*>(const_cast<T*>(&work[0])),
                  vm.recv_size, vm.recv_displ, 
                  MPI_COMM_WORLD);
  T* retp = &ret.val[0];
  size_t node_size = frovedis::get_nodesize();
  for(size_t i = 0; i < node_size; i++) {
    T* current_workp = &work[0] + vm.recv_displ[i] * num_col;
    I* current_exp = &vm.exchange_map[0] + vm.recv_displ[i];
#pragma cdir nodep
#pragma _NEC ivdep
    for(size_t j = 0; j < vm.recv_size[i]; j++) {
      for(size_t k = 0; k < num_col; k++) {
        retp[num_col * current_exp[j] + k] =
          current_workp[num_col * j + k];
      }
    }
  }
  return ret;
}

template <class T, class I>
rowmajor_matrix<T>
shrink_rowmajor_matrix_merge(node_local<rowmajor_matrix_local<T>>& m,
                             node_local<shrink_vector_info<I>>& vm) {
  rowmajor_matrix<T> ret;
  ret.data = m.map(shrink_rowmajor_matrix_merge_local<T,I>, vm);
  auto rows_cols = ret.data.map(rowmajor_get_local_num_row_col<T>).gather();
  ret.num_col = rows_cols[0].second;
  size_t total = 0;
  for(size_t i = 0; i < rows_cols.size(); i++) total += rows_cols[i].first;
  ret.num_row = total;
  return ret;
}

template <class T, class I>
rowmajor_matrix_local<T>
shrink_rowmajor_matrix_local(rowmajor_matrix_local<T>& m,
                             std::vector<I>& tbl) {
  rowmajor_matrix_local<T> ret(tbl.size(), m.local_num_col);
  T* vp = &m.val[0];
  T* retp = &ret.val[0];
  I* tblp = &tbl[0];
  for(size_t i = 0; i < tbl.size(); i++) {
    for(size_t j = 0; j < m.local_num_col; j++) {
      retp[m.local_num_col * i + j] = vp[m.local_num_col * tblp[i] + j];
    }
  }
  return ret;
}

template <class T, class I>
node_local<rowmajor_matrix_local<T>>
shrink_rowmajor_matrix(node_local<rowmajor_matrix_local<T>>& m,
                       node_local<std::vector<I>>& tbl) {
  return m.map(shrink_rowmajor_matrix_local<T,I>, tbl);
}

}

#endif

