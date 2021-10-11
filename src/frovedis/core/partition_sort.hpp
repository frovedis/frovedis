#ifndef PARTITION_SORT_HPP
#define PARTITION_SORT_HPP

#include <vector>
#include "exceptions.hpp"

#define PARTITION_SORT_VLEN 256 
#define PARTITION_SORT_MIN_EACH_LEN 32 // minimum VLEN of 2nd part

#define USE_LOOP_RAKING
//#define PARTITION_SORT_DEBUG

namespace frovedis {

template <class T, class I>
void partition_sort_normal(T* kptr, I* vptr, 
                           size_t size,
                           size_t k,
                           time_spent& comp_t,
                           time_spent& copy_t) {
  require(k >= 1  && k <= size, 
  "the given k must be in between 1 to " + std::to_string(size) + "!\n");
  T work_kptr[size]; 
  I work_vptr[size];
  size_t kidx = k - 1;
#include "partition_sort_normal.incl" 
}

template <class T, class I>
void partition_sort(T* kptr, I* vptr, 
                    size_t size,
                    size_t k,
                    time_spent& comp_t,
                    time_spent& copy_t) {
#ifdef USE_LOOP_RAKING
  require(k >= 1  && k <= size, 
  "the given k must be in between 1 to " + std::to_string(size) + "!\n");
  T work_kptr[size]; 
  I work_vptr[size];
  size_t kidx = k - 1;
  size_t stp[PARTITION_SORT_VLEN];
  size_t endp[PARTITION_SORT_VLEN];
  size_t lowp[PARTITION_SORT_VLEN];
  size_t highp[PARTITION_SORT_VLEN];
#pragma _NEC vreg(stp) 
#pragma _NEC vreg(endp) 
#pragma _NEC vreg(lowp) 
#pragma _NEC vreg(highp) 
#include "partition_sort_raked.incl"
#else
  partition_sort_normal(kptr, vptr, size, k, comp_t, copy_t);
#endif
}

template <class T, class I>
void partition_sort(T* kptr, I* vptr, 
                    size_t size,
                    size_t k) {
  time_spent comp_t(DEBUG), copy_t(DEBUG), part_t(DEBUG);
  part_t.lap_start();
  partition_sort(kptr, vptr, size, k, comp_t, copy_t);
  part_t.lap_stop();
  if(get_selfid()== 0) {
    part_t.show_lap("vector partition time: ");
    comp_t.show_lap("  \\_ comparison time: ");
    copy_t.show_lap("  \\_ copy_back time: ");
  }
}

template <class T, class I>
void partition_sort(std::vector<T>& key, 
                    std::vector<I>& val, 
                    size_t k,
                    time_spent& comp_t,
                    time_spent& copy_t) {
  checkAssumption(key.size() == val.size());
  partition_sort(key.data(), val.data(), key.size(), k, comp_t, copy_t);
}

template <class T, class I>
void partition_sort(std::vector<T>& key, 
                    std::vector<I>& val, 
                    size_t k) {
  checkAssumption(key.size() == val.size());
  partition_sort(key.data(), val.data(), key.size(), k);
}

/*
 *  === matrix version ===
 *
 */

template <class T, class I>
void partition_sort_by_each_row(
       T* keyptr, I* valptr, 
       size_t nrow, size_t ncol,
       size_t k,
       time_spent& comp_t,
       time_spent& copy_t) {
  auto size = ncol;
  require(k >= 1  && k <= size, 
  "the given k must be in between 1 to " + std::to_string(size) + "!\n");
  T work_kptr[size]; 
  I work_vptr[size];
  size_t kidx = k - 1;

#ifdef USE_LOOP_RAKING
  size_t stp[PARTITION_SORT_VLEN];
  size_t endp[PARTITION_SORT_VLEN];
  size_t lowp[PARTITION_SORT_VLEN];
  size_t highp[PARTITION_SORT_VLEN];
#pragma _NEC vreg(stp) 
#pragma _NEC vreg(endp) 
#pragma _NEC vreg(lowp) 
#pragma _NEC vreg(highp) 
#endif

  for(size_t i = 0; i < nrow; ++i) {
    auto kptr = keyptr + i * ncol;    
    auto vptr = valptr + i * ncol;    
#ifdef USE_LOOP_RAKING
#include "partition_sort_raked.incl"
#else
#include "partition_sort_normal.incl"
#endif
  }
}

// for matrix with different no. of elements in each row
template <class T, class I, class O>
void k_partition(T* keyptr, I* valptr,
                 O* offsetptr,
                 size_t nrow, size_t k) {
  O max_ncol = 0, min_ncol = std::numeric_limits<O>::max();
  for(size_t i = 0; i < nrow; ++i) {
    auto cur_ncol = offsetptr[i + 1] - offsetptr[i];
    if (cur_ncol > max_ncol) max_ncol = cur_ncol;
    else if (cur_ncol < min_ncol) min_ncol = cur_ncol;
  }
  require(min_ncol >= k,
  "given k is larger than the minimum no. of cols in given sparse matrix!\n");

  std::vector<T> work_key(max_ncol);  auto work_kptr = work_key.data();
  std::vector<I> work_val(max_ncol);  auto work_vptr = work_val.data();
  size_t kidx = k - 1; // kth index
  time_spent tot_t(DEBUG), comp_t(DEBUG), copy_t(DEBUG);

  tot_t.lap_start();
  for(size_t i = 0; i < nrow; ++i) {
    auto kptr = keyptr + offsetptr[i];
    auto vptr = valptr + offsetptr[i];
    auto ncol = offsetptr[i + 1] - offsetptr[i];
    O st_idx = 0, end_idx = ncol - 1;
    while(st_idx != end_idx) {
      auto piv_key = kptr[st_idx];
      auto piv_val = vptr[st_idx];
      O low = st_idx, high = end_idx;
      comp_t.lap_start();
#pragma _NEC ivdep
      for(size_t i = st_idx + 1; i <= end_idx; ++i) {
        if(kptr[i] < piv_key) {
          work_kptr[low] = kptr[i];
          work_vptr[low] = vptr[i];
          low++;
        }
        else {
          work_kptr[high] = kptr[i];
          work_vptr[high] = vptr[i];
          high--;
        }
      }
      comp_t.lap_stop();

      // at this point (low == high)
      checkAssumption(low == high);
      work_kptr[low] = piv_key;
      work_vptr[low] = piv_val;

      copy_t.lap_start();
      // copy-back to input data buffers for next iteration
      for(size_t i = st_idx; i <= end_idx; ++i) {
        kptr[i] = work_kptr[i];
        vptr[i] = work_vptr[i];
      }
      copy_t.lap_stop();

      // adjustment
      if (low > kidx) end_idx = low - 1;
      else if (low < kidx) st_idx = low + 1;
      else st_idx = end_idx = kidx; // DONE: equal case
    }
  }
  tot_t.lap_stop();
  if(get_selfid()== 0) {
    tot_t.show_lap("total k-partition time: ");
    comp_t.show_lap("  \\_ comparison time: ");
    copy_t.show_lap("  \\_ copy_back time: ");
  }
}

template <class T, class I>
void key_val_matrix_partition_sort_by_multiple_row(
                                        T* kptr, I* vptr,
                                        size_t nrow, size_t ncol,
                                        size_t k,
                                        time_spent& comp_t,
                                        time_spent& copy_t) {
  std::vector<size_t> left(nrow), right(nrow); 
  std::vector<size_t> low(nrow), high(nrow);
  std::vector<T> pivot_key(nrow);
  std::vector<I> pivot_val(nrow);
  auto left_ptr = left.data();
  auto right_ptr = right.data();
  auto low_ptr = low.data();
  auto high_ptr = high.data();
  auto piv_kptr = pivot_key.data();
  auto piv_vptr = pivot_val.data();

  std::vector<T> work_key(nrow * ncol);
  std::vector<I> work_val(nrow * ncol);
  auto wptr_key = work_key.data();
  auto wptr_val = work_val.data();
  auto kidx = k - 1; // kth index

  time_spent init_t(TRACE);
  time_spent pivot_t(TRACE);
  time_spent adjust_t(TRACE);
  time_spent break_t(TRACE);
  time_spent max_t(TRACE);

  init_t.lap_start();
  // initialization loop
  for(size_t i = 0; i < nrow; ++i) {
    low_ptr[i] = left_ptr[i] = 0;
    high_ptr[i] = right_ptr[i] = ncol - 1;
    piv_kptr[i] = kptr[i * ncol + left_ptr[i]]; // extreme left of cur_row is pivot
    piv_vptr[i] = vptr[i * ncol + left_ptr[i]]; // extreme left of cur_row is pivot
  }
  init_t.lap_stop();
  size_t max_len = ncol; // initial max_len for comparision loop
  size_t start_row = 0;
      
  while (true) {
    // comparison loop
    comp_t.lap_start();
    for(size_t j = 0; j < max_len; ++j) {
#pragma _NEC ivdep
      for(size_t i = start_row; i < nrow; ++i) {
        auto cur_col_idx = left_ptr[i] + 1 + j;
        if (cur_col_idx <= right_ptr[i]) {
          auto cur_idx = i * ncol + cur_col_idx;
          auto loaded_key = kptr[cur_idx];
          auto loaded_val = vptr[cur_idx];
          if (loaded_key < piv_kptr[i]) {
            auto wloc = i * ncol + low_ptr[i];
            wptr_key[wloc] = loaded_key;
            wptr_val[wloc] = loaded_val;
            low_ptr[i]++;
          }
          else {
            auto wloc = i * ncol + high_ptr[i];
            wptr_key[wloc] = loaded_key;
            wptr_val[wloc] = loaded_val;
            high_ptr[i]--;
          }
        }
      }
    }
    comp_t.lap_stop();
    // pivot assignment loop
    pivot_t.lap_start();
    for(size_t i = start_row; i < nrow; ++i) {
      // low_ptr[i] and high_ptr[i] both should be same at this point
      // assigning pivot at its own place in work matrix
      auto actual_piv_idx = i * ncol + low_ptr[i];
      wptr_key[actual_piv_idx] = piv_kptr[i];
      wptr_val[actual_piv_idx] = piv_vptr[i];  
    }
    pivot_t.lap_stop();
    // copy back loop
    copy_t.lap_start();
    for(size_t i = start_row; i < nrow; ++i) {
      for(size_t j = left_ptr[i]; j <= right_ptr[i]; ++j) {
        auto idx = i * ncol + j;
        kptr[idx] = wptr_key[idx];
        vptr[idx] = wptr_val[idx];
      }
    }
    copy_t.lap_stop();
    // adjustment loop
    adjust_t.lap_start();
    for(size_t i = start_row; i < nrow; ++i) {
      if (low_ptr[i] < kidx) left_ptr[i] = low_ptr[i] + 1; // no change in right_ptr[i]
      else if (low_ptr[i] > kidx) right_ptr[i] = low_ptr[i] - 1; // no change in left_ptr[i]
      else left_ptr[i] = right_ptr[i] = kidx; // DONE: low_ptr[i] == kidx 
    }
    for(size_t i = start_row; i < nrow; ++i) {
      low_ptr[i] = left_ptr[i];
      high_ptr[i] = right_ptr[i];
      piv_kptr[i] = kptr[i * ncol + left_ptr[i]]; // extreme left of cur_row is pivot
      piv_vptr[i] = vptr[i * ncol + left_ptr[i]]; // extreme left of cur_row is pivot
    }
    adjust_t.lap_stop();

    // whether to break from while loop ?
    break_t.lap_start();
    size_t rid;
    for(rid = start_row; rid < nrow; ++rid) {
      if (left_ptr[rid] != right_ptr[rid]) break;
    }
    if(rid == nrow) break; // all done
    else start_row = rid;
    break_t.lap_stop();

    // deciding max loop length for comparision loop
    max_t.lap_start();
    max_len = right_ptr[start_row] - left_ptr[start_row] + 1;
    for(size_t i = start_row + 1; i < nrow; ++i) {
      auto cur_len = right_ptr[i] - left_ptr[i] + 1;
      if (cur_len > max_len) max_len = cur_len;
    }
    max_t.lap_stop();
  }
  if (get_selfid() == 0){
    init_t.show_lap("initialization short loop: ");
    pivot_t.show_lap("pivot short loop: ");
    adjust_t.show_lap("adjust short loop: ");
    break_t.show_lap("break short loop: ");
    max_t.show_lap("max short loop: ");
  }
}

// --- TODO: fix -> this version is not stable ---
template <class T, class I>
void key_val_matrix_partition_sort_by_multiple_row_loopraked(
                                       T* kptr, I* vptr, 
                                       size_t nrow, size_t ncol,
                                       size_t k,
                                       time_spent& comp_t,
                                       time_spent& copy_t) {
  if (nrow < PARTITION_SORT_VLEN)
    key_val_matrix_partition_sort_by_multiple_row(kptr, vptr, nrow, ncol, k, comp_t, copy_t);
  else { 
#ifdef USE_VREG
    size_t left_ptr[PARTITION_SORT_VLEN], right_ptr[PARTITION_SORT_VLEN];
    size_t low_ptr[PARTITION_SORT_VLEN], high_ptr[PARTITION_SORT_VLEN];
    T piv_kptr[PARTITION_SORT_VLEN];
    I piv_vptr[PARTITION_SORT_VLEN];
#pragma _NEC vreg(left_ptr)
#pragma _NEC vreg(right_ptr)
#pragma _NEC vreg(low_ptr)
#pragma _NEC vreg(high_ptr)
#pragma _NEC vreg(piv_kptr)
#pragma _NEC vreg(piv_vptr)

#else
    std::vector<size_t> left(PARTITION_SORT_VLEN), right(PARTITION_SORT_VLEN); 
    std::vector<size_t> low(PARTITION_SORT_VLEN), high(PARTITION_SORT_VLEN);
    std::vector<T> pivot_key(PARTITION_SORT_VLEN);
    std::vector<I> pivot_val(PARTITION_SORT_VLEN);
    auto left_ptr = left.data();
    auto right_ptr = right.data();
    auto low_ptr = low.data();
    auto high_ptr = high.data();
    auto piv_kptr = pivot_key.data();
    auto piv_vptr = pivot_val.data();
#endif

    std::vector<T> work_key(PARTITION_SORT_VLEN * ncol);
    std::vector<I> work_val(PARTITION_SORT_VLEN * ncol);
    auto wptr_key = work_key.data();
    auto wptr_val = work_val.data();
    auto kidx = k - 1; // kth index

    time_spent init_t(TRACE);
    time_spent pivot_t(TRACE);
    time_spent adjust_t(TRACE);
    time_spent break_t(TRACE);
    time_spent max_t(TRACE);

    init_t.lap_start();
    // initialization loop
    for(size_t i = 0; i < PARTITION_SORT_VLEN; ++i) {
      low_ptr[i] = left_ptr[i] = 0;
      high_ptr[i] = right_ptr[i] = ncol - 1;
      piv_kptr[i] = kptr[i * ncol + left_ptr[i]]; // extreme left of cur_row is pivot
      piv_vptr[i] = vptr[i * ncol + left_ptr[i]]; // extreme left of cur_row is pivot
    }
    init_t.lap_stop();
    size_t max_len = ncol; // initial max_len for comparision loop
    size_t start_row = 0;
      
    while (true) {
      // comparison loop
      comp_t.lap_start();
      for(size_t j = 0; j < max_len; ++j) {
#pragma _NEC ivdep
        for(size_t i = start_row; i < PARTITION_SORT_VLEN; ++i) {
          auto cur_col_idx = left_ptr[i] + 1 + j;
          if (cur_col_idx <= right_ptr[i]) {
            auto cur_idx = i * ncol + cur_col_idx;
            auto loaded_key = kptr[cur_idx];
            auto loaded_val = vptr[cur_idx];
            if (loaded_key < piv_kptr[i]) {
              auto wloc = i * ncol + low_ptr[i];
              wptr_key[wloc] = loaded_key;
              wptr_val[wloc] = loaded_val;
              low_ptr[i]++;
            }
            else {
              auto wloc = i * ncol + high_ptr[i];
              wptr_key[wloc] = loaded_key;
              wptr_val[wloc] = loaded_val;
              high_ptr[i]--;
            }
          }
        }
      }
      comp_t.lap_stop();
      // pivot assignment loop
      pivot_t.lap_start();
      for(size_t i = start_row; i < PARTITION_SORT_VLEN; ++i) {
        // low_ptr[i] and high_ptr[i] both should be same at this point
        // assigning pivot at its own place in work matrix
        auto actual_piv_idx = i * ncol + low_ptr[i];
        wptr_key[actual_piv_idx] = piv_kptr[i];
        wptr_val[actual_piv_idx] = piv_vptr[i];  
      }
      pivot_t.lap_stop();
      // copy back loop
      copy_t.lap_start();
      for(size_t i = start_row; i < PARTITION_SORT_VLEN; ++i) {
        for(size_t j = left_ptr[i]; j <= right_ptr[i]; ++j) {
          auto idx = i * ncol + j;
          kptr[idx] = wptr_key[idx];
          vptr[idx] = wptr_val[idx];
        }
      }
      copy_t.lap_stop();
      // adjustment loop
      adjust_t.lap_start();
      for(size_t i = start_row; i < PARTITION_SORT_VLEN; ++i) {
        if (low_ptr[i] < kidx) left_ptr[i] = low_ptr[i] + 1; // no change in right_ptr[i]
        else if (low_ptr[i] > kidx) right_ptr[i] = low_ptr[i] - 1; // no change in left_ptr[i]
        else left_ptr[i] = right_ptr[i] = kidx; // DONE: low_ptr[i] == kidx 
      }
      for(size_t i = start_row; i < PARTITION_SORT_VLEN; ++i) {
        low_ptr[i] = left_ptr[i];
        high_ptr[i] = right_ptr[i];
        piv_kptr[i] = kptr[i * ncol + left_ptr[i]]; // extreme left of cur_row is pivot
        piv_vptr[i] = vptr[i * ncol + left_ptr[i]]; // extreme left of cur_row is pivot
      }
      adjust_t.lap_stop();

      // whether to break from while loop ?
      break_t.lap_start();
      size_t rid;
      for(rid = start_row; rid < PARTITION_SORT_VLEN; ++rid) {
        if (left_ptr[rid] != right_ptr[rid]) break;
      }
      if(rid == PARTITION_SORT_VLEN) break; // all done
      else start_row = rid;
      break_t.lap_stop();

      // deciding max loop length for comparision loop
      max_t.lap_start();
      max_len = right_ptr[start_row] - left_ptr[start_row] + 1;
      for(size_t i = start_row + 1; i < PARTITION_SORT_VLEN; ++i) {
        auto cur_len = right_ptr[i] - left_ptr[i] + 1;
        if (cur_len > max_len) max_len = cur_len;
      }
      max_t.lap_stop();
    }
    if (get_selfid() == 0){
      init_t.show_lap("initialization loop: ");
      pivot_t.show_lap("pivot loop: ");
      adjust_t.show_lap("adjust loop: ");
      break_t.show_lap("break loop: ");
      max_t.show_lap("max loop: ");
    }
  }
}

template <class T, class I>
void partition_sort(T* kptr, I* vptr, 
                    size_t nrow, size_t ncol,
                    size_t k,
                    time_spent& comp_t,
                    time_spent& copy_t,
                    bool allow_multiple = false) {
  if (allow_multiple) {
    auto niter = ceil_div(nrow, (size_t) PARTITION_SORT_VLEN); 
    std::vector<size_t> rows(niter + 1); auto rows_ptr = rows.data();
    for(size_t i = 0; i <= niter; ++i) rows_ptr[i]  = i * PARTITION_SORT_VLEN;
    if (rows_ptr[niter] > nrow) rows_ptr[niter] = nrow;
    for (size_t i = 0; i < niter; ++i) {
      RLOG(DEBUG) << "(" << i + 1 << ") partitioning [" << rows_ptr[i] 
                  << " : " << rows_ptr[i+1] << "]\n";
      auto kptr_ = kptr + rows_ptr[i] * ncol;
      auto vptr_ = vptr + rows_ptr[i] * ncol;
      auto nrow_ = rows_ptr[i+1] - rows_ptr[i];
      // current implementation of loopraked version has performance issue...
      //key_val_matrix_partition_sort_by_multiple_row_loopraked(
      key_val_matrix_partition_sort_by_multiple_row(
                                             kptr_, vptr_, nrow_,
                                             ncol, k, comp_t, copy_t);
    }
  }
  else {
    partition_sort_by_each_row(kptr, vptr, nrow, ncol, k, comp_t, copy_t);
  }
}

template <class T, class I>
void partition_sort(T* kptr, I* vptr, 
                    size_t nrow, size_t ncol,
                    size_t k,
                    bool allow_multiple = false) {
  time_spent comp_t(DEBUG), copy_t(DEBUG), part_t(DEBUG);
  part_t.lap_start();
  partition_sort(kptr, vptr, nrow, ncol, k, comp_t, copy_t, allow_multiple);
  part_t.lap_stop();
  if(get_selfid()== 0) {
    part_t.show_lap("matrix partition time: ");
    comp_t.show_lap("  \\_ comparison time: ");
    copy_t.show_lap("  \\_ copy_back time: ");
  }
}

}
#endif
