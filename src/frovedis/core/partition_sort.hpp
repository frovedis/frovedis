#ifndef PARTITION_SORT_HPP
#define PARTITION_SORT_HPP

#include <vector>
#include "exceptions.hpp"

#define PARTITION_SORT_VLEN 256
#define USE_VREG

namespace frovedis {
template <class T, class I>
void key_val_vector_partition_sort(T* kptr, I* vptr, // for KEY-VALUE pair
                                   size_t size,
                                   size_t k,
                                   time_spent& comp_t,
                                   time_spent& copy_t) {
  checkAssumption(k >= 1  && k <= size);
  size_t st_idx = 0, end_idx = size - 1, kidx = k - 1;
  std::vector<T> work_key(size);  auto work_kptr = work_key.data();
  std::vector<I> work_val(size);  auto work_vptr = work_val.data();
  while(st_idx != end_idx) {
    auto piv_key = kptr[st_idx];
    auto piv_val = vptr[st_idx];
    size_t low = st_idx, high = end_idx;
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

template <class T, class I>
void key_val_vector_partition_sort_loopraked(T* kptr, I* vptr, 
                                             size_t size, 
                                             size_t k,
                                             time_spent& comp_t,
                                             time_spent& copy_t) { 
  checkAssumption(k >= 1  && k <= size);
  auto nchunk = (size - 1) / PARTITION_SORT_VLEN; // maybe zero
  if (nchunk == 0) 
    key_val_vector_partition_sort(kptr, vptr, size, k, comp_t, copy_t);
  else {
    T work_kptr[size]; 
    I work_vptr[size];
    size_t st_idx = 0, end_idx = size - 1, kidx = k - 1;

    size_t st_ptr[PARTITION_SORT_VLEN];
#pragma _NEC vreg(st_ptr) 

    while(st_idx != end_idx) {
      auto piv_key = kptr[st_idx];
      auto piv_val = vptr[st_idx];
      size_t left_idx = st_idx + 1;
      size_t right_idx = left_idx + (nchunk * PARTITION_SORT_VLEN) - 1;
      size_t rem_left_idx = right_idx + 1;
      size_t rem_right_idx = end_idx;
      size_t low = st_idx, high = end_idx;
      RLOG(DEBUG) << st_idx << " "
                  << end_idx << " "
                  << left_idx << " "
                  << right_idx << " "
                  << rem_left_idx << " "
                  << rem_right_idx << "\n";
      if (nchunk > 0) {
        for(size_t i = 0; i < PARTITION_SORT_VLEN; ++i) {
          st_ptr[i] = left_idx + (i * nchunk);
        }
      }

      comp_t.lap_start();
      // for data in chunks
      for(size_t j = 0; j < nchunk; ++j) {
#pragma _NEC ivdep
        for(size_t i = 0; i < PARTITION_SORT_VLEN; ++i) {
          auto loaded_key = kptr[st_ptr[i]];
          auto loaded_val = vptr[st_ptr[i]];
          if (loaded_key < piv_key) {
            work_kptr[low] = loaded_key;
            work_vptr[low] = loaded_val;
            low++;
          }
          else {
            work_kptr[high] = loaded_key;
            work_vptr[high] = loaded_val;
            high--;
          }
        }
      }

    // for remaining part
#pragma _NEC ivdep
      for(size_t i = rem_left_idx; i <= rem_right_idx; ++i) {
        auto loaded_key = kptr[i];
        auto loaded_val = vptr[i];
        if (loaded_key < piv_key) {
          work_kptr[low] = loaded_key;
          work_vptr[low] = loaded_val;
          low++;
        }
        else {
          work_kptr[high] = loaded_key;
          work_vptr[high] = loaded_val;
          high--;
        }
      }
      comp_t.lap_stop();

      // low == high, at this point
      checkAssumption(low == high);
      work_kptr[low] = piv_key;
      work_vptr[low] = piv_val;

      copy_t.lap_start();
      // copy back
      for(size_t i = st_idx; i <= end_idx; ++i) {
        kptr[i] = work_kptr[i];
        vptr[i] = work_vptr[i];
      }
      copy_t.lap_stop();
    
      // adjustment 
      if (low > kidx) end_idx = low - 1;
      else if (low < kidx) st_idx = low + 1;
      else st_idx = end_idx = kidx; // DONE: equal case

      size = end_idx - st_idx + 1;
      nchunk = (size - 1) / PARTITION_SORT_VLEN; // maybe zero
    } // end of while
  } // end of else
}
 
template <class T, class I>
void partition_sort(T* kptr, I* vptr, 
                    size_t size,
                    size_t k,
                    time_spent& comp_t,
                    time_spent& copy_t) {
  key_val_vector_partition_sort_loopraked(kptr, vptr, size, k, comp_t, copy_t);
}

template <class T, class I>
void partition_sort(T* kptr, I* vptr, 
                    size_t size,
                    size_t k) {
  time_spent comp_t(DEBUG), copy_t(DEBUG), part_t(DEBUG);
  part_t.lap_start();
  key_val_vector_partition_sort_loopraked(kptr, vptr, size, k, comp_t, copy_t);
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
                    size_t size, 
                    size_t k,
                    time_spent& comp_t,
                    time_spent& copy_t) {
  partition_sort(key.data(), val.data(), size, k, comp_t, copy_t);
}

template <class T, class I>
void partition_sort(std::vector<T>& key, 
                    std::vector<I>& val, 
                    size_t size, 
                    size_t k) {
  time_spent comp_t(DEBUG), copy_t(DEBUG), part_t(DEBUG);
  part_t.lap_start();
  partition_sort(key, val, size, k, comp_t, copy_t);
  part_t.lap_stop();
  if(get_selfid()== 0) {
    part_t.show_lap("vector partition time: ");
    comp_t.show_lap("  \\_ comparison time: ");
    copy_t.show_lap("  \\_ copy_back time: ");
  }
}

/*
 *  === matrix version ===
 *
 */

template <class T, class I>
void key_val_matrix_partition_sort_by_each_row(
                    T* keyptr, I* valptr, 
                    size_t nrow, size_t ncol,
                    size_t k,
                    time_spent& comp_t,
                    time_spent& copy_t) {
  std::vector<T> work_key(ncol);  auto work_kptr = work_key.data();
  std::vector<I> work_val(ncol);  auto work_vptr = work_val.data();
  size_t kidx = k - 1; // kth index
  for(size_t i = 0; i < nrow; ++i) {
    auto kptr = keyptr + i * ncol;    
    auto vptr = valptr + i * ncol;    
    size_t st_idx = 0, end_idx = ncol - 1;
    while(st_idx != end_idx) {
      auto piv_key = kptr[st_idx];
      auto piv_val = vptr[st_idx];
      size_t low = st_idx, high = end_idx;
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
}

template <class T, class I>
void key_val_matrix_partition_sort_by_each_row_loopraked(
                              T* keyptr, I* valptr, 
                              size_t nrow, size_t ncol,
                              size_t k,
                              time_spent& comp_t,
                              time_spent& copy_t) {
  if (ncol < PARTITION_SORT_VLEN)
    key_val_matrix_partition_sort_by_each_row(keyptr, valptr, nrow, ncol, k, comp_t, copy_t);
  else {
    auto nchunk = (ncol - 1) / PARTITION_SORT_VLEN; // maybe zero
    T work_kptr[ncol]; 
    I work_vptr[ncol];
    size_t st_ptr[PARTITION_SORT_VLEN];
#pragma _NEC vreg(st_ptr) 
    size_t kidx = k - 1; // kth index

    for(size_t i = 0; i < nrow; ++i) {
      auto kptr = keyptr + i * ncol;    
      auto vptr = valptr + i * ncol;    
      size_t st_idx = 0, end_idx = ncol - 1;
      while(st_idx != end_idx) {
        auto piv_key = kptr[st_idx];
        auto piv_val = vptr[st_idx];
        size_t left_idx = st_idx + 1;
        size_t right_idx = left_idx + (nchunk * PARTITION_SORT_VLEN) - 1;
        size_t rem_left_idx = right_idx + 1;
        size_t rem_right_idx = end_idx;
        size_t low = st_idx, high = end_idx;
        RLOG(DEBUG) << st_idx << " "
                    << end_idx << " "
                    << left_idx << " "
                    << right_idx << " "
                    << rem_left_idx << " "
                    << rem_right_idx << "\n";
        if (nchunk > 0) {
          for(size_t i = 0; i < PARTITION_SORT_VLEN; ++i) {
            st_ptr[i] = left_idx + (i * nchunk);
          }
        }

        comp_t.lap_start();
        // for data in chunks
        for(size_t j = 0; j < nchunk; ++j) {
#pragma _NEC ivdep
          for(size_t i = 0; i < PARTITION_SORT_VLEN; ++i) {
            auto loaded_key = kptr[st_ptr[i]];
            auto loaded_val = vptr[st_ptr[i]];
            if (loaded_key < piv_key) {
              work_kptr[low] = loaded_key;
              work_vptr[low] = loaded_val;
              low++;
            }
            else {
              work_kptr[high] = loaded_key;
              work_vptr[high] = loaded_val;
              high--;
            }
          }
        }

      // for remaining part
#pragma _NEC ivdep
        for(size_t i = rem_left_idx; i <= rem_right_idx; ++i) {
          auto loaded_key = kptr[i];
          auto loaded_val = vptr[i];
          if (loaded_key < piv_key) {
            work_kptr[low] = loaded_key;
            work_vptr[low] = loaded_val;
            low++;
          }
          else {
            work_kptr[high] = loaded_key;
            work_vptr[high] = loaded_val;
            high--;
          }
        }
        comp_t.lap_stop();

        // low == high, at this point
        checkAssumption(low == high);
        work_kptr[low] = piv_key;
        work_vptr[low] = piv_val;

        copy_t.lap_start();
        // copy back
        for(size_t i = st_idx; i <= end_idx; ++i) {
          kptr[i] = work_kptr[i];
          vptr[i] = work_vptr[i];
        }
        copy_t.lap_stop();
    
        // adjustment 
        if (low > kidx) end_idx = low - 1;
        else if (low < kidx) st_idx = low + 1;
        else st_idx = end_idx = kidx; // DONE: equal case

        auto size = end_idx - st_idx + 1;
        nchunk = (size - 1) / PARTITION_SORT_VLEN; // maybe zero
      } // end of while
    }
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
    // current implementation of loopraked version has performance issue...
    //key_val_matrix_partition_sort_by_each_row_loopraked(
    key_val_matrix_partition_sort_by_each_row(
                              kptr, vptr, nrow, ncol, k, comp_t, copy_t);
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
