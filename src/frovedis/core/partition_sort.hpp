#ifndef PARTITION_HPP
#define PARTITION_HPP

#include <vector>
#include <random>
#include "exceptions.hpp"

//#define PARTITION_VLEN 1024     // unrolling=4
#define PARTITION_VLEN 512        // unrolling=2 (seems best as per evaluation)
#define PARTITION_VLEN_EACH 256 
#define PARTITION_MIN_EACH_LEN 32 // minimum VLEN of 2nd part
#define SMALL_SEARCH_LEN 30

#define USE_LOOP_RAKING
#define USE_RANDOM_PIVOT
//#define PARTITION_DEBUG

namespace frovedis {

template <class T, class I>
void find_topk_min(T* kptr, I* vptr, size_t size, size_t k) {
  for (size_t i =  0; i < k; ++i) {
    auto min = i;
    for(size_t j = i + 1; j < size; ++j) if (kptr[j] < kptr[min]) min = j;
    if (i != min) {
      auto tmpk = kptr[i]; kptr[i] = kptr[min]; kptr[min] = tmpk;
      auto tmpv = vptr[i]; vptr[i] = vptr[min]; vptr[min] = tmpv;
    }
  }
}

template <class T>
void find_topk_min(T* kptr, size_t size, size_t k) {
  for (size_t i =  0; i < k; ++i) {
    auto min = i;
    for(size_t j = i + 1; j < size; ++j) if (kptr[j] < kptr[min]) min = j;
    if (i != min) {
      auto tmpk = kptr[i]; kptr[i] = kptr[min]; kptr[min] = tmpk;
    }
  }
}

template <class T, class I>
void partition_normal(T* kptr, I* vptr, 
                      size_t size,
                      size_t k,
                      time_spent& comp_t,
                      time_spent& copy_t) {
  require(k >= 1  && k <= size, 
  "the given k must be in between 1 to " + std::to_string(size) + "!\n");
  if (k <= SMALL_SEARCH_LEN) {
    find_topk_min(kptr, vptr, size, k);
  } else {
    T work_kptr[size]; 
    I work_vptr[size];
    size_t kidx = k - 1;
    srand(0); // for random selection of pivot
#include "partition_keyval_pair_normal.incl" 
  }
}

template <class T>
void partition_normal(T* kptr,
                      size_t size,
                      size_t k,
                      time_spent& comp_t,
                      time_spent& copy_t) {
  require(k >= 1  && k <= size,
  "the given k must be in between 1 to " + std::to_string(size) + "!\n");
  if (k <= SMALL_SEARCH_LEN) {
    find_topk_min(kptr, size, k);
  } else {
    T work_kptr[size];
    size_t kidx = k - 1;
    srand(0); // for random selection of pivot
#include "partition_key_normal.incl"
  }
}

template <class T, class I>
void partition(T* kptr, I* vptr, 
               size_t size,
               size_t k,
               time_spent& comp_t,
               time_spent& copy_t) {
  require(k >= 1  && k <= size, 
  "the given k must be in between 1 to " + std::to_string(size) + "!\n");
  if (k <= SMALL_SEARCH_LEN) {
    find_topk_min(kptr, vptr, size, k);
  } else {
#ifdef USE_LOOP_RAKING
    T work_kptr[size]; 
    I work_vptr[size];
    size_t kidx = k - 1;
    srand(0); // for random selection of pivot
    size_t lowp0[PARTITION_VLEN_EACH];
    size_t lowp1[PARTITION_VLEN_EACH];
    //size_t lowp2[PARTITION_VLEN_EACH];
    //size_t lowp3[PARTITION_VLEN_EACH];
    size_t highp0[PARTITION_VLEN_EACH];
    size_t highp1[PARTITION_VLEN_EACH];
    //size_t highp2[PARTITION_VLEN_EACH];
    //size_t highp3[PARTITION_VLEN_EACH];
#pragma _NEC vreg(lowp0) 
#pragma _NEC vreg(lowp1) 
//#pragma _NEC vreg(lowp2) 
//#pragma _NEC vreg(lowp3) 
#pragma _NEC vreg(highp0) 
#pragma _NEC vreg(highp1) 
//#pragma _NEC vreg(highp2) 
//#pragma _NEC vreg(highp3) 
#include "partition_keyval_pair_raked.incl"
#else
    partition_normal(kptr, vptr, size, k, comp_t, copy_t);
#endif
  }
}

template <class T>
void partition(T* kptr,
               size_t size,
               size_t k,
               time_spent& comp_t,
               time_spent& copy_t) {
  require(k >= 1  && k <= size,
  "the given k must be in between 1 to " + std::to_string(size) + "!\n");
  if (k <= SMALL_SEARCH_LEN) {
    find_topk_min(kptr, size, k);
  } else {
#ifdef USE_LOOP_RAKING
    T work_kptr[size];
    size_t kidx = k - 1;
    srand(0); // for random selection of pivot
    size_t lowp0[PARTITION_VLEN_EACH];
    size_t lowp1[PARTITION_VLEN_EACH];
    //size_t lowp2[PARTITION_VLEN_EACH];
    //size_t lowp3[PARTITION_VLEN_EACH];
    size_t highp0[PARTITION_VLEN_EACH];
    size_t highp1[PARTITION_VLEN_EACH];
    //size_t highp2[PARTITION_VLEN_EACH];
    //size_t highp3[PARTITION_VLEN_EACH];
#pragma _NEC vreg(lowp0)
#pragma _NEC vreg(lowp1)
//#pragma _NEC vreg(lowp2)
//#pragma _NEC vreg(lowp3)
#pragma _NEC vreg(highp0)
#pragma _NEC vreg(highp1)
//#pragma _NEC vreg(highp2)
//#pragma _NEC vreg(highp3)
#include "partition_key_raked.incl"
#else
    partition_normal(kptr, size, k, comp_t, copy_t);
#endif
  }
}

template <class T, class I>
void partition(std::vector<T>& key, 
               std::vector<I>& val, 
               size_t k,
               time_spent& comp_t,
               time_spent& copy_t) {
  checkAssumption(key.size() == val.size());
  partition(key.data(), val.data(), key.size(), k, comp_t, copy_t);
}

template <class T, class I>
void partition(T* kptr, I* vptr, 
               size_t size,
               size_t k) {
  time_spent comp_t(DEBUG), copy_t(DEBUG), part_t(DEBUG);
  part_t.lap_start();
  partition(kptr, vptr, size, k, comp_t, copy_t);
  part_t.lap_stop();
  if(get_selfid()== 0) {
    part_t.show_lap("vector partition time: ");
    comp_t.show_lap("  \\_ comparison time: ");
    copy_t.show_lap("  \\_ copy_back time: ");
  }
}

template <class T, class I>
void partition(std::vector<T>& key, 
               std::vector<I>& val, 
               size_t k) {
  checkAssumption(key.size() == val.size());
  partition(key.data(), val.data(), key.size(), k);
}

template <class T>
void partition(std::vector<T>& key, size_t k,
               time_spent& comp_t,
               time_spent& copy_t) {
  partition(key.data(), key.size(), k, comp_t, copy_t);
}

template <class T>
void partition(T* kptr, size_t size, size_t k) {
  time_spent comp_t(DEBUG), copy_t(DEBUG), part_t(DEBUG);
  part_t.lap_start();
  partition(kptr, size, k, comp_t, copy_t);
  part_t.lap_stop();
  if(get_selfid()== 0) {
    part_t.show_lap("vector partition time: ");
    comp_t.show_lap("  \\_ comparison time: ");
    copy_t.show_lap("  \\_ copy_back time: ");
  }
}

template <class T>
void partition(std::vector<T>& key, size_t k) {
  partition(key.data(), key.size(), k);
}

/*
 *  === matrix version ===
 *
 */

template <class T, class I>
void partition_by_each_row(T* keyptr, I* valptr, 
                           size_t nrow, size_t ncol, size_t k,
                           time_spent& comp_t,
                           time_spent& copy_t) {
  auto size = ncol;
  require(k >= 1  && k <= size, 
  "the given k must be in between 1 to " + std::to_string(size) + "!\n");
  if (k <= SMALL_SEARCH_LEN) {
    for(size_t i = 0; i < nrow; ++i) {
      auto kptr = keyptr + i * ncol;    
      auto vptr = valptr + i * ncol;    
      find_topk_min(kptr, vptr, size, k);
    }
  } else {
    T work_kptr[size]; 
    I work_vptr[size];
    size_t kidx = k - 1;
    srand(0); // for random selection of pivot
#ifdef USE_LOOP_RAKING
    size_t lowp0[PARTITION_VLEN_EACH];
    size_t lowp1[PARTITION_VLEN_EACH];
//  size_t lowp2[PARTITION_VLEN_EACH];
//  size_t lowp3[PARTITION_VLEN_EACH];
    size_t highp0[PARTITION_VLEN_EACH];
    size_t highp1[PARTITION_VLEN_EACH];
//  size_t highp2[PARTITION_VLEN_EACH];
//  size_t highp3[PARTITION_VLEN_EACH];
#pragma _NEC vreg(lowp0)
#pragma _NEC vreg(lowp1)
//#pragma _NEC vreg(lowp2)
//#pragma _NEC vreg(lowp3)
#pragma _NEC vreg(highp0)
#pragma _NEC vreg(highp1)
//#pragma _NEC vreg(highp2)
//#pragma _NEC vreg(highp3)
#endif

    for(size_t i = 0; i < nrow; ++i) {
      auto kptr = keyptr + i * ncol;    
      auto vptr = valptr + i * ncol;    
#ifdef USE_LOOP_RAKING
#include "partition_keyval_pair_raked.incl"
#else
#include "partition_keyval_pair_normal.incl"
#endif
    }
  }
}

template <class T>
void partition_by_each_row(T* keyptr, size_t nrow, size_t ncol, size_t k,
                           time_spent& comp_t,
                           time_spent& copy_t) {
  auto size = ncol;
  require(k >= 1  && k <= size,
  "the given k must be in between 1 to " + std::to_string(size) + "!\n");
  if (k <= SMALL_SEARCH_LEN) {
    for(size_t i = 0; i < nrow; ++i) {
      auto kptr = keyptr + i * ncol;
      find_topk_min(kptr, size, k);
    }
  } else {
    T work_kptr[size];
    size_t kidx = k - 1;
    srand(0); // for random selection of pivot
#ifdef USE_LOOP_RAKING
    size_t lowp0[PARTITION_VLEN_EACH];
    size_t lowp1[PARTITION_VLEN_EACH];
//  size_t lowp2[PARTITION_VLEN_EACH];
//  size_t lowp3[PARTITION_VLEN_EACH];
    size_t highp0[PARTITION_VLEN_EACH];
    size_t highp1[PARTITION_VLEN_EACH];
//  size_t highp2[PARTITION_VLEN_EACH];
//  size_t highp3[PARTITION_VLEN_EACH];
#pragma _NEC vreg(lowp0)
#pragma _NEC vreg(lowp1)
//#pragma _NEC vreg(lowp2)
//#pragma _NEC vreg(lowp3)
#pragma _NEC vreg(highp0)
#pragma _NEC vreg(highp1)
//#pragma _NEC vreg(highp2)
//#pragma _NEC vreg(highp3)
#endif

    for(size_t i = 0; i < nrow; ++i) {
      auto kptr = keyptr + i * ncol;
#ifdef USE_LOOP_RAKING
#include "partition_key_raked.incl"
#else
#include "partition_key_normal.incl"
#endif
    }
  }
}

// for matrix with different no. of elements in each row
template <class T, class I, class O>
void partition_by_each_row_segment(
     T* keyptr, I* valptr, O* offsetptr,
     size_t nrow, size_t k) {
  O max_ncol = 0, min_ncol = std::numeric_limits<O>::max();
  for(size_t i = 0; i < nrow; ++i) {
    auto cur_ncol = offsetptr[i + 1] - offsetptr[i];
    if (cur_ncol > max_ncol) max_ncol = cur_ncol;
    else if (cur_ncol < min_ncol) min_ncol = cur_ncol;
  }
  require(min_ncol >= k,
  "given k is larger than the minimum no. of cols in given segmented matrix!\n");

  if (k <= SMALL_SEARCH_LEN) {
    for(size_t i = 0; i < nrow; ++i) {
      auto kptr = keyptr + offsetptr[i];
      auto vptr = valptr + offsetptr[i];
      auto size = offsetptr[i + 1] - offsetptr[i];
      find_topk_min(kptr, vptr, size, k);
    }
  } else {
    T work_kptr[max_ncol]; 
    I work_vptr[max_ncol];
    size_t kidx = k - 1;
    srand(0); // for random selection of pivot
#ifdef USE_LOOP_RAKING
    size_t lowp0[PARTITION_VLEN_EACH];
    size_t lowp1[PARTITION_VLEN_EACH];
//  size_t lowp2[PARTITION_VLEN_EACH];
//  size_t lowp3[PARTITION_VLEN_EACH];
    size_t highp0[PARTITION_VLEN_EACH];
    size_t highp1[PARTITION_VLEN_EACH];
//  size_t highp2[PARTITION_VLEN_EACH];
//  size_t highp3[PARTITION_VLEN_EACH];
#pragma _NEC vreg(lowp0)
#pragma _NEC vreg(lowp1)
//#pragma _NEC vreg(lowp2)
//#pragma _NEC vreg(lowp3)
#pragma _NEC vreg(highp0)
#pragma _NEC vreg(highp1)
//#pragma _NEC vreg(highp2)
//#pragma _NEC vreg(highp3)
#endif

    time_spent tot_t(DEBUG), comp_t(DEBUG), copy_t(DEBUG);
    tot_t.lap_start();
    for(size_t i = 0; i < nrow; ++i) {
      auto kptr = keyptr + offsetptr[i];
      auto vptr = valptr + offsetptr[i];
      auto size = offsetptr[i + 1] - offsetptr[i];
#ifdef USE_LOOP_RAKING
#include "partition_keyval_pair_raked.incl"
#else
#include "partition_keyval_pair_normal.incl"
#endif
    }
    tot_t.lap_stop();
    if(get_selfid() == 0) {
      tot_t.show_lap("partition time: ");
      comp_t.show_lap("  \\_ comparison time: ");
      copy_t.show_lap("  \\_ copy_back time: ");
    }
  }
}

template <class T, class O>
void partition_by_each_row_segment(
     T* keyptr, O* offsetptr, 
     size_t nrow, size_t k) {
  O max_ncol = 0, min_ncol = std::numeric_limits<O>::max();
  for(size_t i = 0; i < nrow; ++i) {
    auto cur_ncol = offsetptr[i + 1] - offsetptr[i];
    if (cur_ncol > max_ncol) max_ncol = cur_ncol;
    else if (cur_ncol < min_ncol) min_ncol = cur_ncol;
  }
  require(min_ncol >= k,
  "given k is larger than the minimum no. of cols in given segmented matrix!\n");

  if (k <= SMALL_SEARCH_LEN) {
    for(size_t i = 0; i < nrow; ++i) {
      auto kptr = keyptr + offsetptr[i];
      auto size = offsetptr[i + 1] - offsetptr[i];
      find_topk_min(kptr, size, k);
    }
  } else {
    T work_kptr[max_ncol];
    size_t kidx = k - 1;
    srand(0); // for random selection of pivot
#ifdef USE_LOOP_RAKING
    size_t lowp0[PARTITION_VLEN_EACH];
    size_t lowp1[PARTITION_VLEN_EACH];
//  size_t lowp2[PARTITION_VLEN_EACH];
//  size_t lowp3[PARTITION_VLEN_EACH];
    size_t highp0[PARTITION_VLEN_EACH];
    size_t highp1[PARTITION_VLEN_EACH];
//  size_t highp2[PARTITION_VLEN_EACH];
//  size_t highp3[PARTITION_VLEN_EACH];
#pragma _NEC vreg(lowp0)
#pragma _NEC vreg(lowp1)
//#pragma _NEC vreg(lowp2)
//#pragma _NEC vreg(lowp3)
#pragma _NEC vreg(highp0)
#pragma _NEC vreg(highp1)
//#pragma _NEC vreg(highp2)
//#pragma _NEC vreg(highp3)
#endif

    time_spent tot_t(DEBUG), comp_t(DEBUG), copy_t(DEBUG);
    tot_t.lap_start();
    for(size_t i = 0; i < nrow; ++i) {
      auto kptr = keyptr + offsetptr[i];
      auto size = offsetptr[i + 1] - offsetptr[i];
#ifdef USE_LOOP_RAKING
#include "partition_key_raked.incl"
#else
#include "partition_key_normal.incl"
#endif
    }
    tot_t.lap_stop();
    if(get_selfid() == 0) {
      tot_t.show_lap("partition time: ");
      comp_t.show_lap("  \\_ comparison time: ");
      copy_t.show_lap("  \\_ copy_back time: ");
    }
  }
}

template <class T, class I>
void partition(T* kptr, I* vptr, 
               size_t nrow, size_t ncol, size_t k,
               time_spent& comp_t,
               time_spent& copy_t) {
  partition_by_each_row(kptr, vptr, nrow, ncol, k, comp_t, copy_t);
}

template <class T, class I>
void partition(T* kptr, I* vptr, 
               size_t nrow, size_t ncol, size_t k) {
  time_spent comp_t(DEBUG), copy_t(DEBUG), part_t(DEBUG);
  part_t.lap_start();
  partition(kptr, vptr, nrow, ncol, k, comp_t, copy_t);
  part_t.lap_stop();
  if(get_selfid()== 0) {
    part_t.show_lap("matrix partition time: ");
    comp_t.show_lap("  \\_ comparison time: ");
    copy_t.show_lap("  \\_ copy_back time: ");
  }
}

template <class T, class I>
void partition(T* kptr, 
               size_t nrow, size_t ncol, size_t k,
               time_spent& comp_t,
               time_spent& copy_t) {
  partition_by_each_row(kptr, nrow, ncol, k, comp_t, copy_t);
}

template <class T>
void partition(T* kptr, 
               size_t nrow, size_t ncol, size_t k) {
  time_spent comp_t(DEBUG), copy_t(DEBUG), part_t(DEBUG);
  part_t.lap_start();
  partition(kptr, nrow, ncol, k, comp_t, copy_t);
  part_t.lap_stop();
  if(get_selfid()== 0) {
    part_t.show_lap("matrix partition time: ");
    comp_t.show_lap("  \\_ comparison time: ");
    copy_t.show_lap("  \\_ copy_back time: ");
  }
}

}
#endif
