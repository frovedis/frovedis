

#ifndef GRAPH_SET_UNION_MULTIVEC_HPP
#define	GRAPH_SET_UNION_MULTIVEC_HPP

#define GRAPH_SET_VLEN 1023
#define GRAPH_SET_VLEN_EACH 256
#define GRAPH_SET_VLEN_EACH3 255

#include "binary_search.hpp"
#include "find_partitions.hpp"

namespace frovedis {
    

// #ifndef _SX
#if !defined(_SX) && !defined(__ve__)

template <class T>
std::vector<size_t> set_separate_graph(const std::vector<T>& key) {
  size_t size = key.size();
  std::vector<size_t> ret;
  if(size == 0) {
    ret.push_back(0);
    return ret;
  } else {
    T current = key[0];
    ret.push_back(0);
    for(size_t i = 1; i < size; i++) {
      if(key[i] != current) {
        current = key[i];
        ret.push_back(i);
      }
    }
    ret.push_back(size);
    return ret;
  }
}


#else

template <class T>
std::vector<size_t> set_separate_graph(const std::vector<T>& key) {
  size_t size = key.size();
  int isFinished[GRAPH_SET_VLEN];
  for(int i = 0; i < GRAPH_SET_VLEN; i++) isFinished[i] = false;
  size_t each = frovedis::ceil_div(size, size_t(GRAPH_SET_VLEN));
  if(each % 2 == 0) each++;
  size_t key_idx[GRAPH_SET_VLEN];
  size_t key_idx_stop[GRAPH_SET_VLEN];
  size_t out_idx[GRAPH_SET_VLEN];
  size_t out_idx_save[GRAPH_SET_VLEN];
  T current_key[GRAPH_SET_VLEN];
  std::vector<size_t> out(size,0);
//  out.resize(size);
  size_t* outp = &out[0];
  const T* keyp = &key[0];
  if(size > 0) {
    key_idx[0] = 1;
    outp[0] = 0;
    out_idx[0] = 1;
    out_idx_save[0] = 0;
    current_key[0] = keyp[0];
  } else {
    isFinished[0] = true;
    key_idx[0] = size;
    out_idx[0] = size;
    out_idx_save[0] = size;
  }
  for(int i = 1; i < GRAPH_SET_VLEN; i++) {
    size_t pos = each * i;
    if(pos < size) {
      key_idx[i] = pos;
      out_idx[i] = pos;
      out_idx_save[i] = pos;
      current_key[i] = keyp[pos-1];
    } else {
      isFinished[i] = true;
      key_idx[i] = size;
      out_idx[i] = size;
      out_idx_save[i] = size;
    }
  }
  for(int i = 0; i < GRAPH_SET_VLEN - 1; i++) {
    key_idx_stop[i] = key_idx[i + 1];
  }
  key_idx_stop[GRAPH_SET_VLEN-1] = size;
  if(key_idx[0] == key_idx_stop[0]) isFinished[0] = true; 
  while(1) {
#pragma cdir nodep
#pragma _NEC ivdep      
    for(int i = 0; i < GRAPH_SET_VLEN; i++) {
      if(isFinished[i] == false) {
        auto keyval = keyp[key_idx[i]];
        if(keyval != current_key[i]) {
          outp[out_idx[i]++] = key_idx[i];
          current_key[i] = keyval;
        }
        key_idx[i]++;
        if(key_idx[i] == key_idx_stop[i]) {isFinished[i] = true;}
      }
    }
    bool isFinished_all = true;
    for(int i = 0; i < GRAPH_SET_VLEN; i++) {
        if(isFinished[i] == false) {
            isFinished_all = false;
            break;
        }
    }
    if(isFinished_all == true) break;
  }
  size_t total = 0;
  for(size_t i = 0; i < GRAPH_SET_VLEN; i++) {
    total += out_idx[i] - out_idx_save[i];
  }
  std::vector<size_t> ret(total+1,0);
  size_t* retp = &ret[0];
  size_t current = 0;
  for(size_t i = 0; i < GRAPH_SET_VLEN; i++) {
    for(size_t j = 0; j < out_idx[i] - out_idx_save[i]; j++, current++) {
      retp[current] = out[out_idx_save[i] + j];
    }
  }
  retp[current] = size;
  return ret;
}

#endif

template <class T>
size_t set_union_vertical_unrolled(std::vector<T>& left, std::vector<T>& right,std::vector<T>& out, std::vector<T>& ret) { //parallel binary search for right_idx
 


  int valid[GRAPH_SET_VLEN];
  for(int i = 0; i < GRAPH_SET_VLEN; i++) valid[i] = true;

  size_t left_size = left.size();
  size_t right_size = right.size();
  if(left_size == 0) {
      ret = right;
      return right_size;
  }
  if(right_size == 0) {
      ret  = left;
      return left_size;
  }
  
  size_t each = frovedis::ceil_div(left_size, size_t(GRAPH_SET_VLEN));
  if(each % 2 == 0) each++;
  
  size_t left_idx[GRAPH_SET_VLEN];
  size_t right_idx[GRAPH_SET_VLEN];

  size_t left_idx_stop[GRAPH_SET_VLEN];
  size_t right_idx_stop[GRAPH_SET_VLEN];
  size_t out_idx[GRAPH_SET_VLEN];
  size_t out_idx_save[GRAPH_SET_VLEN];
  size_t valid_count = 0;
  for(int i = 0; i < GRAPH_SET_VLEN; i++) {
    size_t pos = each * i;
    if(pos < left_size) {
      left_idx[i] = pos;
      valid_count ++;
    } else {
      valid[i] = false;
      left_idx[i] = left_size;
    }
  }
  std::vector<T> left_milestone(valid_count-1);
  auto* left_milestonep = &left_milestone[0];

  #pragma cdir on_adb(left_milestonep)
  #pragma cdir on_adb(left)
  #pragma cdir on_adb(left_idx)
  for(size_t i=0; i<valid_count-1; i++){
        left_milestonep[i] = left[left_idx[i+1]];
    }
  std::vector<size_t> right_idx_valid(valid_count-1); //except i=0
  right_idx_valid = lower_bound(right,left_milestone);
  auto* right_idx_valid_ptr = &right_idx_valid[0];
  right_idx[0] = 0;

#pragma cdir on_adb(left_milestonep)
#pragma cdir on_adb(right_idx)
#pragma cdir on_adb(right)
#pragma cdir on_adb(valid)
#pragma cdir on_adb(right_idx_valid_ptr)
  for(size_t i=0; i<GRAPH_SET_VLEN-1; i++){
      if(i< valid_count-1){
          if(left_milestone[i] <= right[right_size-1]){
                right_idx[i+1] = right_idx_valid_ptr[i];
          }
          else{
              valid[i+1] = false;
              right_idx[i+1] = right_size;
          }
      }
      else{
          right_idx[i+1] = right_size;
      }
    }
  
  out_idx[0] = 0;
  out_idx_save[0] = 0;
    #pragma cdir nodep
    #pragma _NEC ivdep
  for(int i = 1; i < GRAPH_SET_VLEN; i++) {
    out_idx[i] = (left_idx[i] - left_idx[i-1])
      + (right_idx[i] - right_idx[i-1])
      + out_idx[i-1];
    out_idx_save[i] = out_idx[i];
  }
  for(int i = 0; i < GRAPH_SET_VLEN - 1; i++) {
    left_idx_stop[i] = left_idx[i + 1];
    right_idx_stop[i] = right_idx[i + 1];
  }
  left_idx_stop[GRAPH_SET_VLEN-1] = left_size;
  right_idx_stop[GRAPH_SET_VLEN-1] = right_size;
  for(int i = 0; i < GRAPH_SET_VLEN; i++) {
    if(left_idx[i] == left_idx_stop[i] || right_idx[i] == right_idx_stop[i]) 
      valid[i] = false;
  }

//#include "frovedis/dataframe/set_operations.incl1"
 #include "./set_operations.incl1"
  T* lp = &left[0];
  T* rp = &right[0];
  T* op = &out[0];
  while(1) {
    
      
//#include "frovedis/dataframe/set_operations.incl2"
#include "./set_operations.incl2"
#include "sparse_vector.hpp"

#pragma cdir nodep
      #pragma _NEC ivdep
    for(int j = 0; j < GRAPH_SET_VLEN_EACH; j++) {
      if(valid_0[j]) {
        bool eq = leftelm0[j] == rightelm0[j];
        bool lt = leftelm0[j] < rightelm0[j];
        if(eq || lt) {
          op[out_idx_0[j]++] = leftelm0[j];
          left_idx_0[j]++;
        } else {
          op[out_idx_0[j]++] = rightelm0[j];
          right_idx_0[j]++;
        }
        if(eq) {
          right_idx_0[j]++;
        }
        if(left_idx_0[j] == left_idx_stop_0[j] ||
           right_idx_0[j] == right_idx_stop_0[j]) {
          valid_0[j] = false;
        }
      }
    }

#pragma cdir nodep
    #pragma _NEC ivdep
    for(int j = 0; j < GRAPH_SET_VLEN_EACH; j++) {
      if(valid_1[j]) {
        bool eq = leftelm1[j] == rightelm1[j];
        bool lt = leftelm1[j] < rightelm1[j];
        if(eq || lt) {
          op[out_idx_1[j]++] = leftelm1[j];
          left_idx_1[j]++;
        } else {
          op[out_idx_1[j]++] = rightelm1[j];
          right_idx_1[j]++;
        }
        if(eq) {
          right_idx_1[j]++;
        }
        if(left_idx_1[j] == left_idx_stop_1[j] ||
           right_idx_1[j] == right_idx_stop_1[j]) {
          valid_1[j] = false;
        }
      }
    }

#pragma cdir nodep       
    #pragma _NEC ivdep
    for(int j = 0; j < GRAPH_SET_VLEN_EACH; j++) {
      if(valid_2[j]) {
        bool eq = leftelm2[j] == rightelm2[j];
        bool lt = leftelm2[j] < rightelm2[j];
        if(eq || lt) {
          op[out_idx_2[j]++] = leftelm2[j];
          left_idx_2[j]++;
        } else {
          op[out_idx_2[j]++] = rightelm2[j];
          right_idx_2[j]++;
        }
        if(eq) {
          right_idx_2[j]++;
        }
        if(left_idx_2[j] == left_idx_stop_2[j] ||
           right_idx_2[j] == right_idx_stop_2[j]) {
          valid_2[j] = false;
        }
      }
    }

#pragma cdir nodep   
    #pragma _NEC ivdep
    for(int j = 0; j < GRAPH_SET_VLEN_EACH3; j++) {
      if(valid_3[j]) {
        bool eq = leftelm3[j] == rightelm3[j];
        bool lt = leftelm3[j] < rightelm3[j];
        if(eq || lt) {
          op[out_idx_3[j]++] = leftelm3[j];
          left_idx_3[j]++;
        } else {
          op[out_idx_3[j]++] = rightelm3[j];
          right_idx_3[j]++;
        }
        if(eq) {
          right_idx_3[j]++;
        }
        if(left_idx_3[j] == left_idx_stop_3[j] ||
           right_idx_3[j] == right_idx_stop_3[j]) {
          valid_3[j] = false;
        }
      }
    }
 
  
    bool any_valid = false;
    #pragma cdir on_adb(valid_0)
    #pragma cdir on_adb(valid_1)
    #pragma cdir on_adb(valid_2)
    #pragma cdir on_adb(valid_3)
    for(int i = 0; i < GRAPH_SET_VLEN_EACH; i++) {
      if(valid_0[i] || valid_1[i] || valid_2[i] || valid_3[i])
        any_valid = true;
    }
    if(any_valid == false) break;
  }

    #pragma cdir on_adb(out_idx)
    #pragma cdir on_adb(out_idx_0)
    #pragma cdir on_adb(out_idx_1)
    #pragma cdir on_adb(out_idx_2)
    #pragma cdir on_adb(left_idx)
    #pragma cdir on_adb(left_idx_0)
    #pragma cdir on_adb(left_idx_1)
    #pragma cdir on_adb(left_idx_2)
    #pragma cdir on_adb(right_idx)
    #pragma cdir on_adb(right_idx_0)
    #pragma cdir on_adb(right_idx_1)
    #pragma cdir on_adb(right_idx_2)
  for(size_t i = 0; i < GRAPH_SET_VLEN_EACH; i++) {
    out_idx[i] = out_idx_0[i];
    out_idx[GRAPH_SET_VLEN_EACH * 1 + i] = out_idx_1[i];
    out_idx[GRAPH_SET_VLEN_EACH * 2 + i] = out_idx_2[i];
    left_idx[i] = left_idx_0[i];
    left_idx[GRAPH_SET_VLEN_EACH * 1 + i] = left_idx_1[i];
    left_idx[GRAPH_SET_VLEN_EACH * 2 + i] = left_idx_2[i];
    right_idx[i] = right_idx_0[i];
    right_idx[GRAPH_SET_VLEN_EACH * 1 + i] = right_idx_1[i];
    right_idx[GRAPH_SET_VLEN_EACH * 2 + i] = right_idx_2[i];
  }
    #pragma cdir on_adb(out_idx_3)
    #pragma cdir on_adb(left_idx_3)
    #pragma cdir on_adb(right_idx_3)
    #pragma cdir on_adb(out_idx)
    #pragma cdir on_adb(left_idx)  
    #pragma cdir on_adb(right_idx)  
  for(size_t i = 0; i < GRAPH_SET_VLEN_EACH3; i++) {
    out_idx[GRAPH_SET_VLEN_EACH * 3 + i] = out_idx_3[i];
    left_idx[GRAPH_SET_VLEN_EACH * 3 + i] = left_idx_3[i];
    right_idx[GRAPH_SET_VLEN_EACH * 3 + i] = right_idx_3[i];
  }
  size_t total = 0;
    #pragma cdir on_adb(out_idx)
    #pragma cdir on_adb(out_idx_save)
    #pragma cdir on_adb(left_idx)  
    #pragma cdir on_adb(right_idx)  
    #pragma cdir on_adb(out_idx)
    #pragma cdir on_adb(left_idx_stop)  
    #pragma cdir on_adb(right_idx_stop)    
  for(size_t i = 0; i < GRAPH_SET_VLEN; i++) {
    total += (out_idx[i] - out_idx_save[i]) +
      (left_idx_stop[i] - left_idx[i]) +
      (right_idx_stop[i] - right_idx[i]);
  }

  T* retp = &ret[0];
  size_t current = 0;
  for(size_t i = 0; i < GRAPH_SET_VLEN; i++) {
    #pragma cdir on_adb(retp)
    #pragma cdir on_adb(op)
    #pragma cdir on_adb(out_idx)
    #pragma cdir on_adb(out_idx_save)
    for(size_t j = 0; j < out_idx[i] - out_idx_save[i]; j++) {
      retp[current++] = op[out_idx_save[i] + j];
    }
    
    #pragma cdir on_adb(retp)
    #pragma cdir on_adb(lp)
    #pragma cdir on_adb(left_idx)
    #pragma cdir on_adb(left_idx_stop)
    for(size_t j = 0; j < left_idx_stop[i] - left_idx[i]; j++) {
      retp[current++] = lp[left_idx[i] + j];
    }
    #pragma cdir on_adb(retp)
    #pragma cdir on_adb(rp)
    #pragma cdir on_adb(right_idx)
    #pragma cdir on_adb(right_idx_stop)
    for(size_t j = 0; j < right_idx_stop[i] - right_idx[i]; j++) {
      retp[current++] = rp[right_idx[i] + j];
    }
  }
  return current;
}

template <class T>
std::vector<T> set_union_vertical(std::vector<T>& left, std::vector<T>& right) { //parallel binary search for right_idx
 
  int valid[GRAPH_SET_VLEN_EACH3];
  for(int i = 0; i < GRAPH_SET_VLEN_EACH3; i++) valid[i] = true;

  size_t left_size = left.size();
  size_t right_size = right.size();
  if(left_size == 0) return right;
  if(right_size == 0) return left;
  T* lp = &left[0];
  T* rp = &right[0];
  
  size_t each = frovedis::ceil_div(left_size, size_t(GRAPH_SET_VLEN_EACH3));
  if(each % 2 == 0) each++;
  
  size_t left_idx[GRAPH_SET_VLEN_EACH3];
  size_t right_idx[GRAPH_SET_VLEN_EACH3];

  size_t left_idx_stop[GRAPH_SET_VLEN_EACH3];
  size_t right_idx_stop[GRAPH_SET_VLEN_EACH3];
  size_t out_idx[GRAPH_SET_VLEN_EACH3];
  size_t out_idx_save[GRAPH_SET_VLEN_EACH3];
  std::vector<T> out(left_size + right_size,0);
//  out.resize(left_size + right_size);
  size_t valid_count = 0;
  
  #pragma cdir on_adb(left_idx)
  for(int i = 0; i < GRAPH_SET_VLEN_EACH3; i++) {
    size_t pos = each * i;
    if(pos < left_size) {
      left_idx[i] = pos;
      valid_count ++;
    } else {
      valid[i] = false;
      left_idx[i] = left_size;
    }
  }
  
  std::vector<T> left_milestone(valid_count-1,0);
  auto* left_milestonep = &left_milestone[0];

    #pragma cdir on_adb(left_milestonep)    
    #pragma cdir on_adb(lp)
    #pragma cdir on_adb(left_idx)
  for(size_t i=0; i<valid_count-1; i++){
        left_milestonep[i] = lp[left_idx[i+1]];
    }
  std::vector<size_t> right_idx_valid(valid_count-1,0); //except i=0
  right_idx_valid = lower_bound(right,left_milestone);
  auto* right_idx_valid_ptr = &right_idx_valid[0];
  right_idx[0] = 0;
#pragma cdir nodep
#pragma _NEC ivdep
#pragma cdir on_adb(left_milestonep)
#pragma cdir on_adb(right_idx)
#pragma cdir on_adb(rp)
#pragma cdir on_adb(valid)
#pragma cdir on_adb(right_idx_valid_ptr)
    for(size_t i=0; i<valid_count-1; i++){
        if(left_milestone[i] <= rp[right_size-1]){
              right_idx[i+1] = right_idx_valid_ptr[i];
        }
        else{
            valid[i+1] = false;
            right_idx[i+1] = right_size;
        }
    }
  
    for(size_t i = valid_count; i< GRAPH_SET_VLEN_EACH3; i++){
        right_idx[i] = right_size;
    }
  
  out_idx[0] = 0;
  out_idx_save[0] = 0;
#pragma cdir nodep
#pragma _NEC ivdep
#pragma cdir on_adb(left_idx)
#pragma cdir on_adb(right_idx)
#pragma cdir on_adb(out_idx)
#pragma cdir on_adb(out_idx_save)
  for(int i = 1; i < GRAPH_SET_VLEN_EACH3; i++) {
    out_idx[i] = (left_idx[i] - left_idx[i-1])
      + (right_idx[i] - right_idx[i-1])
      + out_idx[i-1];
    out_idx_save[i] = out_idx[i];
  }
#pragma cdir on_adb(left_idx_stop)
#pragma cdir on_adb(right_idx_Stop)
#pragma cdir on_adb(left_idx)
#pragma cdir on_adb(right_idx)  
  for(int i = 0; i < GRAPH_SET_VLEN_EACH3 - 1; i++) {
    left_idx_stop[i] = left_idx[i + 1];
    right_idx_stop[i] = right_idx[i + 1];
  }
  left_idx_stop[GRAPH_SET_VLEN_EACH3-1] = left_size;
  right_idx_stop[GRAPH_SET_VLEN_EACH3-1] = right_size;


  T* op = &out[0];
  size_t valid_ct = 0;
  size_t total_ct =0;
  while(1) {
  
#pragma cdir nodep
#pragma _NEC ivdep      
#pragma cdir on_adb(op)
#pragma cdir on_adb(lp)
#pragma cdir on_adb(rp)  
#pragma cdir on_adb(left_idx_stop)
#pragma cdir on_adb(right_idx_Stop)
#pragma cdir on_adb(left_idx)
#pragma cdir on_adb(right_idx)  
#pragma cdir on_adb(valid) 
      
    for(int j = 0; j < GRAPH_SET_VLEN_EACH3; j++) {
        total_ct ++;
      if(! (left_idx[j] == left_idx_stop[j] || right_idx[j] == right_idx_stop[j])) {
          valid_ct ++;
          
        int eq = lp[left_idx[j]] == rp[right_idx[j]];
        int lt = lp[left_idx[j]] < rp[right_idx[j]];
        if(eq || lt) {
          op[out_idx[j]++] = lp[left_idx[j]];
          left_idx[j]++;
        } 
        else {
          op[out_idx[j]++] = rp[right_idx[j]];
          right_idx[j]++;
        }
        if(eq) {
          right_idx[j]++;
        }
      }
        else{
          valid[j] = false;
        }
    }

  
    bool any_valid = false;
    #pragma cdir on_adb(valid)
    for(int i = 0; i < GRAPH_SET_VLEN_EACH3; i++) {
      if(valid[i]){
        any_valid = true;
        break;
      }
    }
    if(any_valid == false) break;
  }

  size_t total = 0;
    #pragma cdir on_adb(out_idx)
    #pragma cdir on_adb(out_idx_save)
    #pragma cdir on_adb(left_idx)  
    #pragma cdir on_adb(right_idx)  
    #pragma cdir on_adb(out_idx)
    #pragma cdir on_adb(left_idx_stop)  
    #pragma cdir on_adb(right_idx_stop)    
  for(size_t i = 0; i < GRAPH_SET_VLEN_EACH3; i++) {
    total += (out_idx[i] - out_idx_save[i]) +
      (left_idx_stop[i] - left_idx[i]) +
      (right_idx_stop[i] - right_idx[i]);
  }
  std::vector<T> ret(total,0);
  T* retp = &ret[0];
  size_t current = 0;
#pragma cdir on_adb(op)   
#pragma cdir on_adb(retp)  
  for(size_t i = 0; i < GRAPH_SET_VLEN_EACH3; i++) {
    #pragma cdir on_adb(retp)
    #pragma cdir on_adb(op)
    #pragma cdir on_adb(out_idx)
    #pragma cdir on_adb(out_idx_save)
    for(size_t j = 0; j < out_idx[i] - out_idx_save[i]; j++) {
      retp[current++] = op[out_idx_save[i] + j];
    }
    #pragma cdir on_adb(retp)
    #pragma cdir on_adb(lp)
    #pragma cdir on_adb(left_idx)
    #pragma cdir on_adb(left_idx_stop)
    for(size_t j = 0; j < left_idx_stop[i] - left_idx[i]; j++) {
      retp[current++] = lp[left_idx[i] + j];
    }
    #pragma cdir on_adb(retp)
    #pragma cdir on_adb(rp)
    #pragma cdir on_adb(right_idx)
    #pragma cdir on_adb(right_idx_stop)
    for(size_t j = 0; j < right_idx_stop[i] - right_idx[i]; j++) {
      retp[current++] = rp[right_idx[i] + j];
    }
  }
  return ret;
}



template <class T>
std::vector<T> set_union_vertical_overlap(std::vector<T>& left, std::vector<T>& right) { 
    //parallel binary search for right_idx
    //only compare overlap part 


  int valid[GRAPH_SET_VLEN_EACH3];
  for(int i = 0; i < GRAPH_SET_VLEN_EACH3; i++) valid[i] = true;

  size_t left_size = left.size();
  size_t right_size = right.size();

  if(left_size == 0) return right;
  if(right_size == 0) return left;
  
  
  T* lp = &left[0];
  T* rp = &right[0];
  auto first_elem_left = lp[0];
  auto first_elem_right = rp[0];
  auto last_elem_left = lp[left_size-1];
  auto last_elem_right = rp[right_size-1];
  auto idx_first_elem_right_in_left = std::lower_bound(left.begin(),left.end(), first_elem_right) - left.begin();
  auto idx_last_elem_right_in_left = std::lower_bound(left.begin(),left.end(), last_elem_right) - left.begin();
  
  auto idx_first_elem_left_in_right = std::lower_bound(right.begin(),right.end(), first_elem_left) - right.begin();  
  auto idx_last_elem_left_in_right = std::lower_bound(right.begin(),right.end(), last_elem_left) - right.begin();
                  
  std::vector<T>  smallest_v;
  std::vector<T> largest_v;

  if(idx_last_elem_right_in_left < left_size ){
      if(lp[idx_last_elem_right_in_left] == last_elem_right){
        idx_last_elem_left_in_right --;
      }
    largest_v.resize(left_size-idx_last_elem_right_in_left);
    auto* largest_v_ptr = &largest_v[0]; 
    #pragma cdir on_adb(largest_v_ptr)
    #pragma cdir on_adb(lp)    
    for(size_t k=0; k< largest_v.size();k++){
        largest_v_ptr[k] = lp[idx_last_elem_right_in_left + k];
    }
  }
  else if(idx_last_elem_left_in_right < right_size ){
      if(rp[idx_last_elem_left_in_right] == last_elem_left){
        idx_last_elem_right_in_left --;
      }
    largest_v.resize(right_size-idx_last_elem_left_in_right);
    auto* largest_v_ptr = &largest_v[0]; 
    #pragma cdir on_adb(largest_v_ptr)
    #pragma cdir on_adb(rp)   
    for(size_t k = 0; k< largest_v.size();k++){
        largest_v_ptr[k] = rp[idx_last_elem_left_in_right + k];
    }
   } 

  if(idx_first_elem_left_in_right > 0){ 
    smallest_v.resize(idx_first_elem_left_in_right);
      auto* smallest_v_ptr = &smallest_v[0]; 
    #pragma cdir on_adb(smallest_v_ptr)
    #pragma cdir on_adb(rp)    
    for(size_t i = 0; i< idx_first_elem_left_in_right;i++){
        smallest_v_ptr[i] = rp[i];
    }
  }
  else if(idx_first_elem_right_in_left > 0){
    smallest_v.resize(idx_first_elem_right_in_left);
      auto* smallest_v_ptr = &smallest_v[0]; 
    #pragma cdir on_adb(smallest_v_ptr)
    #pragma cdir on_adb(lp)     
    for(size_t i = 0; i< idx_first_elem_right_in_left;i++){
        smallest_v_ptr[i] = lp[i];
    }     
  }
  

   size_t smallest_v_size = smallest_v.size();
   size_t largest_v_size = largest_v.size(); 
   
   
    std::vector<T> new_left(idx_last_elem_right_in_left - idx_first_elem_right_in_left);
    auto* new_lp = &new_left[0];
    auto new_left_size = new_left.size();
  
    std::vector<T> new_right(idx_last_elem_left_in_right - idx_first_elem_left_in_right);
    auto* new_rp = &new_right[0];  
    auto new_right_size = new_right.size();
    #pragma cdir on_adb(lp)
    #pragma cdir on_adb(new_lp)    
  for(size_t i = 0; i< idx_last_elem_right_in_left - idx_first_elem_right_in_left; i++){
      new_lp[i] = lp[i + idx_first_elem_right_in_left];
  }
    #pragma cdir on_adb(rp)
    #pragma cdir on_adb(new_rp)     
  for(size_t i = 0; i<idx_last_elem_left_in_right - idx_first_elem_left_in_right;i++){
      new_rp[i] = rp[i +idx_first_elem_left_in_right];
  }
    
  std::vector<T> ret;
  ret.reserve(left_size + right_size);
    if(new_left_size == 0){
        auto* smallest_v_ptr = &smallest_v[0];
        auto* largest_v_ptr = &largest_v[0];
        ret.resize(smallest_v_size + largest_v_size + new_right_size);
        auto* retp = &ret[0];
        size_t current = 0;
        #pragma cdir on_adb(smallest_v_ptr)
        #pragma cdir on_adb(retp)         
         for(size_t i=0;i<smallest_v_size;i++){
            retp[current++] = smallest_v_ptr[i];
        } 
        #pragma cdir on_adb(new_rp)
        #pragma cdir on_adb(retp)         
         for(size_t i=0;i<new_right_size;i++){
            retp[current++] = new_rp[i];
        }
        #pragma cdir on_adb(largest_v_ptr)
        #pragma cdir on_adb(retp)        
        for(size_t i=0;i<largest_v_size;i++){
            retp[current++] = largest_v_ptr[i];
        }
        return ret;
    }
  else if(new_right_size == 0){
        auto* smallest_v_ptr = &smallest_v[0];
        auto* largest_v_ptr = &largest_v[0];
        ret.resize(smallest_v_size + largest_v_size + new_left_size);
        auto* retp = &ret[0];
        size_t current = 0;
        #pragma cdir on_adb(smallest_v_ptr)
        #pragma cdir on_adb(retp)        
         for(size_t i=0;i<smallest_v_size;i++){
            retp[current++] = smallest_v_ptr[i];
        } 
        #pragma cdir on_adb(new_lp)
        #pragma cdir on_adb(retp)          
         for(size_t i=0;i<new_left_size;i++){
            retp[current++] = new_lp[i];
        } 
        #pragma cdir on_adb(largest_v_ptr)
        #pragma cdir on_adb(retp)           
        for(size_t i=0;i<largest_v_size;i++){
            retp[current++] = largest_v_ptr[i];
        }
        return ret;
    }

  size_t each = frovedis::ceil_div(new_left_size, size_t(GRAPH_SET_VLEN_EACH3));
  if(each % 2 == 0) each++;
  
  size_t left_idx[GRAPH_SET_VLEN_EACH3];
  size_t right_idx[GRAPH_SET_VLEN_EACH3];

  size_t left_idx_stop[GRAPH_SET_VLEN_EACH3];
  size_t right_idx_stop[GRAPH_SET_VLEN_EACH3];
  size_t out_idx[GRAPH_SET_VLEN_EACH3];
  size_t out_idx_save[GRAPH_SET_VLEN_EACH3];
  std::vector<T> out(new_left_size + new_right_size);
//  out.resize(left_size + right_size);
  size_t valid_count = 0;
  
  #pragma cdir on_adb(left_idx)
  for(int i = 0; i < GRAPH_SET_VLEN_EACH3; i++) {
    size_t pos = each * i;
    if(pos < new_left_size) {
      left_idx[i] = pos;
      valid_count ++;
    } else {
      valid[i] = false;
      left_idx[i] = new_left_size;
    }
  }
        
  std::vector<T> left_milestone(valid_count-1);
  auto* left_milestonep = &left_milestone[0];

    #pragma cdir on_adb(left_milestonep)    
    #pragma cdir on_adb(new_lp)
    #pragma cdir on_adb(left_idx)
  for(size_t i=0; i<valid_count-1; i++){
        left_milestonep[i] = new_lp[left_idx[i+1]];
    }
  std::vector<size_t> right_idx_valid(valid_count-1); //except i=0
  right_idx_valid = lower_bound(new_right,left_milestone);
  
  auto* right_idx_valid_ptr = &right_idx_valid[0];
  right_idx[0] = 0;

    #pragma cdir on_adb(right_idx)
    #pragma cdir on_adb(right_idx_valid_ptr)
    #pragma cdir on_adb(valid)
     for(size_t i=1; i<valid_count; i++){
        right_idx[i] = right_idx_valid_ptr[i-1];
        if(right_idx[i] >= new_right_size){
            valid[i] = false;
        }
    }
   
    for(size_t i = valid_count; i< GRAPH_SET_VLEN_EACH3; i++){
        right_idx[i] = new_right_size;
    }

  out_idx[0] = 0;
  out_idx_save[0] = 0;
#pragma cdir nodep
#pragma _NEC ivdep  
#pragma cdir on_adb(left_idx)
#pragma cdir on_adb(right_idx)
#pragma cdir on_adb(out_idx)
#pragma cdir on_adb(out_idx_save)
  for(int i = 1; i < GRAPH_SET_VLEN_EACH3; i++) {
    out_idx[i] = (left_idx[i] - left_idx[i-1])
      + (right_idx[i] - right_idx[i-1])
      + out_idx[i-1];
    out_idx_save[i] = out_idx[i];
  }
#pragma cdir on_adb(left_idx_stop)
#pragma cdir on_adb(right_idx_Stop)
#pragma cdir on_adb(left_idx)
#pragma cdir on_adb(right_idx)  
  for(int i = 0; i < GRAPH_SET_VLEN_EACH3 - 1; i++) {
    left_idx_stop[i] = left_idx[i + 1];
    right_idx_stop[i] = right_idx[i + 1];
  }
  left_idx_stop[GRAPH_SET_VLEN_EACH3-1] = new_left_size;
  right_idx_stop[GRAPH_SET_VLEN_EACH3-1] = new_right_size;
  
  T* op = &out[0];

  while(1) {

#pragma cdir nodep
#pragma _NEC ivdep    
#pragma cdir on_adb(op)
#pragma cdir on_adb(new_lp)
#pragma cdir on_adb(new_rp)  
#pragma cdir on_adb(left_idx_stop)
#pragma cdir on_adb(right_idx_Stop)
#pragma cdir on_adb(left_idx)
#pragma cdir on_adb(right_idx)  
#pragma cdir on_adb(valid) 
    for(int j = 0; j < GRAPH_SET_VLEN_EACH3; j++) {
      if(! (left_idx[j] == left_idx_stop[j] || right_idx[j] == right_idx_stop[j])) {
   #ifdef DEBUG_SET_UNION         
          valid_ct_each ++;
    #endif   
        bool eq = new_lp[left_idx[j]] == new_rp[right_idx[j]];
        bool lt = new_lp[left_idx[j]] < new_rp[right_idx[j]];
        if(eq || lt) {
          op[out_idx[j]++] = new_lp[left_idx[j]];
          left_idx[j]++;
        } 
        else {
          op[out_idx[j]++] = new_rp[right_idx[j]];
          right_idx[j]++;
        }
        if(eq) {
          right_idx[j]++;
        }
      }
        else{
          valid[j] = false;
        }
    }
    
    bool any_valid = false;
    #pragma cdir on_adb(valid)
    for(int i = 0; i < GRAPH_SET_VLEN_EACH3; i++) {
      if(valid[i]){
        any_valid = true;
        break;
      }
    }
    if(any_valid == false) break;
    
  }

  size_t total = 0;
    #pragma cdir on_adb(out_idx)
    #pragma cdir on_adb(out_idx_save)
    #pragma cdir on_adb(left_idx)  
    #pragma cdir on_adb(right_idx)  
    #pragma cdir on_adb(out_idx)
    #pragma cdir on_adb(left_idx_stop)  
    #pragma cdir on_adb(right_idx_stop)    
  for(size_t i = 0; i < GRAPH_SET_VLEN_EACH3; i++) {
    total += (out_idx[i] - out_idx_save[i]) +
      (left_idx_stop[i] - left_idx[i]) +
      (right_idx_stop[i] - right_idx[i]);
  }
    auto* smallest_v_ptr = &smallest_v[0];
    auto* largest_v_ptr = &largest_v[0];
  ret.resize(total + smallest_v_size + largest_v_size);
  T* retp = &ret[0];
  size_t current = 0;
  for(size_t i=0;i<smallest_v_size;i++){
      retp[current++] = smallest_v_ptr[i];
  }
  
#pragma cdir on_adb(op)   
#pragma cdir on_adb(retp)  
  for(size_t i = 0; i < GRAPH_SET_VLEN_EACH3; i++) {
    #pragma cdir on_adb(retp)
    #pragma cdir on_adb(op)
    #pragma cdir on_adb(out_idx)
    #pragma cdir on_adb(out_idx_save)
    for(size_t j = 0; j < out_idx[i] - out_idx_save[i]; j++) {
      retp[current++] = op[out_idx_save[i] + j];
    }
    #pragma cdir on_adb(retp)
    #pragma cdir on_adb(new_lp)
    #pragma cdir on_adb(left_idx)
    #pragma cdir on_adb(left_idx_stop)
    for(size_t j = 0; j < left_idx_stop[i] - left_idx[i]; j++) {
      retp[current++] = new_lp[left_idx[i] + j];
    }
    #pragma cdir on_adb(retp)
    #pragma cdir on_adb(new_rp)
    #pragma cdir on_adb(right_idx)
    #pragma cdir on_adb(right_idx_stop)
    for(size_t j = 0; j < right_idx_stop[i] - right_idx[i]; j++) {
      retp[current++] = new_rp[right_idx[i] + j];
    }
  }
  for(size_t i=0;i<largest_v_size;i++){
      retp[current++] = largest_v_ptr[i];
  }  
  return ret;
}

template <class T>
std::vector<T> set_union_mergepath(std::vector<T>& left, std::vector<T>& right) { 
    //Partition left and right vectors using mergepath technique for load-balance

  int valid[GRAPH_SET_VLEN_EACH3];
  for(int i = 0; i < GRAPH_SET_VLEN_EACH3; i++) valid[i] = true;

  size_t left_size = left.size();
  size_t right_size = right.size();

  if(left_size == 0) return right;
  if(right_size == 0) return left;
  
  T* lp = &left[0];
  T* rp = &right[0];


//Partition left/right vectors
  
  size_t left_idx[GRAPH_SET_VLEN_EACH3];
  size_t right_idx[GRAPH_SET_VLEN_EACH3];

  size_t left_idx_stop[GRAPH_SET_VLEN_EACH3];
  size_t right_idx_stop[GRAPH_SET_VLEN_EACH3];
  size_t out_idx[GRAPH_SET_VLEN_EACH3];
  size_t out_idx_save[GRAPH_SET_VLEN_EACH3];
  
  std::vector<T> out(left_size + right_size,0);

  std::vector<size_t> lpar(GRAPH_SET_VLEN_EACH3,0);
  std::vector<size_t> rpar(GRAPH_SET_VLEN_EACH3,0); 
  auto* lparp = &lpar[0];
  auto* rparp = &rpar[0];  
  find_partitions(left, right, GRAPH_SET_VLEN_EACH3,lpar,rpar);

  right_idx[0] = 0;
  left_idx[0] = 0;
    #pragma cdir on_adb(lpar)
    #pragma cdir on_adb(left_idx)
    #pragma cdir on_adb(valid)
     for(size_t i=1; i<GRAPH_SET_VLEN_EACH3; i++){
        if(lparp[i-1] >= left_size){
            left_idx[i] = left_size;
            valid[i] = false;
        }else{
            left_idx[i] = lparp[i-1];  
        }   
     }
    #pragma cdir on_adb(rpar)
    #pragma cdir on_adb(right_idx)
    #pragma cdir on_adb(valid)  
     for(size_t i=1; i<GRAPH_SET_VLEN_EACH3; i++){  
        if(rparp[i-1] >= right_size){
            right_idx[i] = right_size;
            valid[i] = false;
        }else{
            right_idx[i] =rparp[i-1];  
        }         
    }

  out_idx[0] = 0;
  out_idx_save[0] = 0;
#pragma cdir nodep
#pragma _NEC ivdep  
#pragma cdir on_adb(left_idx)
#pragma cdir on_adb(right_idx)
#pragma cdir on_adb(out_idx)
#pragma cdir on_adb(out_idx_save)
  for(int i = 1; i < GRAPH_SET_VLEN_EACH3; i++) {
    out_idx[i] = (left_idx[i] - left_idx[i-1])
      + (right_idx[i] - right_idx[i-1])
      + out_idx[i-1];
    out_idx_save[i] = out_idx[i];
  }
#pragma cdir on_adb(left_idx_stop)
#pragma cdir on_adb(right_idx_Stop)
#pragma cdir on_adb(left_idx)
#pragma cdir on_adb(right_idx)  
  for(int i = 0; i < GRAPH_SET_VLEN_EACH3 - 1; i++) {
    left_idx_stop[i] = left_idx[i + 1];
    right_idx_stop[i] = right_idx[i + 1];
  }
  left_idx_stop[GRAPH_SET_VLEN_EACH3-1] = left_size;
  right_idx_stop[GRAPH_SET_VLEN_EACH3-1] = right_size;
  
#pragma cdir on_adb(left_idx_stop)
#pragma cdir on_adb(right_idx_Stop)
#pragma cdir on_adb(left_idx)
#pragma cdir on_adb(right_idx)
#pragma cdir on_adb(valid)  
  for(int i = 0; i < GRAPH_SET_VLEN_EACH3; i++) {
    if(left_idx[i] == left_idx_stop[i] || right_idx[i] == right_idx_stop[i]) 
      valid[i] = false;
  }
  
  T* op = &out[0];
  while(1) {

#pragma cdir nodep
#pragma _NEC ivdep    
#pragma cdir on_adb(op)
#pragma cdir on_adb(lp)
#pragma cdir on_adb(rp)  
#pragma cdir on_adb(left_idx_stop)
#pragma cdir on_adb(right_idx_Stop)
#pragma cdir on_adb(left_idx)
#pragma cdir on_adb(right_idx)  
#pragma cdir on_adb(valid) 
    for(int j = 0; j < GRAPH_SET_VLEN_EACH3; j++) {
      if(! (left_idx[j] == left_idx_stop[j] || right_idx[j] == right_idx_stop[j])) {

        int eq = lp[left_idx[j]] == rp[right_idx[j]];
        int lt = lp[left_idx[j]] < rp[right_idx[j]];
        if(eq || lt) {
          op[out_idx[j]++] = lp[left_idx[j]];
          left_idx[j]++;
        } 
        else {
          op[out_idx[j]++] = rp[right_idx[j]];
          right_idx[j]++;
        }
        if(eq) {
          right_idx[j]++;
        }
      }
        else{
          valid[j] = false;
        }
    }

    bool any_valid = false;
    #pragma cdir on_adb(valid)
    for(int i = 0; i < GRAPH_SET_VLEN_EACH3; i++) {
        if(valid[i]){
          any_valid = true;
          break;
        }
    }
    if(any_valid == false) break;
    
  }

  size_t total = 0;
    #pragma cdir on_adb(out_idx)
    #pragma cdir on_adb(out_idx_save)
    #pragma cdir on_adb(left_idx)  
    #pragma cdir on_adb(right_idx)  
    #pragma cdir on_adb(out_idx)
    #pragma cdir on_adb(left_idx_stop)  
    #pragma cdir on_adb(right_idx_stop)    
  for(size_t i = 0; i < GRAPH_SET_VLEN_EACH3; i++) {
    total += (out_idx[i] - out_idx_save[i]) +
      (left_idx_stop[i] - left_idx[i]) +
      (right_idx_stop[i] - right_idx[i]);
  }

  std::vector<T> ret(total,0);
  T* retp = &ret[0];
  size_t current = 0;

#pragma cdir on_adb(op)   
#pragma cdir on_adb(retp)  
  for(size_t i = 0; i < GRAPH_SET_VLEN_EACH3; i++) {
    #pragma cdir on_adb(retp)
    #pragma cdir on_adb(op)
    #pragma cdir on_adb(out_idx)
    #pragma cdir on_adb(out_idx_save)
    for(size_t j = 0; j < out_idx[i] - out_idx_save[i]; j++) {
      retp[current++] = op[out_idx_save[i] + j];
    }
    #pragma cdir on_adb(retp)
    #pragma cdir on_adb(lp)
    #pragma cdir on_adb(left_idx)
    #pragma cdir on_adb(left_idx_stop)
    for(size_t j = 0; j < left_idx_stop[i] - left_idx[i]; j++) {
      retp[current++] = lp[left_idx[i] + j];
    }
    #pragma cdir on_adb(retp)
    #pragma cdir on_adb(rp)
    #pragma cdir on_adb(right_idx)
    #pragma cdir on_adb(right_idx_stop)
    for(size_t j = 0; j < right_idx_stop[i] - right_idx[i]; j++) {
      retp[current++] = rp[right_idx[i] + j];
    }
  }

  return ret;
}


template <class T>
std::vector<T> set_union_vertical_overlap_mergepath(std::vector<T>& left, std::vector<T>& right) { //parallel binary search for right_idx
  //overlap + mergepath


  int valid[GRAPH_SET_VLEN_EACH3];
  for(int i = 0; i < GRAPH_SET_VLEN_EACH3; i++) valid[i] = true;

  size_t left_size = left.size();
  size_t right_size = right.size();

  if(left_size == 0) return right;
  if(right_size == 0) return left;
  
  T* lp = &left[0];
  T* rp = &right[0];
  auto first_elem_left = lp[0];
  auto first_elem_right = rp[0];
  auto last_elem_left = lp[left_size-1];
  auto last_elem_right = rp[right_size-1];

  auto idx_first_elem_right_in_left = std::lower_bound(left.begin(),left.end(), first_elem_right) - left.begin();
  auto idx_last_elem_right_in_left = std::lower_bound(left.begin(),left.end(), last_elem_right) - left.begin();

  auto idx_first_elem_left_in_right = std::lower_bound(right.begin(),right.end(), first_elem_left) - right.begin();  
  auto idx_last_elem_left_in_right = std::lower_bound(right.begin(),right.end(), last_elem_left) - right.begin();
                    
  std::vector<T>  smallest_v;
  std::vector<T> largest_v;

  if(idx_last_elem_right_in_left < left_size ){
      if(lp[idx_last_elem_right_in_left] == last_elem_right){
        idx_last_elem_left_in_right --;
      }
    largest_v.resize(left_size-idx_last_elem_right_in_left);
    auto* largest_v_ptr = &largest_v[0]; 
    #pragma cdir on_adb(largest_v_ptr)
    #pragma cdir on_adb(lp)    
    for(size_t k=0; k< largest_v.size();k++){
        largest_v_ptr[k] = lp[idx_last_elem_right_in_left + k];
    }
  }
  else if(idx_last_elem_left_in_right < right_size ){
      if(rp[idx_last_elem_left_in_right] == last_elem_left){
        idx_last_elem_right_in_left --;
      }
    largest_v.resize(right_size-idx_last_elem_left_in_right);
    auto* largest_v_ptr = &largest_v[0]; 
    #pragma cdir on_adb(largest_v_ptr)
    #pragma cdir on_adb(rp)   
    for(size_t k = 0; k< largest_v.size();k++){
        largest_v_ptr[k] = rp[idx_last_elem_left_in_right + k];
    }
   } 

  if(idx_first_elem_left_in_right > 0){ 
    smallest_v.resize(idx_first_elem_left_in_right);
      auto* smallest_v_ptr = &smallest_v[0]; 
    #pragma cdir on_adb(smallest_v_ptr)
    #pragma cdir on_adb(rp)    
    for(size_t i = 0; i< idx_first_elem_left_in_right;i++){
        smallest_v_ptr[i] = rp[i];
    }
  }
  else if(idx_first_elem_right_in_left > 0){
    smallest_v.resize(idx_first_elem_right_in_left);
      auto* smallest_v_ptr = &smallest_v[0]; 
    #pragma cdir on_adb(smallest_v_ptr)
    #pragma cdir on_adb(lp)     
    for(size_t i = 0; i< idx_first_elem_right_in_left;i++){
        smallest_v_ptr[i] = lp[i];
    }     
  }
  

  size_t smallest_v_size = smallest_v.size();
   size_t largest_v_size = largest_v.size(); 
   
   
    std::vector<T> new_left(idx_last_elem_right_in_left - idx_first_elem_right_in_left);
    auto* new_lp = &new_left[0];
    auto new_left_size = new_left.size();
  
    std::vector<T> new_right(idx_last_elem_left_in_right - idx_first_elem_left_in_right);
    auto* new_rp = &new_right[0];  
    auto new_right_size = new_right.size();
    #pragma cdir on_adb(lp)
    #pragma cdir on_adb(new_lp)    
  for(size_t i = 0; i< idx_last_elem_right_in_left - idx_first_elem_right_in_left; i++){
      new_lp[i] = lp[i + idx_first_elem_right_in_left];
  }
    #pragma cdir on_adb(rp)
    #pragma cdir on_adb(new_rp)     
  for(size_t i = 0; i<idx_last_elem_left_in_right - idx_first_elem_left_in_right;i++){
      new_rp[i] = rp[i +idx_first_elem_left_in_right];
  }
    
  std::vector<T> ret;
  ret.reserve(left_size + right_size);
    if(new_left_size == 0){
        auto* smallest_v_ptr = &smallest_v[0];
        auto* largest_v_ptr = &largest_v[0];
        ret.resize(smallest_v_size + largest_v_size + new_right_size);
        auto* retp = &ret[0];
        size_t current = 0;
        #pragma cdir on_adb(smallest_v_ptr)
        #pragma cdir on_adb(retp)         
         for(size_t i=0;i<smallest_v_size;i++){
            retp[current++] = smallest_v_ptr[i];
        } 
        #pragma cdir on_adb(new_rp)
        #pragma cdir on_adb(retp)         
         for(size_t i=0;i<new_right_size;i++){
            retp[current++] = new_rp[i];
        }
        #pragma cdir on_adb(largest_v_ptr)
        #pragma cdir on_adb(retp)        
        for(size_t i=0;i<largest_v_size;i++){
            retp[current++] = largest_v_ptr[i];
        }
        return ret;
    }
  else if(new_right_size == 0){
        auto* smallest_v_ptr = &smallest_v[0];
        auto* largest_v_ptr = &largest_v[0];
        ret.resize(smallest_v_size + largest_v_size + new_left_size);
        auto* retp = &ret[0];
        size_t current = 0;
        #pragma cdir on_adb(smallest_v_ptr)
        #pragma cdir on_adb(retp)        
         for(size_t i=0;i<smallest_v_size;i++){
            retp[current++] = smallest_v_ptr[i];
        } 
        #pragma cdir on_adb(new_lp)
        #pragma cdir on_adb(retp)          
         for(size_t i=0;i<new_left_size;i++){
            retp[current++] = new_lp[i];
        } 
        #pragma cdir on_adb(largest_v_ptr)
        #pragma cdir on_adb(retp)           
        for(size_t i=0;i<largest_v_size;i++){
            retp[current++] = largest_v_ptr[i];
        }
        return ret;
    }
//Partition left/right vectors
  
  size_t left_idx[GRAPH_SET_VLEN_EACH3];
  size_t right_idx[GRAPH_SET_VLEN_EACH3];

  size_t left_idx_stop[GRAPH_SET_VLEN_EACH3];
  size_t right_idx_stop[GRAPH_SET_VLEN_EACH3];
  size_t out_idx[GRAPH_SET_VLEN_EACH3];
  size_t out_idx_save[GRAPH_SET_VLEN_EACH3];
  
  std::vector<T> out(new_left_size + new_right_size);
  
//  size_t valid_count = 0;
  
   std::vector<size_t> lpar(GRAPH_SET_VLEN_EACH3);
  std::vector<size_t> rpar(GRAPH_SET_VLEN_EACH3); 
  auto* lparp = &lpar[0];
  auto* rparp = &rpar[0];  
  find_partitions(new_left, new_right, GRAPH_SET_VLEN_EACH3,lpar,rpar);

  right_idx[0] = 0;
  left_idx[0] = 0;
    #pragma cdir on_adb(lpar)
    #pragma cdir on_adb(left_idx)
    #pragma cdir on_adb(valid)
     for(size_t i=1; i<GRAPH_SET_VLEN_EACH3; i++){
        if(lparp[i-1] >= new_left_size){
            left_idx[i] = new_left_size;
            valid[i] = false;
        }else{
            left_idx[i] = lparp[i-1];  
        }   
     }
    #pragma cdir on_adb(rpar)
    #pragma cdir on_adb(right_idx)
    #pragma cdir on_adb(valid)  
     for(size_t i=1; i<GRAPH_SET_VLEN_EACH3; i++){  
        if(rparp[i-1] >= new_right_size){
            right_idx[i] = new_right_size;
            valid[i] = false;
        }else{
            right_idx[i] =rparp[i-1];  
        }         
    }

  out_idx[0] = 0;
  out_idx_save[0] = 0;
#pragma cdir nodep
#pragma _NEC ivdep  
#pragma cdir on_adb(left_idx)
#pragma cdir on_adb(right_idx)
#pragma cdir on_adb(out_idx)
#pragma cdir on_adb(out_idx_save)
  for(int i = 1; i < GRAPH_SET_VLEN_EACH3; i++) {
    out_idx[i] = (left_idx[i] - left_idx[i-1])
      + (right_idx[i] - right_idx[i-1])
      + out_idx[i-1];
    out_idx_save[i] = out_idx[i];
  }
#pragma cdir on_adb(left_idx_stop)
#pragma cdir on_adb(right_idx_Stop)
#pragma cdir on_adb(left_idx)
#pragma cdir on_adb(right_idx)  
  for(int i = 0; i < GRAPH_SET_VLEN_EACH3 - 1; i++) {
    left_idx_stop[i] = left_idx[i + 1];
    right_idx_stop[i] = right_idx[i + 1];
  }
  left_idx_stop[GRAPH_SET_VLEN_EACH3-1] = new_left_size;
  right_idx_stop[GRAPH_SET_VLEN_EACH3-1] = new_right_size;
  
#pragma cdir on_adb(left_idx_stop)
#pragma cdir on_adb(right_idx_Stop)
#pragma cdir on_adb(left_idx)
#pragma cdir on_adb(right_idx)
#pragma cdir on_adb(valid)  
  for(int i = 0; i < GRAPH_SET_VLEN_EACH3; i++) {
    if(left_idx[i] == left_idx_stop[i] || right_idx[i] == right_idx_stop[i]) 
      valid[i] = false;
  }
  
  T* op = &out[0];

  while(1) {
#pragma cdir nodep
#pragma _NEC ivdep    
#pragma cdir on_adb(op)
#pragma cdir on_adb(new_lp)
#pragma cdir on_adb(new_rp)  
#pragma cdir on_adb(left_idx_stop)
#pragma cdir on_adb(right_idx_Stop)
#pragma cdir on_adb(left_idx)
#pragma cdir on_adb(right_idx)  
#pragma cdir on_adb(valid) 
    for(int j = 0; j < GRAPH_SET_VLEN_EACH3; j++) {
      if(! (left_idx[j] == left_idx_stop[j] || right_idx[j] == right_idx_stop[j])) {
   #ifdef DEBUG_SET_UNION         
          valid_ct_each ++;
    #endif   
        bool eq = new_lp[left_idx[j]] == new_rp[right_idx[j]];
        bool lt = new_lp[left_idx[j]] < new_rp[right_idx[j]];
        if(eq || lt) {
          op[out_idx[j]++] = new_lp[left_idx[j]];
          left_idx[j]++;
        } 
        else {
          op[out_idx[j]++] = new_rp[right_idx[j]];
          right_idx[j]++;
        }
        if(eq) {
          right_idx[j]++;
        }
      }
        else{
          valid[j] = false;
        }
    }

    bool any_valid = false;
    #pragma cdir on_adb(valid)
    for(int i = 0; i < GRAPH_SET_VLEN_EACH3; i++) {
      if(valid[i]){
        any_valid = true;
        break;
      }
    }
    if(any_valid == false) break;
    
  }
 
  size_t total = 0;
    #pragma cdir on_adb(out_idx)
    #pragma cdir on_adb(out_idx_save)
    #pragma cdir on_adb(left_idx)  
    #pragma cdir on_adb(right_idx)  
    #pragma cdir on_adb(out_idx)
    #pragma cdir on_adb(left_idx_stop)  
    #pragma cdir on_adb(right_idx_stop)    
  for(size_t i = 0; i < GRAPH_SET_VLEN_EACH3; i++) {
    total += (out_idx[i] - out_idx_save[i]) +
      (left_idx_stop[i] - left_idx[i]) +
      (right_idx_stop[i] - right_idx[i]);
  }
    auto* smallest_v_ptr = &smallest_v[0];
    auto* largest_v_ptr = &largest_v[0];
    ret.resize(total + smallest_v_size + largest_v_size);
    T* retp = &ret[0];
    size_t current = 0;
    for(size_t i=0;i<smallest_v_size;i++){
        retp[current++] = smallest_v_ptr[i];
    }
  
    #pragma cdir on_adb(op)   
    #pragma cdir on_adb(retp)  
  for(size_t i = 0; i < GRAPH_SET_VLEN_EACH3; i++) {
    #pragma cdir on_adb(retp)
    #pragma cdir on_adb(op)
    #pragma cdir on_adb(out_idx)
    #pragma cdir on_adb(out_idx_save)
    for(size_t j = 0; j < out_idx[i] - out_idx_save[i]; j++) {
      retp[current++] = op[out_idx_save[i] + j];
    }
    #pragma cdir on_adb(retp)
    #pragma cdir on_adb(new_lp)
    #pragma cdir on_adb(left_idx)
    #pragma cdir on_adb(left_idx_stop)
    for(size_t j = 0; j < left_idx_stop[i] - left_idx[i]; j++) {
      retp[current++] = new_lp[left_idx[i] + j];
    }
    #pragma cdir on_adb(retp)
    #pragma cdir on_adb(new_rp)
    #pragma cdir on_adb(right_idx)
    #pragma cdir on_adb(right_idx_stop)
    for(size_t j = 0; j < right_idx_stop[i] - right_idx[i]; j++) {
      retp[current++] = new_rp[right_idx[i] + j];
    }
  }
  for(size_t i=0;i<largest_v_size;i++){
      retp[current++] = largest_v_ptr[i];
  }  
  return ret;
}




template <class T>
std::vector<T> set_union_multvec_hierarchical(std::vector< std::vector< T > >& inputVecs) {    //Use vertical merge, but in a hierarchical sequence


    size_t num_inputVecs = inputVecs.size();
    std::vector<size_t> idx_of_inputVecs(num_inputVecs,0);
    auto* idx_of_inputVecs_ptr= &idx_of_inputVecs[0];
    for(size_t i=0; i<num_inputVecs;i++){
        idx_of_inputVecs_ptr[i]= i;
    }
    
    std::vector<size_t> num_elem_inputVecs(num_inputVecs,0);
    auto* num_elem_inputVecs_ptr = &num_elem_inputVecs[0];
    size_t max_num_elem = 0;
    
    for(size_t i = 0;i<num_inputVecs;i++){
        num_elem_inputVecs_ptr[i] = inputVecs[i].size();
        max_num_elem += inputVecs[i].size();
    }   
    size_t idx_mergeIter = 0;
    size_t idx_mergeVec = 0;
    
    size_t num_mergeIters = std::ceil(std::log(num_inputVecs)/std::log(2));
    size_t num_tempMergeVec = std::ceil(std::pow(2, num_mergeIters) - 1); //using ceil for SX. SX sometimes calculate pow() not equal to integer.

    std::vector<std::vector<T> > tempVec_merge(num_tempMergeVec, std::vector<T>(max_num_elem));
    auto* tempVec_merge_ptr = &tempVec_merge[0];

    if(num_inputVecs > 1){
      
        for(size_t i=0; i<num_inputVecs-1; i+=2){//merge all input vectors
            tempVec_merge_ptr[idx_mergeVec++] = set_union_mergepath(inputVecs[idx_of_inputVecs[i]],inputVecs[idx_of_inputVecs[i+1]]); 
        }
        if(num_inputVecs % 2 == 1){//if odd          
            tempVec_merge_ptr[idx_mergeVec++] = inputVecs[idx_of_inputVecs[num_inputVecs-1]];
        }

        size_t num_vecToMerge_current = idx_mergeVec;
        size_t num_vecAfterMerge_current = (num_vecToMerge_current+1)/2;

        size_t idx_mergeStart = 0;
        size_t idx_mergeEnd = idx_mergeStart + num_vecToMerge_current; 
        idx_mergeIter = 1;

        while(num_vecToMerge_current > 1){
            num_vecAfterMerge_current = ((size_t)(num_vecToMerge_current + 1))/2;

            for(size_t i=idx_mergeStart; i<idx_mergeEnd-1; i+=2){//merge all columns
                tempVec_merge_ptr[idx_mergeVec++] = set_union_mergepath(tempVec_merge[i],tempVec_merge[i+1]);    
            }
            if(num_vecToMerge_current % 2 == 1){//if odd
                
                tempVec_merge_ptr[idx_mergeVec++] = tempVec_merge[idx_mergeEnd-1];

            }

            idx_mergeStart =  idx_mergeVec - num_vecAfterMerge_current;
            idx_mergeEnd = idx_mergeStart + num_vecAfterMerge_current;

            num_vecToMerge_current = num_vecAfterMerge_current;
            idx_mergeIter ++;
        }
  
        return tempVec_merge_ptr[idx_mergeVec-1];
    }
    else{

        return inputVecs[0];
    }
}

template< class T> 
std::vector<T> set_union_multivec_seq(std::vector< std::vector<T>>& inputVecs){    //Use vertical merge, but in a serial sequence

    
    size_t num_inputVecs = inputVecs.size();
    auto* inputVecsp = &inputVecs[0];
    
    size_t max_num_elem = 0;
    for(size_t i = 0;i<num_inputVecs;i++){
        max_num_elem += inputVecsp[i].size();
    }
    std::vector<T> ret;
    ret.reserve(max_num_elem);
    ret =inputVecsp[0];
    for(size_t i=1; i<num_inputVecs; i++){//merge all columns     
        ret = set_union_mergepath(ret,inputVecsp[i]); 
    }
    return ret;
}

template <class T>
std::vector<T> set_union_horizontal(std::vector< T >& inputVecs_buff,
        std::vector< size_t >& num_elem_per_inputVecs, 
        size_t vec_idx[], 
        size_t merge_th) {
    //Using parallel merging to merge multiple sorted vectors
    //Optimized for vector processors
 
    size_t num_inputVecs = num_elem_per_inputVecs.size();
    auto* inputVecs_buff_ptr = &inputVecs_buff[0];
    auto* num_elem_per_inputVecs_ptr = &num_elem_per_inputVecs[0];
    size_t max_num_elem = inputVecs_buff.size();
    std::vector<T> ret;
    if(max_num_elem == 0){
        return ret;
    }

    size_t num_mergeIters = 0;
    //Using only two buffers for merging
    std::vector< T > buffer_merge_next(max_num_elem,0);
    std::vector< T > buffer_merge_cur(max_num_elem,0);
    auto* buffer_merge_next_ptr = &buffer_merge_next[0];
    auto* buffer_merge_cur_ptr = &buffer_merge_cur[0];
    
    for(size_t i=0; i<max_num_elem;i++){
        buffer_merge_cur_ptr[i] = inputVecs_buff_ptr[i];
    }
    
    size_t num_vecToMerge = num_inputVecs; 
    size_t num_vecAfterMerge = std::ceil((double)num_vecToMerge/2);
   //TODO: change arrays to vectors
    size_t left_idx[num_vecAfterMerge];
    size_t right_idx[num_vecAfterMerge];
    size_t left_idx_stop[num_vecAfterMerge];
    size_t right_idx_stop[num_vecAfterMerge];
    size_t out_idx[num_vecAfterMerge];
    size_t out_idx_save[num_vecAfterMerge];
    size_t out_idx_save_stop[num_vecAfterMerge];
    size_t vec_idx_stop[num_inputVecs];

    vec_idx_stop[0] = num_elem_per_inputVecs_ptr[0];
    #pragma cdir nodep
    #pragma _NEC ivdep
    for(size_t i=1; i<num_inputVecs-1; i++){
        vec_idx_stop[i] = vec_idx[i+1];
    }    
    vec_idx_stop[num_inputVecs-1] = max_num_elem;
    
    size_t idx_left = 0;
    size_t idx_right = 0;

  
    #pragma cdir nodep
    #pragma _NEC ivdep
    #pragma cdir on_adb(vec_idx)
    #pragma cdir on_adb(vec_idx_stop)    
    #pragma cdir on_adb(left_idx)
    #pragma cdir on_adb(left_idx_stop)
    #pragma cdir on_adb(out_idx)     
    for(size_t i=0; i<num_inputVecs; i+=2){    //set pointers for left vectors
            left_idx[idx_left] = vec_idx[i];
            left_idx_stop[idx_left] = vec_idx_stop[i];
            out_idx[idx_left] = vec_idx[i];
            idx_left++;
       }
    #pragma cdir nodep 
    #pragma _NEC ivdep    
    #pragma cdir on_adb(vec_idx)
    #pragma cdir on_adb(vec_idx_stop)      
    #pragma cdir on_adb(right_idx)
    #pragma cdir on_adb(right_idx_stop)     
    for(size_t i=1; i<num_inputVecs; i+=2){    //set pointers for right vectors     
           right_idx[idx_right] = vec_idx[i];
           right_idx_stop[idx_right] = vec_idx_stop[i];
           idx_right++;            
    }
  
    while(num_vecToMerge > merge_th){ 
        num_mergeIters ++;
        #pragma cdir nodep            
        #pragma _NEC ivdep         
        #pragma cdir on_adb(out_idx) 
        #pragma cdir on_adb(out_idx_save)         
        for(size_t j = 0; j < num_vecAfterMerge; j++) {        
            out_idx_save[j] = out_idx[j];
        }
        
        size_t real_num_vecAfterMerge = num_vecAfterMerge - num_vecToMerge%2; //odd and even number of vectorToMerge 
        
        int isFinished[real_num_vecAfterMerge];
        bool all_finished = true;
        
        #pragma cdir on_adb(isFinished)         
        for(size_t i=0;i<real_num_vecAfterMerge;i++){
                isFinished[i] = false;
        }    
       while(1){   
                #pragma cdir nodep            
                #pragma _NEC ivdep 
                #pragma cdir on_adb(buffer_merge_next_ptr)
                #pragma cdir on_adb(buffer_merge_cur_ptr)
                #pragma cdir on_adb(left_idx)
                #pragma cdir on_adb(left_idx_stop)
                #pragma cdir on_adb(right_idx)
                #pragma cdir on_adb(right_idx_stop) 
                #pragma cdir on_adb(out_idx) 
                #pragma cdir on_adb(isFinished)              
                for(size_t j = 0; j < real_num_vecAfterMerge; j++) {
                    
                        if(left_idx[j] < left_idx_stop[j] && right_idx[j] < right_idx_stop[j]){

                         int eq = buffer_merge_cur_ptr[left_idx[j]]  == buffer_merge_cur_ptr[right_idx[j]];
                         int lt = buffer_merge_cur_ptr[left_idx[j]]  < buffer_merge_cur_ptr[right_idx[j]];


                        if(eq || lt) {
                          buffer_merge_next_ptr[out_idx[j]++] = buffer_merge_cur_ptr[left_idx[j]];
                          left_idx[j]++;
                        } 

                        else{
                          buffer_merge_next_ptr[out_idx[j]++] = buffer_merge_cur_ptr[right_idx[j]];
                          right_idx[j]++;
                        }
                        
                        if(eq) {
                          right_idx[j]++;
                        }

                    }                
                    else{ 
                        isFinished[j] = true;
                    }
                }
                all_finished = true;
                #pragma cdir on_adb(isFinished)
                for(size_t m = 0; m<real_num_vecAfterMerge; m++){
                    if(isFinished[m] == false){
                        all_finished = false;
                        break;
                    }
                }
                if(all_finished == true){
                    break;
                }

            }
      
  size_t num_leftover_left[real_num_vecAfterMerge];
  size_t num_leftover_right[real_num_vecAfterMerge];
                #pragma _NEC ivdep 
                #pragma cdir nodep    
                #pragma cdir on_adb(left_idx)
                #pragma cdir on_adb(left_idx_stop)
                #pragma cdir on_adb(right_idx)
                #pragma cdir on_adb(right_idx_stop)
                #pragma cdir on_adb(num_leftover_left)
                #pragma cdir on_adb(num_leftover_right)
            for(size_t j = 0; j < real_num_vecAfterMerge; j++) {
                num_leftover_left[j] = left_idx_stop[j] - left_idx[j];
                num_leftover_right[j] = right_idx_stop[j] - right_idx[j];
            } 
  
  
            #pragma _NEC ivdep 
            #pragma cdir nodep
            for(size_t j = 0; j < real_num_vecAfterMerge; j++) {             
            #pragma cdir nodep
            #pragma cdir on_adb(buffer_merge_next_ptr)
            #pragma cdir on_adb(buffer_merge_cur_ptr)  
                for(size_t k= 0 ;k<num_leftover_left[j];k++){
                    buffer_merge_next_ptr[out_idx[j] + k] =   buffer_merge_cur_ptr[left_idx[j] + k];              
                }
            }
            #pragma _NEC ivdep 
            #pragma cdir nodep  
            #pragma cdir on_adb(out_idx) 
            #pragma cdir on_adb(num_leftover_left)  
            for(size_t j = 0; j < real_num_vecAfterMerge; j++) {                
                out_idx[j] = out_idx[j] + num_leftover_left[j];
            }
            
            #pragma _NEC ivdep 
            #pragma cdir nodep
            for(size_t j = 0; j < real_num_vecAfterMerge; j++) {
            #pragma cdir nodep
            #pragma cdir on_adb(buffer_merge_next_ptr)
            #pragma cdir on_adb(buffer_merge_cur_ptr)  
                for(size_t k= 0 ;k<num_leftover_right[j];k++){
                    buffer_merge_next_ptr[out_idx[j] + k] =   buffer_merge_cur_ptr[right_idx[j] + k];                              
                }
            }
            #pragma _NEC ivdep 
            #pragma cdir nodep  
            #pragma cdir on_adb(out_idx) 
            #pragma cdir on_adb(num_leftover_right) 
            for(size_t j = 0; j < real_num_vecAfterMerge; j++) {                
                out_idx[j] = out_idx[j] + num_leftover_right[j];
            }
            #pragma _NEC ivdep 
            #pragma cdir nodep 
            #pragma cdir on_adb(out_idx)  
            #pragma cdir on_adb(out_idx_save_stop)   
            for(size_t j = 0; j < real_num_vecAfterMerge; j++) {
                out_idx_save_stop[j] = out_idx[j];
            }   
        for(size_t p=real_num_vecAfterMerge; p<num_vecAfterMerge; p++){//copy the last vector to buffer_next if it is leftover (odd)
                    #pragma _NEC ivdep                     
                    #pragma cdir nodep            
            for(size_t k= 0 ; k<left_idx_stop[p] - left_idx[p]; k++){ //copy the last vector to the buffer_merge_next
                buffer_merge_next_ptr[out_idx[p]+k] = buffer_merge_cur_ptr[left_idx[p] + k];
            }
            out_idx[p] += left_idx_stop[p]- left_idx[p];
            out_idx_save_stop[p] = out_idx[p];
        }

        idx_left = 0;
        idx_right = 0;
                #pragma cdir on_adb(left_idx) 
                #pragma cdir on_adb(left_idx_stop)
                #pragma cdir on_adb(out_idx) 
                #pragma cdir on_adb(out_idx_save)  
                #pragma cdir on_adb(out_idx_save_stop)        
        for(size_t i = 0; i<num_vecAfterMerge;i+=2){
                left_idx[idx_left] = out_idx_save[i];
                left_idx_stop[idx_left] = out_idx_save_stop[i];
                out_idx[idx_left] = out_idx_save[i];
                idx_left ++;
        }
                #pragma cdir on_adb(out_idx_save)  
                #pragma cdir on_adb(out_idx_save_stop) 
                #pragma cdir on_adb(right_idx_stop)        
                #pragma cdir on_adb(right_idx)            
        for(size_t i = 1; i<num_vecAfterMerge;i+=2){            

                right_idx[idx_right] = out_idx_save[i];
                right_idx_stop[idx_right] = out_idx_save_stop[i];
                idx_right++;
        }
   
        num_vecToMerge = num_vecAfterMerge;
        num_vecAfterMerge = std::ceil((double)num_vecToMerge/2); 
        

            #pragma cdir on_adb(buffer_merge_next_ptr) 
            #pragma cdir on_adb(buffer_merge_cur_ptr)   
        for(size_t i = 0;i < max_num_elem; i++){
            buffer_merge_cur_ptr[i] = buffer_merge_next_ptr[i];
        }       
    }
  
    //changing to hierarchical vertial-merge

    if(num_vecToMerge >= 2){
        
        std::vector<std::vector<T> > buffer_merge_seq_vecs(num_vecToMerge);
        for(size_t i=0;i<num_vecToMerge; i++){
            size_t idx_elem = 0;
            buffer_merge_seq_vecs[i].resize(out_idx_save_stop[i] - out_idx_save[i]);
            #pragma cdir nodep            
            #pragma _NEC ivdep
            #pragma cdir on_adb(buffer_merge_cur_ptr) 
            #pragma cdir on_adb(buffer_merge_seq_vecs[i]) 
            for(size_t j=out_idx_save[i];j<out_idx_save_stop[i];j++){
                buffer_merge_seq_vecs[i][idx_elem++] = buffer_merge_cur_ptr[j];
            }
        }
        
        auto buffer_merge_seq = set_union_multvec_hierarchical<T>(buffer_merge_seq_vecs);
//        auto buffer_merge_seq = set_union_multiVec_seq<T>(buffer_merge_seq_vecs);        
        return buffer_merge_seq;
    }    

    size_t num_elem = left_idx_stop[0] - left_idx[0];
    std::vector<T> ret_final(num_elem,0);
    auto* retp = &ret_final[0];
    #pragma cdir on_adb(buffer_merge_cur_ptr)
    #pragma cdir on_adb(retp)    
    for(size_t i=0; i<num_elem; i++){
        retp[i] = buffer_merge_cur_ptr[i];
    }
    return ret_final;
   
}


template <class T>
std::vector<T> set_union_horizontal_compvalid(std::vector< T >& inputVecs_buff,std::vector< size_t >& num_elem_per_inputVecs, size_t vec_idx[], size_t merge_th) {
    //Using parallel merging to merge multiple sorted short vectors
    //Only compare those vectors that haven't finished. It means we have to check every pair if finished frequently.


    size_t num_inputVecs = num_elem_per_inputVecs.size();
    auto* inputVecs_buff_ptr = &inputVecs_buff[0];
    auto* num_elem_per_inputVecs_ptr = &num_elem_per_inputVecs[0];
    size_t max_num_elem = 0;
    for(size_t i = 0;i<num_inputVecs;i++){
        max_num_elem += num_elem_per_inputVecs_ptr[i];
    }

    size_t num_mergeIters = 0;
    //Using only two buffers for merging
    std::vector< T > buffer_merge_next(max_num_elem);
    std::vector< T > buffer_merge_cur(max_num_elem);
    auto* buffer_merge_next_ptr = &buffer_merge_next[0];
    auto* buffer_merge_cur_ptr = &buffer_merge_cur[0];
    
    for(size_t i=0; i<max_num_elem;i++){
        buffer_merge_cur_ptr[i] = inputVecs_buff_ptr[i];
    }

    size_t num_vecToMerge = num_inputVecs; 
    size_t num_vecAfterMerge = std::ceil((double)num_vecToMerge/2);
   
    size_t left_idx[num_vecAfterMerge];
    size_t right_idx[num_vecAfterMerge];
    size_t left_idx_stop[num_vecAfterMerge];
    size_t right_idx_stop[num_vecAfterMerge];
    size_t out_idx[num_vecAfterMerge];
    size_t out_idx_save[num_vecAfterMerge];
    size_t out_idx_save_stop[num_vecAfterMerge];
    size_t vec_idx_stop[num_inputVecs];

    vec_idx_stop[0] = num_elem_per_inputVecs_ptr[0];
            #pragma cdir nodep
            #pragma _NEC ivdep
    for(size_t i=1; i<num_inputVecs-1; i++){
        vec_idx_stop[i] = vec_idx[i+1];
    }    
    vec_idx_stop[num_inputVecs-1] = max_num_elem;
    
    size_t idx_left = 0;
    size_t idx_right = 0;

  
    #pragma cdir nodep
    #pragma _NEC ivdep
    #pragma cdir on_adb(vec_idx)
    #pragma cdir on_adb(vec_idx_stop)    
    #pragma cdir on_adb(left_idx)
    #pragma cdir on_adb(left_idx_stop)
    #pragma cdir on_adb(out_idx)     
    for(size_t i=0; i<num_inputVecs; i+=2){    //set pointers for left vectors
            left_idx[idx_left] = vec_idx[i];
            left_idx_stop[idx_left] = vec_idx_stop[i];
            out_idx[idx_left] = vec_idx[i];
            idx_left++;
       }
    
    #pragma cdir on_adb(vec_idx)
    #pragma cdir on_adb(vec_idx_stop)      
    #pragma cdir on_adb(right_idx)
    #pragma cdir on_adb(right_idx_stop)     
    for(size_t i=1; i<num_inputVecs; i+=2){    //set pointers for right vectors     
           right_idx[idx_right] = vec_idx[i];
           right_idx_stop[idx_right] = vec_idx_stop[i];
           idx_right++;            
    }
  
    while(num_vecToMerge > merge_th){ 
        num_mergeIters ++;
        for(size_t j = 0; j < num_vecAfterMerge; j++) {        
            out_idx_save[j] = out_idx[j];
        }
    size_t real_num_vecAfterMerge = num_vecAfterMerge - num_vecToMerge%2; //odd and even number of vectorToMerge 
        
    if(real_num_vecAfterMerge > GRAPH_SET_VLEN_EACH3){
    size_t valid_vec_idx[real_num_vecAfterMerge];

  
        while(1){   

             size_t valid_num_vec = 0;
                #pragma cdir on_adb(valid_vec_idx)
                #pragma cdir on_adb(left_idx)
                #pragma cdir on_adb(left_idx_stop)
                #pragma cdir on_adb(right_idx)
                #pragma cdir on_adb(right_idx_stop) 
             for(size_t j = 0; j < real_num_vecAfterMerge; j++) {//Check which vectors are not finished yet and only compare those vectors.
                   if(left_idx[j] < left_idx_stop[j] && right_idx[j] < right_idx_stop[j]){
                       valid_vec_idx[valid_num_vec++] = j;
                   }
             }
             if(valid_num_vec == 0){
                 break;
             }           
           
                #pragma cdir nodep
                #pragma _NEC ivdep
                #pragma cdir on_adb(buffer_merge_next_ptr)
                #pragma cdir on_adb(buffer_merge_cur_ptr)
                #pragma cdir on_adb(left_idx)
                #pragma cdir on_adb(left_idx_stop)
                #pragma cdir on_adb(right_idx)
                #pragma cdir on_adb(right_idx_stop) 
                #pragma cdir on_adb(out_idx)                
                #pragma cdir on_adb(valid_vec_idx)
                for(size_t j = 0; j < valid_num_vec; j++) {
                    
                    bool eq = buffer_merge_cur_ptr[left_idx[valid_vec_idx[j]]]  == buffer_merge_cur_ptr[right_idx[valid_vec_idx[j]]];
                    bool lt = buffer_merge_cur_ptr[left_idx[valid_vec_idx[j]]]  < buffer_merge_cur_ptr[right_idx[valid_vec_idx[j]]];


                   if(eq || lt) {
                     buffer_merge_next_ptr[out_idx[valid_vec_idx[j]]++] = buffer_merge_cur_ptr[left_idx[valid_vec_idx[j]]];
                     left_idx[valid_vec_idx[j]]++;
                   } 

                   else{
                     buffer_merge_next_ptr[out_idx[valid_vec_idx[j]]++] = buffer_merge_cur_ptr[right_idx[valid_vec_idx[j]]];
                     right_idx[valid_vec_idx[j]]++;
                   }

                   if(eq) {
                     right_idx[valid_vec_idx[j]]++;
                   }
                }                
            }
    }
  else{
  
        int isFinished[real_num_vecAfterMerge];
        bool all_finished = true;
        for(size_t i=0;i<real_num_vecAfterMerge;i++){
            isFinished[i] = false;
        }
        
        while(1){   
            
                #pragma cdir nodep
                #pragma _NEC ivdep
                #pragma cdir on_adb(buffer_merge_next_ptr)
                #pragma cdir on_adb(buffer_merge_cur_ptr)
                #pragma cdir on_adb(left_idx)
                #pragma cdir on_adb(left_idx_stop)
                #pragma cdir on_adb(right_idx)
                #pragma cdir on_adb(right_idx_stop) 
                #pragma cdir on_adb(out_idx)                
                #pragma cdir on_adb(valid_vec_idx)
                for(size_t j = 0; j < real_num_vecAfterMerge; j++) {

                        if(!(left_idx[j] == left_idx_stop[j] || right_idx[j] == right_idx_stop[j])){

                            bool eq = buffer_merge_cur_ptr[left_idx[j]]  == buffer_merge_cur_ptr[right_idx[j]];
                            bool lt = buffer_merge_cur_ptr[left_idx[j]]  < buffer_merge_cur_ptr[right_idx[j]];


                           if(eq || lt) {
                             buffer_merge_next_ptr[out_idx[j]++] = buffer_merge_cur_ptr[left_idx[j]];
                             left_idx[j]++;
                           } 

                           else{
                             buffer_merge_next_ptr[out_idx[j]++] = buffer_merge_cur_ptr[right_idx[j]];
                             right_idx[j]++;
                           }

                           if(eq) {
                             right_idx[j]++;
                           }

                    }                
                    else{ 
                        isFinished[j] = true;
                    }
                }
                all_finished = true;
                #pragma cdir on_adb(isFinished)
                for(size_t m = 0; m<real_num_vecAfterMerge; m++){
                    if(isFinished[m] == false){
                        all_finished = false;
                        break;
                    }
                }
                if(all_finished == true){
                    break;
                }

            }  
  
  } 
  size_t num_leftover_left[real_num_vecAfterMerge];
  size_t num_leftover_right[real_num_vecAfterMerge];
  
                #pragma cdir on_adb(left_idx)
                #pragma cdir on_adb(left_idx_stop)
                #pragma cdir on_adb(right_idx)
                #pragma cdir on_adb(right_idx_stop)
                #pragma cdir on_adb(num_leftover_left)
                #pragma cdir on_adb(num_leftover_right)  
            for(size_t j = 0; j < real_num_vecAfterMerge; j++) {
                num_leftover_left[j] = left_idx_stop[j] - left_idx[j];
                num_leftover_right[j] = right_idx_stop[j] - right_idx[j];
            } 
  
  
            #pragma cdir nodep
            #pragma _NEC ivdep
            for(size_t j = 0; j < real_num_vecAfterMerge; j++) {             
            #pragma cdir nodep
            #pragma _NEC ivdep
            #pragma cdir on_adb(buffer_merge_next_ptr)
            #pragma cdir on_adb(buffer_merge_cur_ptr)  
                for(size_t k= 0 ;k<num_leftover_left[j];k++){
                    buffer_merge_next_ptr[out_idx[j] + k] =   buffer_merge_cur_ptr[left_idx[j] + k];              
                }
            }
            #pragma cdir on_adb(out_idx) 
            #pragma cdir on_adb(num_leftover_left)  
            for(size_t j = 0; j < real_num_vecAfterMerge; j++) {                
                out_idx[j] = out_idx[j] + num_leftover_left[j];
            }
            
            #pragma cdir nodep
            for(size_t j = 0; j < real_num_vecAfterMerge; j++) {
            #pragma cdir nodep
            #pragma cdir on_adb(buffer_merge_next_ptr)
            #pragma cdir on_adb(buffer_merge_cur_ptr)  
                for(size_t k= 0 ;k<num_leftover_right[j];k++){
                    buffer_merge_next_ptr[out_idx[j] + k] =   buffer_merge_cur_ptr[right_idx[j] + k];                              
                }
            }
                 #pragma cdir on_adb(out_idx) 
               #pragma cdir on_adb(num_leftover_right) 
            for(size_t j = 0; j < real_num_vecAfterMerge; j++) {                
                out_idx[j] = out_idx[j] + num_leftover_right[j];
            }
  
               #pragma cdir on_adb(out_idx)  
                #pragma cdir on_adb(out_idx_save_stop)   
            for(size_t j = 0; j < real_num_vecAfterMerge; j++) {
                out_idx_save_stop[j] = out_idx[j];
            }  
        for(size_t p=real_num_vecAfterMerge; p<num_vecAfterMerge; p++){//copy the last vector to buffer_next if it is leftover (odd)
            
            for(size_t k=left_idx[p]; k<left_idx_stop[p]; k++){ //copy the last vector to the buffer_merge_next
                buffer_merge_next_ptr[out_idx[p]++] = buffer_merge_cur_ptr[k];
            }
            out_idx_save_stop[p] = out_idx[p];
        }

        idx_left = 0;
        idx_right = 0;
                #pragma cdir on_adb(left_idx) 
                #pragma cdir on_adb(left_idx_stop)
                #pragma cdir on_adb(out_idx) 
                #pragma cdir on_adb(out_idx_save)  
                #pragma cdir on_adb(out_idx_save_stop)        
        for(size_t i = 0; i<num_vecAfterMerge;i+=2){
                left_idx[idx_left] = out_idx_save[i];
                left_idx_stop[idx_left] = out_idx_save_stop[i];
                out_idx[idx_left] = out_idx_save[i];
                idx_left ++;
        }
                #pragma cdir on_adb(out_idx_save)  
                #pragma cdir on_adb(out_idx_save_stop) 
                #pragma cdir on_adb(right_idx_stop)        
                #pragma cdir on_adb(right_idx)            
        for(size_t i = 1; i<num_vecAfterMerge;i+=2){            
                right_idx[idx_right] = out_idx_save[i];
                right_idx_stop[idx_right] = out_idx_save_stop[i];
        }
    
        num_vecToMerge = num_vecAfterMerge;
        num_vecAfterMerge = std::ceil((double)num_vecToMerge/2); 
       
        #pragma cdir on_adb(buffer_merge_next_ptr) 
        #pragma cdir on_adb(buffer_merge_cur_ptr)   
        for(size_t i = 0;i < max_num_elem; i++){
            buffer_merge_cur_ptr[i] = buffer_merge_next_ptr[i];
        }     
    }
  
    if(num_vecToMerge >= 2){
        std::vector<std::vector<T> > buffer_merge_seq_vecs(num_vecToMerge);
        for(size_t i=0;i<num_vecToMerge; i++){
            size_t idx_elem = 0;
            buffer_merge_seq_vecs[i].resize(out_idx_save_stop[i] - out_idx_save[i]);
            #pragma cdir nodep
            #pragma cdir on_adb(buffer_merge_cur_ptr) 
            #pragma cdir on_adb(buffer_merge_seq_vecs[i]) 
            for(size_t j=out_idx_save[i];j<out_idx_save_stop[i];j++){
                buffer_merge_seq_vecs[i][idx_elem++] = buffer_merge_cur_ptr[j];
            }
        }
        
        auto buffer_merge_seq = set_union_multvec_hierarchical<T>(buffer_merge_seq_vecs);       
        return buffer_merge_seq;
    }    

    size_t num_elem = left_idx_stop[0] - left_idx[0];
    std::vector<T> ret(num_elem);
    auto* retp = &ret[0];
    #pragma cdir on_adb(buffer_merge_cur_ptr)
    #pragma cdir on_adb(retp)    
    for(size_t i=0; i<num_elem; i++){
        retp[i] = buffer_merge_cur_ptr[i];
    }
    return ret;
   
}


template <class T>
std::vector<T> set_union_horizontal_overlap(std::vector< T >& inputVecs_buff,std::vector< size_t >& num_elem_per_inputVecs, size_t vec_idx[], size_t merge_th) {
    //Only compare overlap part
 
    size_t num_inputVecs = num_elem_per_inputVecs.size();
    auto* inputVecs_buff_ptr = &inputVecs_buff[0];
    auto* num_elem_per_inputVecs_ptr = &num_elem_per_inputVecs[0];
    size_t max_num_elem = 0;
    for(size_t i = 0;i<num_inputVecs;i++){
        max_num_elem += num_elem_per_inputVecs_ptr[i];
    }

    size_t num_mergeIters = 0;
    //Using only two buffers for merging
    std::vector< T > buffer_merge_next(max_num_elem);
    std::vector< T > buffer_merge_cur(max_num_elem);
    auto* buffer_merge_next_ptr = &buffer_merge_next[0];
    auto* buffer_merge_cur_ptr = &buffer_merge_cur[0];
    
    for(size_t i=0; i<max_num_elem;i++){
        buffer_merge_cur_ptr[i] = inputVecs_buff_ptr[i];
    }

    size_t num_vecToMerge = num_inputVecs; 
    size_t num_vecAfterMerge = std::ceil((double)num_vecToMerge/2);
   
    size_t left_idx[num_vecAfterMerge];
    size_t right_idx[num_vecAfterMerge];
    size_t new_left_idx[num_vecAfterMerge];
    size_t new_right_idx[num_vecAfterMerge];     
    size_t left_idx_stop[num_vecAfterMerge];
    size_t right_idx_stop[num_vecAfterMerge];
    size_t out_idx[num_vecAfterMerge];
    size_t out_idx_save[num_vecAfterMerge];
    size_t out_idx_save_stop[num_vecAfterMerge];
    size_t vec_idx_stop[num_inputVecs];

    vec_idx_stop[0] = num_elem_per_inputVecs_ptr[0];
    #pragma cdir nodep
    for(size_t i=1; i<num_inputVecs-1; i++){
        vec_idx_stop[i] = vec_idx[i+1];
    }    
    vec_idx_stop[num_inputVecs-1] = max_num_elem;
    
    size_t idx_left = 0;
    size_t idx_right = 0;

  
    #pragma cdir nodep
    #pragma cdir on_adb(vec_idx)
    #pragma cdir on_adb(vec_idx_stop)    
    #pragma cdir on_adb(left_idx)
    #pragma cdir on_adb(left_idx_stop)
    #pragma cdir on_adb(out_idx)     
    for(size_t i=0; i<num_inputVecs; i+=2){    //set pointers for left vectors
            left_idx[idx_left] = vec_idx[i];
            left_idx_stop[idx_left] = vec_idx_stop[i];
            out_idx[idx_left] = vec_idx[i];
            idx_left++;
       }
    
    #pragma cdir on_adb(vec_idx)
    #pragma cdir on_adb(vec_idx_stop)      
    #pragma cdir on_adb(right_idx)
    #pragma cdir on_adb(right_idx_stop)     
    for(size_t i=1; i<num_inputVecs; i+=2){    //set pointers for right vectors     
           right_idx[idx_right] = vec_idx[i];
           right_idx_stop[idx_right] = vec_idx_stop[i];
           idx_right++;            
    }
 
    while(num_vecToMerge > merge_th){ 
        num_mergeIters ++;
        for(size_t j = 0; j < num_vecAfterMerge; j++) {        
            out_idx_save[j] = out_idx[j];
        }
        size_t real_num_vecAfterMerge = num_vecAfterMerge - num_vecToMerge%2; //odd and even number of vectorToMerge 
        if(num_vecToMerge < GRAPH_SET_VLEN_EACH3){

        lower_bound_HMerge(buffer_merge_cur,real_num_vecAfterMerge, left_idx, left_idx_stop, right_idx, right_idx_stop,new_left_idx,new_right_idx );
        size_t num_smallest_left[real_num_vecAfterMerge];
        size_t num_smallest_right[real_num_vecAfterMerge];
    
                #pragma cdir on_adb(left_idx)
                #pragma cdir on_adb(new_left_idx)
                #pragma cdir on_adb(right_idx)
                #pragma cdir on_adb(new_right_idx)
                #pragma cdir on_adb(num_smallest_left)
                #pragma cdir on_adb(num_smallest_right)
            for(size_t j = 0; j < real_num_vecAfterMerge; j++) {
                num_smallest_left[j] = new_left_idx[j] - left_idx[j];
                num_smallest_right[j] = new_right_idx[j] - right_idx[j];
            } 
  
  
            #pragma cdir nodep
            for(size_t j = 0; j < real_num_vecAfterMerge; j++) {             
            #pragma cdir nodep
            #pragma cdir on_adb(buffer_merge_next_ptr)
            #pragma cdir on_adb(buffer_merge_cur_ptr)  
                for(size_t k= 0 ;k<num_smallest_left[j];k++){
                    buffer_merge_next_ptr[out_idx[j] + k] =   buffer_merge_cur_ptr[left_idx[j] + k];              
                }
            }
            #pragma cdir on_adb(out_idx) 
            #pragma cdir on_adb(num_leftover_left)  
            for(size_t j = 0; j < real_num_vecAfterMerge; j++) {                
                out_idx[j] = out_idx[j] + num_smallest_left[j];
            }
            
            #pragma cdir nodep
            for(size_t j = 0; j < real_num_vecAfterMerge; j++) {
            #pragma cdir nodep
            #pragma cdir on_adb(buffer_merge_next_ptr)
            #pragma cdir on_adb(buffer_merge_cur_ptr)  
                for(size_t k= 0 ;k<num_smallest_right[j];k++){
                    buffer_merge_next_ptr[out_idx[j] + k] =   buffer_merge_cur_ptr[right_idx[j] + k];                              
                }
            }
                 #pragma cdir on_adb(out_idx) 
               #pragma cdir on_adb(num_leftover_right) 
            for(size_t j = 0; j < real_num_vecAfterMerge; j++) {                
                out_idx[j] = out_idx[j] + num_smallest_right[j];
            }      
        }
        else{
                #pragma cdir on_adb(left_idx)
                #pragma cdir on_adb(new_left_idx)
                #pragma cdir on_adb(right_idx)
                #pragma cdir on_adb(new_right_idx)
            for(size_t j = 0; j < real_num_vecAfterMerge; j++) {
                new_left_idx[j] = left_idx[j];
                new_right_idx[j] = right_idx[j];
           
            } 
        }

        
        int isFinished[real_num_vecAfterMerge];
        bool all_finished = true;
        for(size_t i=0;i<real_num_vecAfterMerge;i++){
            isFinished[i] = false;
        }
        while(1){   
      
                #pragma cdir nodep
                #pragma cdir on_adb(buffer_merge_next_ptr)
                #pragma cdir on_adb(buffer_merge_cur_ptr)
                #pragma cdir on_adb(new_left_idx)
                #pragma cdir on_adb(left_idx_stop)
                #pragma cdir on_adb(new_right_idx)
                #pragma cdir on_adb(right_idx_stop) 
                #pragma cdir on_adb(out_idx)                
                #pragma cdir on_adb(valid_vec_idx)
                #pragma cdir on_adb(isFinished)
                for(size_t j = 0; j < real_num_vecAfterMerge; j++) { 
                        if(new_left_idx[j] < left_idx_stop[j] && new_right_idx[j] < right_idx_stop[j]){
                            
                            bool eq = buffer_merge_cur_ptr[new_left_idx[j]]  == buffer_merge_cur_ptr[new_right_idx[j]];
                            bool lt = buffer_merge_cur_ptr[new_left_idx[j]]  < buffer_merge_cur_ptr[new_right_idx[j]];
                            if(eq || lt) {
                              buffer_merge_next_ptr[out_idx[j]++] = buffer_merge_cur_ptr[new_left_idx[j]];
                              new_left_idx[j]++;
                            } 

                            else{
                              buffer_merge_next_ptr[out_idx[j]++] = buffer_merge_cur_ptr[new_right_idx[j]];
                              new_right_idx[j]++;
                            }

                            if(eq) {
                              new_right_idx[j]++;
                            }

                    }                
                    else{ 
                        isFinished[j] = true;
                    }
                }
                all_finished = true;
                #pragma cdir on_adb(isFinished)
                for(size_t m = 0; m<real_num_vecAfterMerge; m++){
                    if(isFinished[m] == false){
                        all_finished = false;
                        break;
                    }
                }
                if(all_finished == true){
                    break;
                }

            }
    
            #pragma cdir nodep
            for(size_t j = 0; j < real_num_vecAfterMerge; j++) {             
            #pragma cdir nodep
            #pragma cdir on_adb(buffer_merge_next_ptr)
            #pragma cdir on_adb(buffer_merge_cur_ptr) 
            #pragma cdir on_adb(out_idx)       
            #pragma cdir on_adb(new_left_idx)                 
                for(size_t k= 0 ;k<left_idx_stop[j] - new_left_idx[j] ;k++){
                    buffer_merge_next_ptr[out_idx[j] + k] = buffer_merge_cur_ptr[new_left_idx[j] + k];              
                }
                out_idx[j] = out_idx[j] + left_idx_stop[j] - new_left_idx[j];
            }

            
            #pragma cdir nodep
            for(size_t j = 0; j < real_num_vecAfterMerge; j++) {
            #pragma cdir nodep
            #pragma cdir on_adb(buffer_merge_next_ptr)
            #pragma cdir on_adb(buffer_merge_cur_ptr)
            #pragma cdir on_adb(out_idx)       
            #pragma cdir on_adb(new_right_idx)        
                for(size_t k= 0 ;k<right_idx_stop[j] - new_right_idx[j];k++){
                    buffer_merge_next_ptr[out_idx[j] + k] =   buffer_merge_cur_ptr[new_right_idx[j] + k];                              
                }
                out_idx[j] = out_idx[j] + right_idx_stop[j] - new_right_idx[j];
            }

  
               #pragma cdir on_adb(out_idx)  
                #pragma cdir on_adb(out_idx_save_stop_p)   
            for(size_t j = 0; j < real_num_vecAfterMerge; j++) {
                out_idx_save_stop[j] = out_idx[j];
            }
        for(size_t p=real_num_vecAfterMerge; p<num_vecAfterMerge; p++){//copy the last vector to buffer_next if it is leftover (odd)
            
            for(size_t k=left_idx[p]; k<left_idx_stop[p]; k++){ //copy the last vector to the buffer_merge_next
                buffer_merge_next_ptr[out_idx[p]++] = buffer_merge_cur_ptr[k];
            }
            out_idx_save_stop[p] = out_idx[p];
        }

        idx_left = 0;
        idx_right = 0;
                #pragma cdir on_adb(left_idx) 
                #pragma cdir on_adb(left_idx_stop)
                #pragma cdir on_adb(out_idx) 
                #pragma cdir on_adb(out_idx_save_p)  
                #pragma cdir on_adb(out_idx_save_stop_p)        
        for(size_t i = 0; i<num_vecAfterMerge;i+=2){
                left_idx[idx_left] = out_idx_save[i];
                left_idx_stop[idx_left] = out_idx_save_stop[i];
                out_idx[idx_left] = out_idx_save[i];
                idx_left ++;
        }
                #pragma cdir on_adb(out_idx_save_p)  
                #pragma cdir on_adb(out_idx_save_stop_p) 
                #pragma cdir on_adb(right_idx_stop)        
                #pragma cdir on_adb(right_idx)            
        for(size_t i = 1; i<num_vecAfterMerge;i+=2){            

                right_idx[idx_right] = out_idx_save[i];
                right_idx_stop[idx_right] = out_idx_save_stop[i];
                idx_right++;
        }
 
        num_vecToMerge = num_vecAfterMerge;
        num_vecAfterMerge = std::ceil((double)num_vecToMerge/2); 

        #pragma cdir on_adb(buffer_merge_next_ptr) 
        #pragma cdir on_adb(buffer_merge_cur_ptr)   
        for(size_t i = 0;i < max_num_elem; i++){
            buffer_merge_cur_ptr[i] = buffer_merge_next_ptr[i];
        }        
    }
    //changing to sequential but hierarchical merging
    if(num_vecToMerge >= 2){
        std::vector<std::vector<T> > buffer_merge_seq_vecs(num_vecToMerge);
        for(size_t i=0;i<num_vecToMerge; i++){
            size_t idx_elem = 0;
            buffer_merge_seq_vecs[i].resize(out_idx_save_stop[i] - out_idx_save[i]);
            #pragma cdir nodep
            #pragma cdir on_adb(buffer_merge_cur_ptr) 
            #pragma cdir on_adb(buffer_merge_seq_vecs[i]) 
            for(size_t j=out_idx_save[i];j<out_idx_save_stop[i];j++){
                buffer_merge_seq_vecs[i][idx_elem++] = buffer_merge_cur_ptr[j];
            }
        }
        
        auto buffer_merge_seq = set_union_multvec_hierarchical<T>(buffer_merge_seq_vecs);       
        return buffer_merge_seq;
    }    

    size_t num_elem = left_idx_stop[0] - left_idx[0];
    std::vector<T> ret(num_elem);
    auto* retp = &ret[0];
    #pragma cdir on_adb(buffer_merge_cur_ptr)
    #pragma cdir on_adb(retp)    
    for(size_t i=0; i<num_elem; i++){
        retp[i] = buffer_merge_cur_ptr[i];
    }
    return ret;
   
}


template <class T>
sparse_vector<T> set_union_2d(sparse_vector< T >& inputVecs_buff,
        std::vector< size_t >& num_elem_per_inputVecs, 
        std::vector<size_t>& vec_idx, 
        size_t merge_th) {
    //Generalized version, input/output version are sparse vectors
    auto* vec_idx_ptr = &vec_idx[0];
    size_t num_inputVecs = num_elem_per_inputVecs.size();
    
    auto* inputVecs_buff_idx_ptr = &inputVecs_buff.idx[0];
    auto* inputVecs_buff_val_ptr = &inputVecs_buff.val[0];
    
    auto* num_elem_per_inputVecs_ptr = &num_elem_per_inputVecs[0];
    size_t max_num_elem = 0;

    max_num_elem = inputVecs_buff.idx.size();
    sparse_vector<T> ret;
    if(max_num_elem == 0){
        return ret;
    }
    size_t num_mergeIters = 0;

    //Using only two buffers for merging
    sparse_vector< T > buffer_merge_next(max_num_elem,0);
    sparse_vector< T > buffer_merge_cur(max_num_elem,0);
    sparse_vector< T > buffer_merge_temp(max_num_elem,0);
    
    auto* buffer_merge_next_idx_ptr = &buffer_merge_next.idx[0];
    auto* buffer_merge_cur_idx_ptr = &buffer_merge_cur.idx[0];
    auto* buffer_merge_temp_idx_ptr = &buffer_merge_temp.idx[0];
    
    auto* buffer_merge_next_val_ptr = &buffer_merge_next.val[0];
    auto* buffer_merge_cur_val_ptr = &buffer_merge_cur.val[0];
    auto* buffer_merge_temp_val_ptr = &buffer_merge_temp.val[0];
    
    for(size_t i=0; i<max_num_elem;i++){
        buffer_merge_cur_idx_ptr[i] = inputVecs_buff_idx_ptr[i];
        buffer_merge_cur_val_ptr[i] = inputVecs_buff_val_ptr[i];
    }
       
    size_t num_vecToMerge = num_inputVecs; 
    size_t num_vecAfterMerge = std::ceil((double)num_vecToMerge/2);
   
    std::vector<size_t> left_idx(num_vecAfterMerge,0);
    std::vector<size_t> right_idx(num_vecAfterMerge,0);    
    std::vector<size_t> left_idx_stop(num_vecAfterMerge,0);
    std::vector<size_t> right_idx_stop(num_vecAfterMerge,0);
    std::vector<size_t> out_idx(num_vecAfterMerge,0);
    std::vector<size_t> out_idx_save(num_vecAfterMerge,0);
    std::vector<size_t> out_idx_save_stop(num_vecAfterMerge,0);
    std::vector<size_t> vec_idx_stop(num_inputVecs,0);
    std::vector<int> isFinished(num_vecAfterMerge,0);

    vec_idx_stop[0] = num_elem_per_inputVecs_ptr[0];
            #pragma _NEC ivdep 
            #pragma cdir nodep
    for(size_t i=1; i<num_inputVecs-1; i++){
        vec_idx_stop[i] = vec_idx_ptr[i+1];
    }    
    vec_idx_stop[num_inputVecs-1] = max_num_elem;
    
    size_t idx_left = 0;
    size_t idx_right = 0;

  
    #pragma _NEC ivdep 
    #pragma cdir nodep
    #pragma cdir on_adb(vec_idx_ptr)
    #pragma cdir on_adb(vec_idx_stop)    
    #pragma cdir on_adb(left_idx)
    #pragma cdir on_adb(left_idx_stop)
    #pragma cdir on_adb(out_idx)     
    for(size_t i=0; i<num_inputVecs; i+=2){    //set pointers for left vectors
            left_idx[idx_left] = vec_idx_ptr[i];
            left_idx_stop[idx_left] = vec_idx_stop[i];
            out_idx[idx_left] = vec_idx_ptr[i];
            idx_left++;
    }
    #pragma _NEC ivdep 
    #pragma cdir nodep    
    #pragma cdir on_adb(vec_idx_ptr)
    #pragma cdir on_adb(vec_idx_stop)      
    #pragma cdir on_adb(right_idx)
    #pragma cdir on_adb(right_idx_stop)     
    for(size_t i=1; i<num_inputVecs; i+=2){    //set pointers for right vectors     
           right_idx[idx_right] = vec_idx_ptr[i];
           right_idx_stop[idx_right] = vec_idx_stop[i];
           idx_right++;            
    }
  
    while(num_vecToMerge > 1){ 

        num_mergeIters ++;
        #pragma _NEC ivdep 
        #pragma cdir nodep        
        #pragma cdir on_adb(out_idx) 
        #pragma cdir on_adb(out_idx_save) 
        for(size_t j = 0; j < num_vecAfterMerge; j++) {        
            out_idx_save[j] = out_idx[j];
        }
        
        size_t real_num_vecAfterMerge = num_vecAfterMerge - num_vecToMerge%2; //odd and even number of vectorToMerge 
        size_t upper_bound = merge_th;        
        size_t num_par_each = frovedis::ceil_div((size_t)upper_bound,real_num_vecAfterMerge);
        size_t num_par_total = num_par_each * real_num_vecAfterMerge;

        
        if(num_par_each == 1 || max_num_elem < upper_bound){ // No need to partition each vector pair
        bool all_finished = true;
        
        #pragma cdir on_adb(isFinished)         
        for(size_t i=0;i<real_num_vecAfterMerge;i++){
                isFinished[i] = false;
        }    

       while(1){   

                #pragma cdir nodep            
                #pragma _NEC ivdep 
                #pragma cdir on_adb(buffer_merge_next_idx_ptr)
                #pragma cdir on_adb(buffer_merge_cur_idx_ptr)
                #pragma cdir on_adb(buffer_merge_next_val_ptr)
                #pragma cdir on_adb(buffer_merge_cur_val_ptr)
                #pragma cdir on_adb(left_idx)
                #pragma cdir on_adb(left_idx_stop)
                #pragma cdir on_adb(right_idx)
                #pragma cdir on_adb(right_idx_stop) 
                #pragma cdir on_adb(out_idx) 
                #pragma cdir on_adb(isFinished)              
                for(size_t j = 0; j < real_num_vecAfterMerge; j++) {

                        if(left_idx[j] < left_idx_stop[j] && right_idx[j] < right_idx_stop[j]){

                         int eq = buffer_merge_cur_idx_ptr[left_idx[j]]  == buffer_merge_cur_idx_ptr[right_idx[j]];
                         int lt = buffer_merge_cur_idx_ptr[left_idx[j]]  < buffer_merge_cur_idx_ptr[right_idx[j]];


                        if(eq) {
                            buffer_merge_next_idx_ptr[out_idx[j]] = buffer_merge_cur_idx_ptr[left_idx[j]];
                            buffer_merge_next_val_ptr[out_idx[j]] = buffer_merge_cur_val_ptr[left_idx[j]]
                                    + buffer_merge_cur_val_ptr[right_idx[j]];
                          out_idx[j]++;
                          left_idx[j]++;
                          right_idx[j]++;
                        } 
                        else if(lt) {
                            buffer_merge_next_idx_ptr[out_idx[j]] = buffer_merge_cur_idx_ptr[left_idx[j]];
                            buffer_merge_next_val_ptr[out_idx[j]] = buffer_merge_cur_val_ptr[left_idx[j]];
                          out_idx[j]++;
                          left_idx[j]++;
                        } 
                        else{
                          buffer_merge_next_idx_ptr[out_idx[j]] = buffer_merge_cur_idx_ptr[right_idx[j]];
                          buffer_merge_next_val_ptr[out_idx[j]] = buffer_merge_cur_val_ptr[right_idx[j]];                          
                          out_idx[j]++;
                          right_idx[j]++;
                        }                       
                    }                
                    else{ 
                        isFinished[j] = true;
                    }
                }
                all_finished = true;
                #pragma cdir on_adb(isFinished)
                for(size_t m = 0; m<real_num_vecAfterMerge; m++){
                    if(isFinished[m] == false){
                        all_finished = false;
                        break;
                    }
                }
                if(all_finished == true){
                    break;
                }

            }
  
  size_t num_leftover_left[real_num_vecAfterMerge];
  size_t num_leftover_right[real_num_vecAfterMerge];
                #pragma _NEC ivdep 
                #pragma cdir nodep    
                #pragma cdir on_adb(left_idx)
                #pragma cdir on_adb(left_idx_stop)
                #pragma cdir on_adb(right_idx)
                #pragma cdir on_adb(right_idx_stop)
                #pragma cdir on_adb(num_leftover_left)
                #pragma cdir on_adb(num_leftover_right)
            for(size_t j = 0; j < real_num_vecAfterMerge; j++) {
                num_leftover_left[j] = left_idx_stop[j] - left_idx[j];
                num_leftover_right[j] = right_idx_stop[j] - right_idx[j];
            } 
  
  
            #pragma _NEC ivdep 
            #pragma cdir nodep
            for(size_t j = 0; j < real_num_vecAfterMerge; j++) {             
            #pragma cdir nodep
            #pragma cdir on_adb(buffer_merge_next_idx_ptr)
            #pragma cdir on_adb(buffer_merge_cur_idx_ptr)  
                for(size_t k= 0 ;k<num_leftover_left[j];k++){
                    buffer_merge_next_idx_ptr[out_idx[j] + k] =   buffer_merge_cur_idx_ptr[left_idx[j] + k];  
                    buffer_merge_next_val_ptr[out_idx[j] + k] =   buffer_merge_cur_val_ptr[left_idx[j] + k];
                }

            #pragma cdir nodep
            #pragma cdir on_adb(buffer_merge_next_idx_ptr)
            #pragma cdir on_adb(buffer_merge_cur_idx_ptr)  
                for(size_t k= 0 ;k<num_leftover_right[j];k++){
                    buffer_merge_next_idx_ptr[out_idx[j] + k] =   buffer_merge_cur_idx_ptr[right_idx[j] + k];    
                    buffer_merge_next_val_ptr[out_idx[j] + k] =   buffer_merge_cur_val_ptr[right_idx[j] + k];  
                }
            }
            #pragma _NEC ivdep 
            #pragma cdir nodep  
            #pragma cdir on_adb(out_idx) 
            #pragma cdir on_adb(num_leftover_left)  
            for(size_t j = 0; j < real_num_vecAfterMerge; j++) {                
                out_idx[j] = out_idx[j] + num_leftover_left[j];
            }  
            #pragma _NEC ivdep 
            #pragma cdir nodep  
            #pragma cdir on_adb(out_idx) 
            #pragma cdir on_adb(num_leftover_right) 
            for(size_t j = 0; j < real_num_vecAfterMerge; j++) {                
                out_idx[j] = out_idx[j] + num_leftover_right[j];
            }
            #pragma _NEC ivdep 
            #pragma cdir nodep 
            #pragma cdir on_adb(out_idx)  
            #pragma cdir on_adb(out_idx_save_stop)   
            for(size_t j = 0; j < real_num_vecAfterMerge; j++) {
                out_idx_save_stop[j] = out_idx[j];
            }
        }
        else{
            size_t left_idx_each[num_par_total];
            size_t left_idx_stop_each[num_par_total];
            size_t right_idx_each[num_par_total];
            size_t right_idx_stop_each[num_par_total];        
            size_t out_idx_each[num_par_total];
            size_t out_idx_each_save[num_par_total];
            
  
            AdaptPartition_2D(buffer_merge_cur.idx, 
                    real_num_vecAfterMerge,
                    num_par_each,
                    left_idx, 
                    left_idx_stop, 
                    right_idx, 
                    right_idx_stop,
                    left_idx_each, 
                    left_idx_stop_each, 
                    right_idx_each, 
                    right_idx_stop_each);
       
            #pragma _NEC ivdep 
            #pragma cdir nodep  
            #pragma cdir on_adb(left_idx_each)
            #pragma cdir on_adb(out_idx_each) 
            for(size_t m=0; m<real_num_vecAfterMerge; m++){ 
               out_idx_each[m*num_par_each + 0] = left_idx_each[m*num_par_each + 0];
            }      
            #pragma _NEC ivdep 
            #pragma cdir nodep        
            for(size_t n=1; n<num_par_each; n++){
            #pragma _NEC ivdep                 
            #pragma cdir nodep                
                for(size_t m=0; m<real_num_vecAfterMerge; m++){           //TODO: UNROLL 
                 #pragma cdir on_adb(out_idx_each) 
                 #pragma cdir on_adb(left_idx_each) 
                 #pragma cdir on_adb(right_idx_stop_each) 
                 #pragma cdir on_adb(right_idx_each) 
                 #pragma cdir on_adb(left_idx_stop_each) 
                 #pragma cdir on_adb(out_idx_each_save)                 
                    out_idx_each[m*num_par_each + n] = left_idx_stop_each[m*num_par_each + n-1] - left_idx_each[m*num_par_each + n-1] 
                            + right_idx_stop_each[m*num_par_each + n-1] - right_idx_each[m*num_par_each + n-1] 
                            + out_idx_each[m*num_par_each + n-1];
                }
            }
            #pragma cdir nodep            
            #pragma _NEC ivdep 
            #pragma cdir on_adb(out_idx_each_save) 
            #pragma cdir on_adb(out_idx_each)             
            for(size_t n=0; n<num_par_total; n++){
               out_idx_each_save[n] = out_idx_each[n];
            }             
             
        std::vector<int> isFinished(num_par_total,0);
        bool all_finished = true;
   
        while(1){   
                #pragma _NEC ivdep 
                #pragma cdir nodep
                #pragma cdir on_adb(buffer_merge_temp_idx_ptr)
                #pragma cdir on_adb(buffer_merge_cur_idx_ptr)
                #pragma cdir on_adb(right_idx_each)
                #pragma cdir on_adb(right_idx_stop_each)
                #pragma cdir on_adb(left_idx_each)
                #pragma cdir on_adb(left_idx_stop_each) 
                #pragma cdir on_adb(out_idx_each)                
                #pragma cdir on_adb(isFinished)                            
                    for(size_t j=0;j< num_par_total;j++){
                        if(left_idx_each[j] < left_idx_stop_each[j] && right_idx_each[j] < right_idx_stop_each[j]){
                            int eq = buffer_merge_cur_idx_ptr[left_idx_each[j]]  == buffer_merge_cur_idx_ptr[right_idx_each[j]];
                            int lt = buffer_merge_cur_idx_ptr[left_idx_each[j]]  < buffer_merge_cur_idx_ptr[right_idx_each[j]];


                            if(eq) {
                              buffer_merge_temp_idx_ptr[out_idx_each[j]] = buffer_merge_cur_idx_ptr[left_idx_each[j]];
                              buffer_merge_temp_idx_ptr[out_idx_each[j]] = buffer_merge_cur_val_ptr[left_idx_each[j]] 
                                      + buffer_merge_cur_val_ptr[right_idx_each[j]];
                              out_idx_each[j]++; 
                              left_idx_each[j]++;
                              right_idx_each[j]++;
                            } 

                            else if(lt){
                              buffer_merge_temp_idx_ptr[out_idx_each[j]] = buffer_merge_cur_idx_ptr[left_idx_each[j]];
                              buffer_merge_temp_idx_ptr[out_idx_each[j]] = buffer_merge_cur_val_ptr[left_idx_each[j]];
                              out_idx_each[j]++; 
                              left_idx_each[j]++;
                            }

                            else{
                              buffer_merge_temp_idx_ptr[out_idx_each[j]] = buffer_merge_cur_idx_ptr[right_idx_each[j]];
                              buffer_merge_temp_idx_ptr[out_idx_each[j]] = buffer_merge_cur_val_ptr[right_idx_each[j]];
                              out_idx_each[j]++; 
                              right_idx_each[j]++;
                            }

                        }                
                        else{ 
                            isFinished[j] = true;
                        }
                    }
                all_finished = true;
                #pragma cdir on_adb(isFinished)
                for(size_t n = 0; n<num_par_total; n++){
                    if(isFinished[n] == false){
                        all_finished = false;
                        break;
                    }
                }
                if(all_finished == true){
                    break;
                }
        }
     
            #pragma cdir nodep
            for(size_t j=0;j< num_par_total;j++){
                size_t pair_idx = j/num_par_each;


                #pragma _NEC ivdep 
                #pragma cdir nodep
                #pragma cdir on_adb(buffer_merge_next_idx_ptr)
                #pragma cdir on_adb(buffer_merge_temp_idx_ptr) 
                #pragma cdir on_adb(out_idx_each)       
                #pragma cdir on_adb(out_idx_each_save)                 
                for(size_t k= 0 ;k< out_idx_each[j] - out_idx_each_save[j];k++){
                    buffer_merge_next_idx_ptr[out_idx[pair_idx]+k] = buffer_merge_temp_idx_ptr[out_idx_each_save[j] + k];              
                    buffer_merge_next_val_ptr[out_idx[pair_idx]+k] = buffer_merge_temp_val_ptr[out_idx_each_save[j] + k];    
                }

                out_idx[pair_idx] +=  out_idx_each[j] - out_idx_each_save[j];

                #pragma _NEC ivdep                     
                #pragma cdir nodep
                #pragma cdir on_adb(buffer_merge_next_idx_ptr)
                #pragma cdir on_adb(buffer_merge_cur_idx_ptr) 
                #pragma cdir on_adb(left_idx_each)       
                #pragma cdir on_adb(left_idx_stop_each)      
//                    std::cout<<left_idx_stop_each[j] - left_idx_each[j]<<std::endl;
                for(size_t k= 0 ;k<left_idx_stop_each[j] - left_idx_each[j] ;k++){
                    buffer_merge_next_idx_ptr[out_idx[pair_idx] +k] = buffer_merge_cur_idx_ptr[left_idx_each[j] + k];   
                    buffer_merge_next_val_ptr[out_idx[pair_idx] +k] = buffer_merge_cur_val_ptr[left_idx_each[j] + k];  
                }

                out_idx[pair_idx] += left_idx_stop_each[j] - left_idx_each[j];
                
                #pragma _NEC ivdep                     
                #pragma cdir nodep
                #pragma cdir on_adb(buffer_merge_next_idx_ptr)
                #pragma cdir on_adb(buffer_merge_cur_idx_ptr) 
                #pragma cdir on_adb(right_idx_each)       
                #pragma cdir on_adb(right_idx_stop_each)                 
                for(size_t k= 0 ;k<right_idx_stop_each[j] - right_idx_each[j];k++){
                    buffer_merge_next_idx_ptr[out_idx[pair_idx] +k] = buffer_merge_cur_idx_ptr[right_idx_each[j] + k];    
                    buffer_merge_next_val_ptr[out_idx[pair_idx] +k] = buffer_merge_cur_val_ptr[right_idx_each[j] + k];                       
                }
                out_idx[pair_idx] += right_idx_stop_each[j] - right_idx_each[j];
            }
                #pragma _NEC ivdep                     
                #pragma cdir nodep  
            for(size_t j = 0; j<real_num_vecAfterMerge; j++){
                out_idx_save_stop[j] = out_idx[j];
            }
        }

        for(size_t p=real_num_vecAfterMerge; p<num_vecAfterMerge; p++){//copy the last vector to buffer_next if it is leftover (odd)
                    #pragma _NEC ivdep                     
                    #pragma cdir nodep            
            for(size_t k= 0 ; k<left_idx_stop[p] - left_idx[p]; k++){ //copy the last vector to the buffer_merge_next
                buffer_merge_next_idx_ptr[out_idx[p]+k] = buffer_merge_cur_idx_ptr[left_idx[p] + k];
                buffer_merge_next_val_ptr[out_idx[p]+k] = buffer_merge_cur_val_ptr[left_idx[p] + k];                
            }
            out_idx[p] += left_idx_stop[p]- left_idx[p];
            out_idx_save_stop[p] = out_idx[p];
        }
          
        idx_left = 0;
        idx_right = 0;
                    #pragma _NEC ivdep                     
                    #pragma cdir nodep        
                #pragma cdir on_adb(left_idx) 
                #pragma cdir on_adb(left_idx_stop)
                #pragma cdir on_adb(out_idx) 
                #pragma cdir on_adb(out_idx_save_p)  
                #pragma cdir on_adb(out_idx_save_stop_p)        
        for(size_t i = 0; i<num_vecAfterMerge;i+=2){
                left_idx[idx_left] = out_idx_save[i];
                left_idx_stop[idx_left] = out_idx_save_stop[i];
                out_idx[idx_left] = out_idx_save[i];
                idx_left ++;
        }
                    #pragma _NEC ivdep                     
                    #pragma cdir nodep        
                #pragma cdir on_adb(out_idx_save_p)  
                #pragma cdir on_adb(out_idx_save_stop_p) 
                #pragma cdir on_adb(right_idx_stop)        
                #pragma cdir on_adb(right_idx)            
        for(size_t i = 1; i<num_vecAfterMerge;i+=2){            

                right_idx[idx_right] = out_idx_save[i];
                right_idx_stop[idx_right] = out_idx_save_stop[i];
                idx_right++;
        }
    
        num_vecToMerge = num_vecAfterMerge;
        num_vecAfterMerge = std::ceil((double)num_vecToMerge/2); 
        #pragma cdir on_adb(buffer_merge_next_idx_ptr) 
        #pragma cdir on_adb(buffer_merge_cur_idx_ptr)   
        for(size_t i = 0;i < max_num_elem; i++){
            buffer_merge_cur_idx_ptr[i] = buffer_merge_next_idx_ptr[i];
        }
    
  }

    size_t num_elem = left_idx_stop[0] - left_idx[0];

    sparse_vector<T> ret_final(num_elem,0);
    auto* ret_idxp = &ret_final.idx[0];
    auto* ret_valp = &ret_final.val[0];
    
    #pragma cdir on_adb(buffer_merge_cur_idx_ptr)
    #pragma cdir on_adb(ret_idxp)    
    #pragma cdir on_adb(ret_valp)       
    for(size_t i=0; i<num_elem; i++){
        ret_idxp[i] = buffer_merge_cur_idx_ptr[i];
        ret_valp[i] = buffer_merge_cur_val_ptr[i];        
    }

    return ret_final;
   
}

template <class T>
sparse_vector_tri<T> set_union_2d_sssp(sparse_vector< T >& inputVecs_buff,
        std::vector<size_t>& vec_idx,
        std::vector<size_t>& vpred,
        size_t upperbound) {
    //For sssp Bellman-ford, vpred added.
    //Generalized version, input/output version are sparse vectors
    auto* vec_idx_ptr = &vec_idx[0];
    auto* vpredp = &vpred[0];

    size_t num_inputVecs = vec_idx.size();

    
    auto* inputVecs_buff_idx_ptr = &inputVecs_buff.idx[0];
    auto* inputVecs_buff_val_ptr = &inputVecs_buff.val[0];
    
    size_t max_num_elem = 0;

    max_num_elem = inputVecs_buff.idx.size();
    if(max_num_elem == 0 || num_inputVecs == 1){
        sparse_vector_tri<T> ret(inputVecs_buff, vpred);
        return ret;
    }
    size_t num_mergeIters = 0;

    //Using only two buffers for merging
    sparse_vector< T > buffer_merge_next(max_num_elem,0);
    sparse_vector< T > buffer_merge_cur(max_num_elem,0);
    sparse_vector< T > buffer_merge_temp(max_num_elem,0);

    
    auto* buffer_merge_next_idx_ptr = &buffer_merge_next.idx[0];
    auto* buffer_merge_cur_idx_ptr = &buffer_merge_cur.idx[0];
    auto* buffer_merge_temp_idx_ptr = &buffer_merge_temp.idx[0];
    
    auto* buffer_merge_next_val_ptr = &buffer_merge_next.val[0];
    auto* buffer_merge_cur_val_ptr = &buffer_merge_cur.val[0];
    auto* buffer_merge_temp_val_ptr = &buffer_merge_temp.val[0];
    
    
    std::vector<T> buffer_merge_next_pred(max_num_elem,0); 
    std::vector<T> buffer_merge_cur_pred(max_num_elem,0); 
    std::vector<T> buffer_merge_temp_pred(max_num_elem,0); 
    auto* buffer_merge_next_pred_ptr = &buffer_merge_next_pred[0];
    auto* buffer_merge_cur_pred_ptr = &buffer_merge_cur_pred[0];
    auto* buffer_merge_temp_pred_ptr = &buffer_merge_temp_pred[0];
        
    
    for(size_t i=0; i<max_num_elem;i++){
        buffer_merge_cur_idx_ptr[i] = inputVecs_buff_idx_ptr[i];
        buffer_merge_cur_val_ptr[i] = inputVecs_buff_val_ptr[i];
    }
       
    size_t num_vecToMerge = num_inputVecs; 
    size_t num_vecAfterMerge = std::ceil((double)num_vecToMerge/2);
   
    std::vector<size_t> left_idx(num_vecAfterMerge,0);
    std::vector<size_t> right_idx(num_vecAfterMerge,0);    
    std::vector<size_t> left_idx_stop(num_vecAfterMerge,0);
    std::vector<size_t> right_idx_stop(num_vecAfterMerge,0);
    std::vector<size_t> out_idx(num_vecAfterMerge,0);
    std::vector<size_t> out_idx_save(num_vecAfterMerge,0);
    std::vector<size_t> out_idx_save_stop(num_vecAfterMerge,0);
    std::vector<size_t> vec_idx_stop(num_inputVecs,0);
    std::vector<int> isFinished(num_vecAfterMerge,0);

    vec_idx_stop[0] = vec_idx[1];
            #pragma _NEC ivdep 
            #pragma cdir nodep
    for(size_t i=1; i<num_inputVecs-1; i++){
        vec_idx_stop[i] = vec_idx_ptr[i+1];
    }    
    vec_idx_stop[num_inputVecs-1] = max_num_elem;
    
    #pragma cdir nodep    
    for(size_t i=0; i<num_inputVecs; i++){    //copy pred into buffer
        #pragma _NEC ivdep         
        #pragma cdir nodep
        for(size_t j=0;j<vec_idx_stop[i]-vec_idx[i];j++){
            buffer_merge_cur_pred_ptr[vec_idx[i] + j] = vpredp[i];
        }
    } 
    
    size_t idx_left = 0;
    size_t idx_right = 0;

  
    #pragma _NEC ivdep 
    #pragma cdir nodep
    #pragma cdir on_adb(vec_idx_ptr)
    #pragma cdir on_adb(vec_idx_stop)    
    #pragma cdir on_adb(left_idx)
    #pragma cdir on_adb(left_idx_stop)
    #pragma cdir on_adb(out_idx)     
    for(size_t i=0; i<num_inputVecs; i+=2){    //set pointers for left vectors
            left_idx[idx_left] = vec_idx_ptr[i];
            left_idx_stop[idx_left] = vec_idx_stop[i];
            out_idx[idx_left] = vec_idx_ptr[i];
            idx_left++;
    }
    #pragma _NEC ivdep 
    #pragma cdir nodep    
    #pragma cdir on_adb(vec_idx_ptr)
    #pragma cdir on_adb(vec_idx_stop)      
    #pragma cdir on_adb(right_idx)
    #pragma cdir on_adb(right_idx_stop)     
    for(size_t i=1; i<num_inputVecs; i+=2){    //set pointers for right vectors     
           right_idx[idx_right] = vec_idx_ptr[i];
           right_idx_stop[idx_right] = vec_idx_stop[i];
           idx_right++;            
    }
  
    while(num_vecToMerge > 1){ 

        num_mergeIters ++;
        #pragma _NEC ivdep 
        #pragma cdir nodep        
        #pragma cdir on_adb(out_idx) 
        #pragma cdir on_adb(out_idx_save) 
        for(size_t j = 0; j < num_vecAfterMerge; j++) {        
            out_idx_save[j] = out_idx[j];
        }
        
        size_t real_num_vecAfterMerge = num_vecAfterMerge - num_vecToMerge%2; //odd and even number of vectorToMerge    
        size_t num_par_each = frovedis::ceil_div((size_t)upperbound,real_num_vecAfterMerge);
        size_t num_par_total = num_par_each * real_num_vecAfterMerge;

        
        if(num_par_each == 1 || max_num_elem < upperbound){ // No need to partition each vector pair
        bool all_finished = true;
        
        #pragma cdir on_adb(isFinished)         
        for(size_t i=0;i<real_num_vecAfterMerge;i++){
                isFinished[i] = false;
        }    

       while(1){   

                #pragma cdir nodep            
                #pragma _NEC ivdep 
                #pragma cdir on_adb(buffer_merge_next_idx_ptr)
                #pragma cdir on_adb(buffer_merge_cur_idx_ptr)
                #pragma cdir on_adb(buffer_merge_next_val_ptr)
                #pragma cdir on_adb(buffer_merge_cur_val_ptr)
                #pragma cdir on_adb(left_idx)
                #pragma cdir on_adb(left_idx_stop)
                #pragma cdir on_adb(right_idx)
                #pragma cdir on_adb(right_idx_stop) 
                #pragma cdir on_adb(out_idx) 
                #pragma cdir on_adb(isFinished)              
                for(size_t j = 0; j < real_num_vecAfterMerge; j++) {

                        if(left_idx[j] < left_idx_stop[j] && right_idx[j] < right_idx_stop[j]){

                         int eq = buffer_merge_cur_idx_ptr[left_idx[j]]  == buffer_merge_cur_idx_ptr[right_idx[j]];
                         int lt = buffer_merge_cur_idx_ptr[left_idx[j]]  < buffer_merge_cur_idx_ptr[right_idx[j]];


                        if(eq) {
                            if( buffer_merge_cur_val_ptr[left_idx[j]] < buffer_merge_cur_val_ptr[right_idx[j]]){
                                buffer_merge_next_idx_ptr[out_idx[j]] = buffer_merge_cur_idx_ptr[left_idx[j]];
                                buffer_merge_next_val_ptr[out_idx[j]] = buffer_merge_cur_val_ptr[left_idx[j]];
                                buffer_merge_next_pred_ptr[out_idx[j]] = buffer_merge_temp_pred_ptr[left_idx[j]];                            
                            }else{
                                buffer_merge_next_idx_ptr[out_idx[j]] = buffer_merge_cur_idx_ptr[right_idx[j]];
                                buffer_merge_next_val_ptr[out_idx[j]] = buffer_merge_cur_val_ptr[right_idx[j]];
                                buffer_merge_next_pred_ptr[out_idx[j]] = buffer_merge_temp_pred_ptr[right_idx[j]];                                  
                            }
                          out_idx[j]++;
                          left_idx[j]++;
                          right_idx[j]++;
                        } 
                        else if(lt) {
                            buffer_merge_next_idx_ptr[out_idx[j]] = buffer_merge_cur_idx_ptr[left_idx[j]];
                            buffer_merge_next_val_ptr[out_idx[j]] = buffer_merge_cur_val_ptr[left_idx[j]];
                            buffer_merge_next_pred_ptr[out_idx[j]] = buffer_merge_temp_pred_ptr[left_idx[j]];
                          out_idx[j]++;
                          left_idx[j]++;
                        } 
                        else{
                          buffer_merge_next_idx_ptr[out_idx[j]] = buffer_merge_cur_idx_ptr[right_idx[j]];
                          buffer_merge_next_val_ptr[out_idx[j]] = buffer_merge_cur_val_ptr[right_idx[j]];
                          buffer_merge_next_pred_ptr[out_idx[j]] = buffer_merge_temp_pred_ptr[right_idx[j]];                          
                          out_idx[j]++;
                          right_idx[j]++;
                        }                       
                    }                
                    else{ 
                        isFinished[j] = true;
                    }
                }
                all_finished = true;
                #pragma cdir on_adb(isFinished)
                for(size_t m = 0; m<real_num_vecAfterMerge; m++){
                    if(isFinished[m] == false){
                        all_finished = false;
                        break;
                    }
                }
                if(all_finished == true){
                    break;
                }

            }
  
  size_t num_leftover_left[real_num_vecAfterMerge];
  size_t num_leftover_right[real_num_vecAfterMerge];
                #pragma _NEC ivdep 
                #pragma cdir nodep    
                #pragma cdir on_adb(left_idx)
                #pragma cdir on_adb(left_idx_stop)
                #pragma cdir on_adb(right_idx)
                #pragma cdir on_adb(right_idx_stop)
                #pragma cdir on_adb(num_leftover_left)
                #pragma cdir on_adb(num_leftover_right)
            for(size_t j = 0; j < real_num_vecAfterMerge; j++) {
                num_leftover_left[j] = left_idx_stop[j] - left_idx[j];
                num_leftover_right[j] = right_idx_stop[j] - right_idx[j];
            } 
  
  
            #pragma _NEC ivdep 
            #pragma cdir nodep
            for(size_t j = 0; j < real_num_vecAfterMerge; j++) {             
            #pragma cdir nodep
            #pragma cdir on_adb(buffer_merge_next_idx_ptr)
            #pragma cdir on_adb(buffer_merge_cur_idx_ptr)  
                for(size_t k= 0 ;k<num_leftover_left[j];k++){
                    buffer_merge_next_idx_ptr[out_idx[j] + k] =   buffer_merge_cur_idx_ptr[left_idx[j] + k];  
                    buffer_merge_next_val_ptr[out_idx[j] + k] =   buffer_merge_cur_val_ptr[left_idx[j] + k];
                    buffer_merge_next_pred_ptr[out_idx[j] + k] =   buffer_merge_cur_pred_ptr[left_idx[j] + k];  
                }

            #pragma cdir nodep
            #pragma cdir on_adb(buffer_merge_next_idx_ptr)
            #pragma cdir on_adb(buffer_merge_cur_idx_ptr)  
                for(size_t k= 0 ;k<num_leftover_right[j];k++){
                    buffer_merge_next_idx_ptr[out_idx[j] + k] =   buffer_merge_cur_idx_ptr[right_idx[j] + k];    
                    buffer_merge_next_val_ptr[out_idx[j] + k] =   buffer_merge_cur_val_ptr[right_idx[j] + k]; 
                    buffer_merge_next_pred_ptr[out_idx[j] + k] =   buffer_merge_cur_pred_ptr[right_idx[j] + k];                      
                }
            }
            #pragma _NEC ivdep 
            #pragma cdir nodep  
            #pragma cdir on_adb(out_idx) 
            #pragma cdir on_adb(num_leftover_left)  
            for(size_t j = 0; j < real_num_vecAfterMerge; j++) {                
                out_idx[j] = out_idx[j] + num_leftover_left[j];
            }  
            #pragma _NEC ivdep 
            #pragma cdir nodep  
            #pragma cdir on_adb(out_idx) 
            #pragma cdir on_adb(num_leftover_right) 
            for(size_t j = 0; j < real_num_vecAfterMerge; j++) {                
                out_idx[j] = out_idx[j] + num_leftover_right[j];
            }
            #pragma _NEC ivdep 
            #pragma cdir nodep 
            #pragma cdir on_adb(out_idx)  
            #pragma cdir on_adb(out_idx_save_stop)   
            for(size_t j = 0; j < real_num_vecAfterMerge; j++) {
                out_idx_save_stop[j] = out_idx[j];
            }
        }
        else{
            size_t left_idx_each[num_par_total];
            size_t left_idx_stop_each[num_par_total];
            size_t right_idx_each[num_par_total];
            size_t right_idx_stop_each[num_par_total];        
            size_t out_idx_each[num_par_total];
            size_t out_idx_each_save[num_par_total];
            
  
            AdaptPartition_2D(buffer_merge_cur.idx, 
                    real_num_vecAfterMerge,
                    num_par_each,
                    left_idx, 
                    left_idx_stop, 
                    right_idx, 
                    right_idx_stop,
                    left_idx_each, 
                    left_idx_stop_each, 
                    right_idx_each, 
                    right_idx_stop_each);
       
            #pragma _NEC ivdep 
            #pragma cdir nodep  
            #pragma cdir on_adb(left_idx_each)
            #pragma cdir on_adb(out_idx_each) 
            for(size_t m=0; m<real_num_vecAfterMerge; m++){ 
               out_idx_each[m*num_par_each + 0] = left_idx_each[m*num_par_each + 0];
            }      
            #pragma _NEC ivdep 
            #pragma cdir nodep        
            for(size_t n=1; n<num_par_each; n++){
            #pragma _NEC ivdep                 
            #pragma cdir nodep                
                for(size_t m=0; m<real_num_vecAfterMerge; m++){           //TODO: UNROLL 
                 #pragma cdir on_adb(out_idx_each) 
                 #pragma cdir on_adb(left_idx_each) 
                 #pragma cdir on_adb(right_idx_stop_each) 
                 #pragma cdir on_adb(right_idx_each) 
                 #pragma cdir on_adb(left_idx_stop_each) 
                 #pragma cdir on_adb(out_idx_each_save)                 
                    out_idx_each[m*num_par_each + n] = left_idx_stop_each[m*num_par_each + n-1] - left_idx_each[m*num_par_each + n-1] 
                            + right_idx_stop_each[m*num_par_each + n-1] - right_idx_each[m*num_par_each + n-1] 
                            + out_idx_each[m*num_par_each + n-1];
                }
            }
            #pragma cdir nodep            
            #pragma _NEC ivdep 
            #pragma cdir on_adb(out_idx_each_save) 
            #pragma cdir on_adb(out_idx_each)             
            for(size_t n=0; n<num_par_total; n++){
               out_idx_each_save[n] = out_idx_each[n];
            }             
             
        std::vector<int> isFinished(num_par_total,0);
        bool all_finished = true;
   
        while(1){   
                #pragma _NEC ivdep 
                #pragma cdir nodep
                #pragma cdir on_adb(buffer_merge_temp_idx_ptr)
                #pragma cdir on_adb(buffer_merge_cur_idx_ptr)
                #pragma cdir on_adb(right_idx_each)
                #pragma cdir on_adb(right_idx_stop_each)
                #pragma cdir on_adb(left_idx_each)
                #pragma cdir on_adb(left_idx_stop_each) 
                #pragma cdir on_adb(out_idx_each)                
                #pragma cdir on_adb(isFinished)                            
                    for(size_t j=0;j< num_par_total;j++){
                        if(left_idx_each[j] < left_idx_stop_each[j] && right_idx_each[j] < right_idx_stop_each[j]){
                            int eq = buffer_merge_cur_idx_ptr[left_idx_each[j]]  == buffer_merge_cur_idx_ptr[right_idx_each[j]];
                            int lt = buffer_merge_cur_idx_ptr[left_idx_each[j]]  < buffer_merge_cur_idx_ptr[right_idx_each[j]];


                            if(eq) {
                              buffer_merge_temp_idx_ptr[out_idx_each[j]] = buffer_merge_cur_idx_ptr[left_idx_each[j]];
                              
                              if(buffer_merge_cur_val_ptr[left_idx_each[j]] < buffer_merge_cur_val_ptr[right_idx_each[j]]){
                                buffer_merge_temp_idx_ptr[out_idx_each[j]] = buffer_merge_cur_idx_ptr[left_idx_each[j]];
                                buffer_merge_temp_val_ptr[out_idx_each[j]] = buffer_merge_cur_val_ptr[left_idx_each[j]];
                                buffer_merge_temp_pred_ptr[out_idx_each[j]] = buffer_merge_cur_pred_ptr[left_idx_each[j]];                                   
                              }else{
                                buffer_merge_temp_idx_ptr[out_idx_each[j]] = buffer_merge_cur_idx_ptr[right_idx_each[j]];
                                buffer_merge_temp_val_ptr[out_idx_each[j]] = buffer_merge_cur_val_ptr[right_idx_each[j]];
                                buffer_merge_temp_pred_ptr[out_idx_each[j]] = buffer_merge_cur_pred_ptr[right_idx_each[j]];   
                              }                             
                              out_idx_each[j]++; 
                              left_idx_each[j]++;
                              right_idx_each[j]++;
                            } 

                            else if(lt){
                              buffer_merge_temp_idx_ptr[out_idx_each[j]] = buffer_merge_cur_idx_ptr[left_idx_each[j]];
                              buffer_merge_temp_val_ptr[out_idx_each[j]] = buffer_merge_cur_val_ptr[left_idx_each[j]];
                              buffer_merge_temp_pred_ptr[out_idx_each[j]] = buffer_merge_cur_pred_ptr[left_idx_each[j]];                                
                              out_idx_each[j]++; 
                              left_idx_each[j]++;
                            }

                            else{
                              buffer_merge_temp_idx_ptr[out_idx_each[j]] = buffer_merge_cur_idx_ptr[right_idx_each[j]];
                              buffer_merge_temp_val_ptr[out_idx_each[j]] = buffer_merge_cur_val_ptr[right_idx_each[j]];
                              buffer_merge_temp_pred_ptr[out_idx_each[j]] = buffer_merge_cur_pred_ptr[right_idx_each[j]];                                
                              out_idx_each[j]++; 
                              right_idx_each[j]++;
                            }

                        }                
                        else{ 
                            isFinished[j] = true;
                        }
                    }
                all_finished = true;
                #pragma cdir on_adb(isFinished)
                for(size_t n = 0; n<num_par_total; n++){
                    if(isFinished[n] == false){
                        all_finished = false;
                        break;
                    }
                }
                if(all_finished == true){
                    break;
                }
        }
     
            #pragma cdir nodep
            for(size_t j=0;j< num_par_total;j++){
                size_t pair_idx = j/num_par_each;


                #pragma _NEC ivdep 
                #pragma cdir nodep
                #pragma cdir on_adb(buffer_merge_next_idx_ptr)
                #pragma cdir on_adb(buffer_merge_temp_idx_ptr) 
                #pragma cdir on_adb(out_idx_each)       
                #pragma cdir on_adb(out_idx_each_save)                 
                for(size_t k= 0 ;k< out_idx_each[j] - out_idx_each_save[j];k++){
                    buffer_merge_next_idx_ptr[out_idx[pair_idx]+k] = buffer_merge_temp_idx_ptr[out_idx_each_save[j] + k];              
                    buffer_merge_next_val_ptr[out_idx[pair_idx]+k] = buffer_merge_temp_val_ptr[out_idx_each_save[j] + k];
                    buffer_merge_next_pred_ptr[out_idx[pair_idx]+k] = buffer_merge_temp_pred_ptr[out_idx_each_save[j] + k];                      
                }

                out_idx[pair_idx] +=  out_idx_each[j] - out_idx_each_save[j];

                #pragma _NEC ivdep                     
                #pragma cdir nodep
                #pragma cdir on_adb(buffer_merge_next_idx_ptr)
                #pragma cdir on_adb(buffer_merge_cur_idx_ptr) 
                #pragma cdir on_adb(left_idx_each)       
                #pragma cdir on_adb(left_idx_stop_each)      
                for(size_t k= 0 ;k<left_idx_stop_each[j] - left_idx_each[j] ;k++){
                    buffer_merge_next_idx_ptr[out_idx[pair_idx] +k] = buffer_merge_cur_idx_ptr[left_idx_each[j] + k];   
                    buffer_merge_next_val_ptr[out_idx[pair_idx] +k] = buffer_merge_cur_val_ptr[left_idx_each[j] + k];  
                    buffer_merge_next_pred_ptr[out_idx[pair_idx] +k] = buffer_merge_cur_pred_ptr[left_idx_each[j] + k];                      
                }

                out_idx[pair_idx] += left_idx_stop_each[j] - left_idx_each[j];
                
                #pragma _NEC ivdep                     
                #pragma cdir nodep
                #pragma cdir on_adb(buffer_merge_next_idx_ptr)
                #pragma cdir on_adb(buffer_merge_cur_idx_ptr) 
                #pragma cdir on_adb(right_idx_each)       
                #pragma cdir on_adb(right_idx_stop_each)                 
                for(size_t k= 0 ;k<right_idx_stop_each[j] - right_idx_each[j];k++){
                    buffer_merge_next_idx_ptr[out_idx[pair_idx] +k] = buffer_merge_cur_idx_ptr[right_idx_each[j] + k];    
                    buffer_merge_next_val_ptr[out_idx[pair_idx] +k] = buffer_merge_cur_val_ptr[right_idx_each[j] + k];  
                    buffer_merge_next_pred_ptr[out_idx[pair_idx] +k] = buffer_merge_cur_pred_ptr[right_idx_each[j] + k];                     
                }
                out_idx[pair_idx] += right_idx_stop_each[j] - right_idx_each[j];
            }
                #pragma _NEC ivdep                     
                #pragma cdir nodep  
            for(size_t j = 0; j<real_num_vecAfterMerge; j++){
                out_idx_save_stop[j] = out_idx[j];
            }
        }

        for(size_t p=real_num_vecAfterMerge; p<num_vecAfterMerge; p++){//copy the last vector to buffer_next if it is leftover (odd)
                    #pragma _NEC ivdep                     
                    #pragma cdir nodep            
            for(size_t k= 0 ; k<left_idx_stop[p] - left_idx[p]; k++){ //copy the last vector to the buffer_merge_next
                buffer_merge_next_idx_ptr[out_idx[p]+k] = buffer_merge_cur_idx_ptr[left_idx[p] + k];
                buffer_merge_next_val_ptr[out_idx[p]+k] = buffer_merge_cur_val_ptr[left_idx[p] + k];     
                buffer_merge_next_pred_ptr[out_idx[p]+k] = buffer_merge_cur_pred_ptr[left_idx[p] + k];                  
            }
            out_idx[p] += left_idx_stop[p]- left_idx[p];
            out_idx_save_stop[p] = out_idx[p];
        }
          
        idx_left = 0;
        idx_right = 0;
                    #pragma _NEC ivdep                     
                    #pragma cdir nodep        
                #pragma cdir on_adb(left_idx) 
                #pragma cdir on_adb(left_idx_stop)
                #pragma cdir on_adb(out_idx) 
                #pragma cdir on_adb(out_idx_save_p)  
                #pragma cdir on_adb(out_idx_save_stop_p)        
        for(size_t i = 0; i<num_vecAfterMerge;i+=2){
                left_idx[idx_left] = out_idx_save[i];
                left_idx_stop[idx_left] = out_idx_save_stop[i];
                out_idx[idx_left] = out_idx_save[i];
                idx_left ++;
        }
                    #pragma _NEC ivdep                     
                    #pragma cdir nodep        
                #pragma cdir on_adb(out_idx_save_p)  
                #pragma cdir on_adb(out_idx_save_stop_p) 
                #pragma cdir on_adb(right_idx_stop)        
                #pragma cdir on_adb(right_idx)            
        for(size_t i = 1; i<num_vecAfterMerge;i+=2){            

                right_idx[idx_right] = out_idx_save[i];
                right_idx_stop[idx_right] = out_idx_save_stop[i];
                idx_right++;
        }
    
        num_vecToMerge = num_vecAfterMerge;
        num_vecAfterMerge = std::ceil((double)num_vecToMerge/2); 
        #pragma cdir on_adb(buffer_merge_next_idx_ptr) 
        #pragma cdir on_adb(buffer_merge_cur_idx_ptr)   
        for(size_t i = 0;i < max_num_elem; i++){
            buffer_merge_cur_idx_ptr[i] = buffer_merge_next_idx_ptr[i];
        }
    
  }

    size_t num_elem = left_idx_stop[0] - left_idx[0];

    sparse_vector_tri<T> ret_final(num_elem,0);
    auto* ret_idxp = &ret_final.idx[0];
    auto* ret_valp = &ret_final.val[0];
    auto* ret_predp = &ret_final.pred[0];
    
    #pragma cdir on_adb(buffer_merge_cur_idx_ptr)
    #pragma cdir on_adb(ret_idxp)    
    #pragma cdir on_adb(ret_valp)       
    for(size_t i=0; i<num_elem; i++){
        ret_idxp[i] = buffer_merge_cur_idx_ptr[i];
        ret_valp[i] = buffer_merge_cur_val_ptr[i];
        ret_predp[i] = buffer_merge_cur_pred_ptr[i];
//        std::cout<<i<<" : "<<ret_idxp[i]<<" : "<<ret_valp[i]<<" : "<<ret_predp[i]<<std::endl;
    }

    return ret_final;
   
}

template <class T>
std::vector<T> set_union_2d_idx_highd(std::vector< T >& inputVecs_buff,
        std::vector< size_t >& num_elem_per_inputVecs, 
        std::vector<size_t>& vec_idx, 
        size_t merge_th) {
 
    auto* vec_idx_ptr = &vec_idx[0];
    size_t num_inputVecs = num_elem_per_inputVecs.size();
    auto* inputVecs_buff_ptr = &inputVecs_buff[0];
    auto* num_elem_per_inputVecs_ptr = &num_elem_per_inputVecs[0];
    size_t max_num_elem = 0;

    max_num_elem = inputVecs_buff.size();
    std::vector<T> ret;
    if(max_num_elem == 0){
        return ret;
    }
    size_t num_mergeIters = 0;

    //Using only two buffers for merging
    std::vector< T > buffer_merge_next(max_num_elem,0);
    std::vector< T > buffer_merge_cur(max_num_elem,0);
    std::vector< T > buffer_merge_temp(max_num_elem,0);
    auto* buffer_merge_next_ptr = &buffer_merge_next[0];
    auto* buffer_merge_cur_ptr = &buffer_merge_cur[0];
    auto* buffer_merge_temp_ptr = &buffer_merge_temp[0];
    for(size_t i=0; i<max_num_elem;i++){
        buffer_merge_cur_ptr[i] = inputVecs_buff_ptr[i];
    }
       
    size_t num_vecToMerge = num_inputVecs; 
    size_t num_vecAfterMerge = std::ceil((double)num_vecToMerge/2);
   
    std::vector<size_t> left_idx(num_vecAfterMerge,0);
    std::vector<size_t> right_idx(num_vecAfterMerge,0);    
    std::vector<size_t> left_idx_stop(num_vecAfterMerge,0);
    std::vector<size_t> right_idx_stop(num_vecAfterMerge,0);
    std::vector<size_t> out_idx(num_vecAfterMerge,0);
    std::vector<size_t> out_idx_save(num_vecAfterMerge,0);
    std::vector<size_t> out_idx_save_stop(num_vecAfterMerge,0);
    std::vector<size_t> vec_idx_stop(num_inputVecs,0);
    std::vector<int> isFinished(num_vecAfterMerge,0);

    vec_idx_stop[0] = num_elem_per_inputVecs_ptr[0];
            #pragma _NEC ivdep 
            #pragma cdir nodep
    for(size_t i=1; i<num_inputVecs-1; i++){
        vec_idx_stop[i] = vec_idx_ptr[i+1];
    }    
    vec_idx_stop[num_inputVecs-1] = max_num_elem;
    
    size_t idx_left = 0;
    size_t idx_right = 0;

  
    #pragma _NEC ivdep 
    #pragma cdir nodep
    #pragma cdir on_adb(vec_idx_ptr)
    #pragma cdir on_adb(vec_idx_stop)    
    #pragma cdir on_adb(left_idx)
    #pragma cdir on_adb(left_idx_stop)
    #pragma cdir on_adb(out_idx)     
    for(size_t i=0; i<num_inputVecs; i+=2){    //set pointers for left vectors
            left_idx[idx_left] = vec_idx_ptr[i];
            left_idx_stop[idx_left] = vec_idx_stop[i];
            out_idx[idx_left] = vec_idx_ptr[i];
            idx_left++;
    }
    #pragma _NEC ivdep 
    #pragma cdir nodep    
    #pragma cdir on_adb(vec_idx_ptr)
    #pragma cdir on_adb(vec_idx_stop)      
    #pragma cdir on_adb(right_idx)
    #pragma cdir on_adb(right_idx_stop)     
    for(size_t i=1; i<num_inputVecs; i+=2){    //set pointers for right vectors     
           right_idx[idx_right] = vec_idx_ptr[i];
           right_idx_stop[idx_right] = vec_idx_stop[i];
           idx_right++;            
    }
  
    while(num_vecToMerge > 1){ 

        num_mergeIters ++;
        #pragma _NEC ivdep 
        #pragma cdir nodep        
        #pragma cdir on_adb(out_idx) 
        #pragma cdir on_adb(out_idx_save) 
        for(size_t j = 0; j < num_vecAfterMerge; j++) {        
            out_idx_save[j] = out_idx[j];
        }
        
        size_t real_num_vecAfterMerge = num_vecAfterMerge - num_vecToMerge%2; //odd and even number of vectorToMerge 
        size_t upper_bound = merge_th;        
        size_t num_par_each = frovedis::ceil_div((size_t)upper_bound,real_num_vecAfterMerge);
        size_t num_par_total = num_par_each * real_num_vecAfterMerge;

        
        if(num_par_each == 1 || max_num_elem < upper_bound){
        bool all_finished = true;
        
        #pragma cdir on_adb(isFinished)         
        for(size_t i=0;i<real_num_vecAfterMerge;i++){
                isFinished[i] = false;
        }    

       while(1){   

                #pragma cdir nodep            
                #pragma _NEC ivdep 
                #pragma cdir on_adb(buffer_merge_next_ptr)
                #pragma cdir on_adb(buffer_merge_cur_ptr)
                #pragma cdir on_adb(left_idx)
                #pragma cdir on_adb(left_idx_stop)
                #pragma cdir on_adb(right_idx)
                #pragma cdir on_adb(right_idx_stop) 
                #pragma cdir on_adb(out_idx) 
                #pragma cdir on_adb(isFinished)              
                for(size_t j = 0; j < real_num_vecAfterMerge; j++) {

                        if(left_idx[j] < left_idx_stop[j] && right_idx[j] < right_idx_stop[j]){

                         int eq = buffer_merge_cur_ptr[left_idx[j]]  == buffer_merge_cur_ptr[right_idx[j]];
                         int lt = buffer_merge_cur_ptr[left_idx[j]]  < buffer_merge_cur_ptr[right_idx[j]];


                        if(eq || lt) {
                          buffer_merge_next_ptr[out_idx[j]++] = buffer_merge_cur_ptr[left_idx[j]];
                          left_idx[j]++;
                        } 

                        else{
                          buffer_merge_next_ptr[out_idx[j]++] = buffer_merge_cur_ptr[right_idx[j]];
                          right_idx[j]++;
                        }
                        
                        if(eq) {
                          right_idx[j]++;
                        }

                    }                
                    else{ 
                        isFinished[j] = true;
                    }
                }
                all_finished = true;
                #pragma cdir on_adb(isFinished)
                for(size_t m = 0; m<real_num_vecAfterMerge; m++){
                    if(isFinished[m] == false){
                        all_finished = false;
                        break;
                    }
                }
                if(all_finished == true){
                    break;
                }

            }
  
  size_t num_leftover_left[real_num_vecAfterMerge];
  size_t num_leftover_right[real_num_vecAfterMerge];
                #pragma _NEC ivdep 
                #pragma cdir nodep    
                #pragma cdir on_adb(left_idx)
                #pragma cdir on_adb(left_idx_stop)
                #pragma cdir on_adb(right_idx)
                #pragma cdir on_adb(right_idx_stop)
                #pragma cdir on_adb(num_leftover_left)
                #pragma cdir on_adb(num_leftover_right)
            for(size_t j = 0; j < real_num_vecAfterMerge; j++) {
                num_leftover_left[j] = left_idx_stop[j] - left_idx[j];
                num_leftover_right[j] = right_idx_stop[j] - right_idx[j];
            } 
  
  
            #pragma _NEC ivdep 
            #pragma cdir nodep
            for(size_t j = 0; j < real_num_vecAfterMerge; j++) {             
            #pragma cdir nodep
            #pragma cdir on_adb(buffer_merge_next_ptr)
            #pragma cdir on_adb(buffer_merge_cur_ptr)  
                for(size_t k= 0 ;k<num_leftover_left[j];k++){
                    buffer_merge_next_ptr[out_idx[j] + k] =   buffer_merge_cur_ptr[left_idx[j] + k];              
                }

            #pragma cdir nodep
            #pragma cdir on_adb(buffer_merge_next_ptr)
            #pragma cdir on_adb(buffer_merge_cur_ptr)  
                for(size_t k= 0 ;k<num_leftover_right[j];k++){
                    buffer_merge_next_ptr[out_idx[j] + k] =   buffer_merge_cur_ptr[right_idx[j] + k];                              
                }
            }
            #pragma _NEC ivdep 
            #pragma cdir nodep  
            #pragma cdir on_adb(out_idx) 
            #pragma cdir on_adb(num_leftover_left)  
            for(size_t j = 0; j < real_num_vecAfterMerge; j++) {                
                out_idx[j] = out_idx[j] + num_leftover_left[j];
            }  
            #pragma _NEC ivdep 
            #pragma cdir nodep  
            #pragma cdir on_adb(out_idx) 
            #pragma cdir on_adb(num_leftover_right) 
            for(size_t j = 0; j < real_num_vecAfterMerge; j++) {                
                out_idx[j] = out_idx[j] + num_leftover_right[j];
            }
            #pragma _NEC ivdep 
            #pragma cdir nodep 
            #pragma cdir on_adb(out_idx)  
            #pragma cdir on_adb(out_idx_save_stop)   
            for(size_t j = 0; j < real_num_vecAfterMerge; j++) {
                out_idx_save_stop[j] = out_idx[j];
            }
        }
        else{
            size_t left_idx_each[num_par_total];
            size_t left_idx_stop_each[num_par_total];
            size_t right_idx_each[num_par_total];
            size_t right_idx_stop_each[num_par_total];        
            size_t out_idx_each[num_par_total];
            size_t out_idx_each_save[num_par_total];
            
  
            AdaptPartition_2D(buffer_merge_cur, 
                    real_num_vecAfterMerge,
                    num_par_each,
                    left_idx, 
                    left_idx_stop, 
                    right_idx, 
                    right_idx_stop,
                    left_idx_each, 
                    left_idx_stop_each, 
                    right_idx_each, 
                    right_idx_stop_each);
       
            #pragma _NEC ivdep 
            #pragma cdir nodep  
            #pragma cdir on_adb(left_idx_each)
            #pragma cdir on_adb(out_idx_each) 
            for(size_t m=0; m<real_num_vecAfterMerge; m++){ 
               out_idx_each[m*num_par_each + 0] = left_idx_each[m*num_par_each + 0];
            }      
            #pragma _NEC ivdep 
            #pragma cdir nodep        
            for(size_t n=1; n<num_par_each; n++){
            #pragma _NEC ivdep                 
            #pragma cdir nodep                
                for(size_t m=0; m<real_num_vecAfterMerge; m++){           //TODO: UNROLL 
                 #pragma cdir on_adb(out_idx_each) 
                 #pragma cdir on_adb(left_idx_each) 
                 #pragma cdir on_adb(right_idx_stop_each) 
                 #pragma cdir on_adb(right_idx_each) 
                 #pragma cdir on_adb(left_idx_stop_each) 
                 #pragma cdir on_adb(out_idx_each_save)                 
                    out_idx_each[m*num_par_each + n] = left_idx_stop_each[m*num_par_each + n-1] - left_idx_each[m*num_par_each + n-1] 
                            + right_idx_stop_each[m*num_par_each + n-1] - right_idx_each[m*num_par_each + n-1] 
                            + out_idx_each[m*num_par_each + n-1];
                }
            }
            #pragma cdir nodep            
            #pragma _NEC ivdep 
            #pragma cdir on_adb(out_idx_each_save) 
            #pragma cdir on_adb(out_idx_each)             
            for(size_t n=0; n<num_par_total; n++){
               out_idx_each_save[n] = out_idx_each[n];
            }             
             
        std::vector<int> isFinished(num_par_total,0);
        bool all_finished = true;
   
        while(1){   
                #pragma _NEC ivdep 
                #pragma cdir nodep
                #pragma cdir on_adb(buffer_merge_temp_ptr)
                #pragma cdir on_adb(buffer_merge_cur_ptr)
                #pragma cdir on_adb(right_idx_each)
                #pragma cdir on_adb(right_idx_stop_each)
                #pragma cdir on_adb(left_idx_each)
                #pragma cdir on_adb(left_idx_stop_each) 
                #pragma cdir on_adb(out_idx_each)                
                #pragma cdir on_adb(isFinished)                            
                    for(size_t j=0;j< num_par_total;j++){
                        if(left_idx_each[j] < left_idx_stop_each[j] && right_idx_each[j] < right_idx_stop_each[j]){
                            int eq = buffer_merge_cur_ptr[left_idx_each[j]]  == buffer_merge_cur_ptr[right_idx_each[j]];
                            int lt = buffer_merge_cur_ptr[left_idx_each[j]]  < buffer_merge_cur_ptr[right_idx_each[j]];


                            if(eq || lt) {
                              buffer_merge_temp_ptr[out_idx_each[j]++] = buffer_merge_cur_ptr[left_idx_each[j]];
                                left_idx_each[j]++;
                            } 

                            else{
                              buffer_merge_temp_ptr[out_idx_each[j]++] = buffer_merge_cur_ptr[right_idx_each[j]];
                              right_idx_each[j]++;
                            }

                            if(eq) {
                              right_idx_each[j]++;
                            }

                        }                
                        else{ 
                            isFinished[j] = true;
                        }
                    }
                all_finished = true;
                #pragma cdir on_adb(isFinished)
                for(size_t n = 0; n<num_par_total; n++){
                    if(isFinished[n] == false){
                        all_finished = false;
                        break;
                    }
                }
                if(all_finished == true){
                    break;
                }
        }
     
            #pragma cdir nodep
            for(size_t j=0;j< num_par_total;j++){
                size_t pair_idx = j/num_par_each;


                #pragma _NEC ivdep 
                #pragma cdir nodep
                #pragma cdir on_adb(buffer_merge_next_ptr)
                #pragma cdir on_adb(buffer_merge_temp_ptr) 
                #pragma cdir on_adb(out_idx_each)       
                #pragma cdir on_adb(out_idx_each_save)                 
                for(size_t k= 0 ;k< out_idx_each[j] - out_idx_each_save[j];k++){
                    buffer_merge_next_ptr[out_idx[pair_idx]+k] = buffer_merge_temp_ptr[out_idx_each_save[j] + k];              
                }

                out_idx[pair_idx] +=  out_idx_each[j] - out_idx_each_save[j];

                #pragma _NEC ivdep                     
                #pragma cdir nodep
                #pragma cdir on_adb(buffer_merge_next_ptr)
                #pragma cdir on_adb(buffer_merge_cur_ptr) 
                #pragma cdir on_adb(left_idx_each)       
                #pragma cdir on_adb(left_idx_stop_each)      
//                    std::cout<<left_idx_stop_each[j] - left_idx_each[j]<<std::endl;
                for(size_t k= 0 ;k<left_idx_stop_each[j] - left_idx_each[j] ;k++){
                    buffer_merge_next_ptr[out_idx[pair_idx] +k] = buffer_merge_cur_ptr[left_idx_each[j] + k];              
                }

                out_idx[pair_idx] += left_idx_stop_each[j] - left_idx_each[j];
                
                #pragma _NEC ivdep                     
                #pragma cdir nodep
                #pragma cdir on_adb(buffer_merge_next_ptr)
                #pragma cdir on_adb(buffer_merge_cur_ptr) 
                #pragma cdir on_adb(right_idx_each)       
                #pragma cdir on_adb(right_idx_stop_each)                 
                for(size_t k= 0 ;k<right_idx_stop_each[j] - right_idx_each[j];k++){
                    buffer_merge_next_ptr[out_idx[pair_idx] +k] = buffer_merge_cur_ptr[right_idx_each[j] + k];              
                }
                out_idx[pair_idx] += right_idx_stop_each[j] - right_idx_each[j];
            }
                #pragma _NEC ivdep                     
                #pragma cdir nodep  
            for(size_t j = 0; j<real_num_vecAfterMerge; j++){
                out_idx_save_stop[j] = out_idx[j];
            }
        }

        for(size_t p=real_num_vecAfterMerge; p<num_vecAfterMerge; p++){//copy the last vector to buffer_next if it is leftover (odd)
                    #pragma _NEC ivdep                     
                    #pragma cdir nodep            
            for(size_t k= 0 ; k<left_idx_stop[p] - left_idx[p]; k++){ //copy the last vector to the buffer_merge_next
                buffer_merge_next_ptr[out_idx[p]+k] = buffer_merge_cur_ptr[left_idx[p] + k];
            }
            out_idx[p] += left_idx_stop[p]- left_idx[p];
            out_idx_save_stop[p] = out_idx[p];
        }
          
        idx_left = 0;
        idx_right = 0;
                    #pragma _NEC ivdep                     
                    #pragma cdir nodep        
                #pragma cdir on_adb(left_idx) 
                #pragma cdir on_adb(left_idx_stop)
                #pragma cdir on_adb(out_idx) 
                #pragma cdir on_adb(out_idx_save_p)  
                #pragma cdir on_adb(out_idx_save_stop_p)        
        for(size_t i = 0; i<num_vecAfterMerge;i+=2){
                left_idx[idx_left] = out_idx_save[i];
                left_idx_stop[idx_left] = out_idx_save_stop[i];
                out_idx[idx_left] = out_idx_save[i];
                idx_left ++;
        }
                    #pragma _NEC ivdep                     
                    #pragma cdir nodep        
                #pragma cdir on_adb(out_idx_save_p)  
                #pragma cdir on_adb(out_idx_save_stop_p) 
                #pragma cdir on_adb(right_idx_stop)        
                #pragma cdir on_adb(right_idx)            
        for(size_t i = 1; i<num_vecAfterMerge;i+=2){            

                right_idx[idx_right] = out_idx_save[i];
                right_idx_stop[idx_right] = out_idx_save_stop[i];
                idx_right++;
        }
    
        num_vecToMerge = num_vecAfterMerge;
        num_vecAfterMerge = std::ceil((double)num_vecToMerge/2); 
        #pragma cdir on_adb(buffer_merge_next_ptr) 
        #pragma cdir on_adb(buffer_merge_cur_ptr)   
        for(size_t i = 0;i < max_num_elem; i++){
            buffer_merge_cur_ptr[i] = buffer_merge_next_ptr[i];
        }
    
  }

    size_t num_elem = left_idx_stop[0] - left_idx[0];

    std::vector<T> ret_final(num_elem,0);
    auto* retp = &ret_final[0];
    #pragma cdir on_adb(buffer_merge_cur_ptr)
    #pragma cdir on_adb(retp)    
    for(size_t i=0; i<num_elem; i++){
        retp[i] = buffer_merge_cur_ptr[i];
    }

    return ret_final;
   
}

template <class T>
std::vector<T> set_union_2d_idx_noleftover(std::vector< T >& inputVecs_buff,
        std::vector< size_t >& num_elem_per_inputVecs, 
        size_t vec_idx[], 
        size_t merge_th) {
    // No leftover copy, 
 
    size_t num_inputVecs = num_elem_per_inputVecs.size();
    auto* inputVecs_buff_ptr = &inputVecs_buff[0];
    auto* num_elem_per_inputVecs_ptr = &num_elem_per_inputVecs[0];
    size_t max_num_elem = 0;

    max_num_elem = inputVecs_buff.size();
    std::vector<T> ret;
    if(max_num_elem == 0){
        return ret;
    }

    size_t num_mergeIters = 0;

    //Using only two buffers for merging
    std::vector< T > buffer_merge_next(max_num_elem,0);
    std::vector< T > buffer_merge_cur(max_num_elem,0);
    std::vector< T > buffer_merge_temp(max_num_elem,0);
    auto* buffer_merge_next_ptr = &buffer_merge_next[0];
    auto* buffer_merge_cur_ptr = &buffer_merge_cur[0];
    auto* buffer_merge_temp_ptr = &buffer_merge_temp[0];
    for(size_t i=0; i<max_num_elem;i++){
        buffer_merge_cur_ptr[i] = inputVecs_buff_ptr[i];
    }
       
    size_t num_vecToMerge = num_inputVecs; 
    size_t num_vecAfterMerge = std::ceil((double)num_vecToMerge/2);
   
    size_t left_idx[num_vecAfterMerge];
    size_t right_idx[num_vecAfterMerge];   
    size_t left_idx_stop[num_vecAfterMerge];
    size_t right_idx_stop[num_vecAfterMerge];
    size_t out_idx[num_vecAfterMerge];
    size_t out_idx_save[num_vecAfterMerge];
    size_t out_idx_save_stop[num_vecAfterMerge];
    size_t vec_idx_stop[num_inputVecs];

    vec_idx_stop[0] = num_elem_per_inputVecs_ptr[0];
            #pragma _NEC ivdep 
            #pragma cdir nodep
    for(size_t i=1; i<num_inputVecs-1; i++){
        vec_idx_stop[i] = vec_idx[i+1];
    }    
    vec_idx_stop[num_inputVecs-1] = max_num_elem;
    
    size_t idx_left = 0;
    size_t idx_right = 0;

  
    #pragma _NEC ivdep 
    #pragma cdir nodep
    #pragma cdir on_adb(vec_idx)
    #pragma cdir on_adb(vec_idx_stop)    
    #pragma cdir on_adb(left_idx)
    #pragma cdir on_adb(left_idx_stop)
    #pragma cdir on_adb(out_idx)     
    for(size_t i=0; i<num_inputVecs; i+=2){    //set pointers for left vectors
            left_idx[idx_left] = vec_idx[i];
            left_idx_stop[idx_left] = vec_idx_stop[i];
            out_idx[idx_left] = vec_idx[i];
            idx_left++;
    }
    #pragma _NEC ivdep 
    #pragma cdir nodep    
    #pragma cdir on_adb(vec_idx)
    #pragma cdir on_adb(vec_idx_stop)      
    #pragma cdir on_adb(right_idx)
    #pragma cdir on_adb(right_idx_stop)     
    for(size_t i=1; i<num_inputVecs; i+=2){    //set pointers for right vectors     
           right_idx[idx_right] = vec_idx[i];
           right_idx_stop[idx_right] = vec_idx_stop[i];
           idx_right++;            
    }
    
  
    while(num_vecToMerge > 1){ 

        num_mergeIters ++;
        #pragma _NEC ivdep 
        #pragma cdir nodep        
        #pragma cdir on_adb(out_idx) 
        #pragma cdir on_adb(out_idx_save) 
        for(size_t j = 0; j < num_vecAfterMerge; j++) {        
            out_idx_save[j] = out_idx[j];
        }
        
        size_t real_num_vecAfterMerge = num_vecAfterMerge - num_vecToMerge%2; //odd and even number of vectorToMerge 
        size_t upper_bound = merge_th;        
        size_t num_par_each = frovedis::ceil_div((size_t)upper_bound,real_num_vecAfterMerge);
        size_t num_par_total = num_par_each * real_num_vecAfterMerge;

        size_t left_idx_each[num_par_total];
        size_t left_idx_stop_each[num_par_total];
        size_t right_idx_each[num_par_total];
        size_t right_idx_stop_each[num_par_total];        
        size_t out_idx_each[num_par_total];
        size_t out_idx_each_save[num_par_total];
        
        if(num_par_each == 1 || max_num_elem < upper_bound){             
        int isFinished[real_num_vecAfterMerge];
        bool all_finished = true;
        
        #pragma cdir on_adb(isFinished)         
        for(size_t i=0;i<real_num_vecAfterMerge;i++){
                isFinished[i] = false;
        }    

       while(1){   
                #pragma cdir nodep            
                #pragma _NEC ivdep 
                #pragma cdir on_adb(buffer_merge_next_ptr)
                #pragma cdir on_adb(buffer_merge_cur_ptr)
                #pragma cdir on_adb(left_idx)
                #pragma cdir on_adb(left_idx_stop)
                #pragma cdir on_adb(right_idx)
                #pragma cdir on_adb(right_idx_stop) 
                #pragma cdir on_adb(out_idx) 
                #pragma cdir on_adb(isFinished)              
                for(size_t j = 0; j < real_num_vecAfterMerge; j++) {
                        if(left_idx[j] < left_idx_stop[j] && right_idx[j] < right_idx_stop[j]){
                            int eq = buffer_merge_cur_ptr[left_idx[j]]  == buffer_merge_cur_ptr[right_idx[j]];
                            int lt = buffer_merge_cur_ptr[left_idx[j]]  < buffer_merge_cur_ptr[right_idx[j]];
                            if(eq || lt) {
                              buffer_merge_next_ptr[out_idx[j]++] = buffer_merge_cur_ptr[left_idx[j]];
                              left_idx[j]++;
                            } 

                            else{
                              buffer_merge_next_ptr[out_idx[j]++] = buffer_merge_cur_ptr[right_idx[j]];
                              right_idx[j]++;
                            }

                            if(eq) {
                              right_idx[j]++;
                            }

                        }
                        else if(left_idx[j] < left_idx_stop[j]){
                                buffer_merge_next_ptr[out_idx[j]++] = buffer_merge_cur_ptr[left_idx[j]++];
                        } 
                        else if(right_idx[j] < right_idx_stop[j]){
                                buffer_merge_next_ptr[out_idx[j]++] = buffer_merge_cur_ptr[right_idx[j]++];
                        }  
                        else{ 
                            isFinished[j] = true;
                        }
                }
                all_finished = true;
                #pragma cdir on_adb(isFinished)
                for(size_t m = 0; m<real_num_vecAfterMerge; m++){
                    if(isFinished[m] == false){
                        all_finished = false;
                        break;
                    }
                }
                if(all_finished == true){
                    break;
                }

            }
            #pragma _NEC ivdep 
            #pragma cdir nodep 
            #pragma cdir on_adb(out_idx)  
            #pragma cdir on_adb(out_idx_save_stop)   
            for(size_t j = 0; j < real_num_vecAfterMerge; j++) {
                out_idx_save_stop[j] = out_idx[j];
            }
        }
        else{
 
            AdaptPartition_2D(buffer_merge_cur, 
                    real_num_vecAfterMerge,
                    num_par_each,
                    left_idx, 
                    left_idx_stop, 
                    right_idx, 
                    right_idx_stop,
                    left_idx_each, 
                    left_idx_stop_each, 
                    right_idx_each, 
                    right_idx_stop_each);
            
            #pragma _NEC ivdep 
            #pragma cdir nodep  
            #pragma cdir on_adb(left_idx_each)
            #pragma cdir on_adb(out_idx_each) 
            for(size_t m=0; m<real_num_vecAfterMerge; m++){ 
               out_idx_each[m*num_par_each + 0] = left_idx_each[m*num_par_each + 0];
            }      
            #pragma _NEC ivdep 
            #pragma cdir nodep        
            for(size_t n=1; n<num_par_each; n++){
            #pragma _NEC ivdep                 
            #pragma cdir nodep                
                for(size_t m=0; m<real_num_vecAfterMerge; m++){           //TODO: UNROLL 
                 #pragma cdir on_adb(out_idx_each) 
                 #pragma cdir on_adb(left_idx_each) 
                 #pragma cdir on_adb(right_idx_stop_each) 
                 #pragma cdir on_adb(right_idx_each) 
                 #pragma cdir on_adb(left_idx_stop_each) 
                 #pragma cdir on_adb(out_idx_each_save)                 
                    out_idx_each[m*num_par_each + n] = left_idx_stop_each[m*num_par_each + n-1] - left_idx_each[m*num_par_each + n-1] 
                            + right_idx_stop_each[m*num_par_each + n-1] - right_idx_each[m*num_par_each + n-1] 
                            + out_idx_each[m*num_par_each + n-1];
                }
            }
            #pragma cdir nodep            
            #pragma _NEC ivdep 
            #pragma cdir on_adb(out_idx_each_save) 
             #pragma cdir on_adb(out_idx_each)             
            for(size_t n=0; n<num_par_total; n++){
                out_idx_each_save[n] = out_idx_each[n];
            }             
             

        int isFinished[num_par_total];
        bool all_finished = true;
        
        #pragma cdir on_adb(isFinished)         
        for(size_t i=0;i<num_par_total;i++){
                isFinished[i] = false;
        }
  
        while(1){   
                #pragma _NEC ivdep 
                #pragma cdir nodep
                #pragma cdir on_adb(buffer_merge_temp_ptr)
                #pragma cdir on_adb(buffer_merge_cur_ptr)
                #pragma cdir on_adb(right_idx_each)
                #pragma cdir on_adb(right_idx_stop_each)
                #pragma cdir on_adb(left_idx_each)
                #pragma cdir on_adb(left_idx_stop_each) 
                #pragma cdir on_adb(out_idx_each)                
                #pragma cdir on_adb(isFinished)                            
                    for(size_t j=0;j< num_par_total;j++){

                        if(left_idx_each[j] < left_idx_stop_each[j] && right_idx_each[j] < right_idx_stop_each[j]){
                            int eq = buffer_merge_cur_ptr[left_idx_each[j]]  == buffer_merge_cur_ptr[right_idx_each[j]];
                            int lt = buffer_merge_cur_ptr[left_idx_each[j]]  < buffer_merge_cur_ptr[right_idx_each[j]];


                            if(eq || lt) {
                              buffer_merge_temp_ptr[out_idx_each[j]++] = buffer_merge_cur_ptr[left_idx_each[j]];
                                left_idx_each[j]++;
                            } 

                            else{
                              buffer_merge_temp_ptr[out_idx_each[j]++] = buffer_merge_cur_ptr[right_idx_each[j]];
                              right_idx_each[j]++;
                            }

                            if(eq) {
                              right_idx_each[j]++;
                            }

                        }                
                        else if(left_idx_each[j] < left_idx_stop_each[j]){
                            buffer_merge_temp_ptr[out_idx_each[j]++] = buffer_merge_cur_ptr[left_idx_each[j]++];
                        } 
                        else if(right_idx_each[j] < right_idx_stop_each[j]){
                            buffer_merge_temp_ptr[out_idx_each[j]++] = buffer_merge_cur_ptr[right_idx_each[j]++];
                        }                    
                        else{
                            isFinished[j] = true;
                        }
                    }

                all_finished = true;
                #pragma cdir on_adb(isFinished)
                for(size_t n = 0; n<num_par_total; n++){
                    if(isFinished[n] == false){
                        all_finished = false;
                        break;
                    }
                }
                if(all_finished == true){
                    break;
                }
        }
            #pragma cdir nodep
            for(size_t j=0;j< num_par_total;j++){
                size_t pair_idx = j/num_par_each;


                #pragma _NEC ivdep 
                #pragma cdir nodep
                #pragma cdir on_adb(buffer_merge_next_ptr)
                #pragma cdir on_adb(buffer_merge_temp_ptr) 
                #pragma cdir on_adb(out_idx_each)       
                #pragma cdir on_adb(out_idx_each_save)                 
                for(size_t k= 0 ;k< out_idx_each[j] - out_idx_each_save[j];k++){
                    buffer_merge_next_ptr[out_idx[pair_idx]+k] = buffer_merge_temp_ptr[out_idx_each_save[j] + k];              
                }

                out_idx[pair_idx] +=  out_idx_each[j] - out_idx_each_save[j];


                #pragma _NEC ivdep                     
                #pragma cdir nodep  
                for(size_t j = 0; j<real_num_vecAfterMerge; j++){
                    out_idx_save_stop[j] = out_idx[j];
                }
            }
        }

        for(size_t p=real_num_vecAfterMerge; p<num_vecAfterMerge; p++){//copy the last vector to buffer_next if it is leftover (odd)
            #pragma _NEC ivdep                     
            #pragma cdir nodep            
            for(size_t k= 0 ; k<left_idx_stop[p] - left_idx[p]; k++){ //copy the last vector to the buffer_merge_next
                buffer_merge_next_ptr[out_idx[p]+k] = buffer_merge_cur_ptr[left_idx[p] + k];
            }
            out_idx[p] += left_idx_stop[p]- left_idx[p];
            out_idx_save_stop[p] = out_idx[p];
        }
          
        idx_left = 0;
        idx_right = 0;
        #pragma _NEC ivdep                     
        #pragma cdir nodep        
        #pragma cdir on_adb(left_idx) 
        #pragma cdir on_adb(left_idx_stop)
        #pragma cdir on_adb(out_idx) 
        #pragma cdir on_adb(out_idx_save_p)  
        #pragma cdir on_adb(out_idx_save_stop_p)        
        for(size_t i = 0; i<num_vecAfterMerge;i+=2){
                left_idx[idx_left] = out_idx_save[i];
                left_idx_stop[idx_left] = out_idx_save_stop[i];
                out_idx[idx_left] = out_idx_save[i];
                idx_left ++;
        }
        #pragma _NEC ivdep                     
        #pragma cdir nodep        
        #pragma cdir on_adb(out_idx_save_p)  
        #pragma cdir on_adb(out_idx_save_stop_p) 
        #pragma cdir on_adb(right_idx_stop)        
        #pragma cdir on_adb(right_idx)            
        for(size_t i = 1; i<num_vecAfterMerge;i+=2){            
            right_idx[idx_right] = out_idx_save[i];
            right_idx_stop[idx_right] = out_idx_save_stop[i];
            idx_right++;
        }
    
        num_vecToMerge = num_vecAfterMerge;
        num_vecAfterMerge = std::ceil((double)num_vecToMerge/2); 
        #pragma cdir on_adb(buffer_merge_next_ptr) 
        #pragma cdir on_adb(buffer_merge_cur_ptr)   
        for(size_t i = 0;i < max_num_elem; i++){
            buffer_merge_cur_ptr[i] = buffer_merge_next_ptr[i];
        }
    
  }

    size_t num_elem = left_idx_stop[0] - left_idx[0];

    std::vector<T> ret_final(num_elem,0);
    auto* retp = &ret_final[0];
    #pragma cdir on_adb(buffer_merge_cur_ptr)
    #pragma cdir on_adb(retp)    
    for(size_t i=0; i<num_elem; i++){
        retp[i] = buffer_merge_cur_ptr[i];
    }

    return ret_final;
   
}

template <class T>
std::vector<T> set_union_2d_idx_lowd(std::vector< T >& inputVecs_buff,
        std::vector< size_t >& num_elem_per_inputVecs, 
        std::vector<size_t>& vec_idx, 
        size_t merge_th) {
    //check valid for all three stages
    //compare on valid in stage B
    
    auto* vec_idx_ptr = &vec_idx[0];
    size_t num_inputVecs = num_elem_per_inputVecs.size();
    auto* inputVecs_buff_ptr = &inputVecs_buff[0];
    auto* num_elem_per_inputVecs_ptr = &num_elem_per_inputVecs[0];
    size_t max_num_elem = 0;
    size_t check_valid_period = 50;
    size_t partition_factor = 5;
    max_num_elem = inputVecs_buff.size();
    std::vector<T> ret;
    if(max_num_elem == 0){
        return ret;
    }

    size_t num_mergeIters = 0;

    //Using only two buffers for merging
    std::vector< T > buffer_merge_next(max_num_elem,0);
    std::vector< T > buffer_merge_cur(max_num_elem,0);
    std::vector< T > buffer_merge_temp(max_num_elem,0);
    auto* buffer_merge_next_ptr = &buffer_merge_next[0];
    auto* buffer_merge_cur_ptr = &buffer_merge_cur[0];
    auto* buffer_merge_temp_ptr = &buffer_merge_temp[0];
    for(size_t i=0; i<max_num_elem;i++){
        buffer_merge_cur_ptr[i] = inputVecs_buff_ptr[i];
    }
       
    size_t num_vecToMerge = num_inputVecs; 
    size_t num_vecAfterMerge = std::ceil((double)num_vecToMerge/2);
   
    std::vector<size_t> left_idx(num_vecAfterMerge,0);
    std::vector<size_t> right_idx(num_vecAfterMerge,0);    
    std::vector<size_t> left_idx_stop(num_vecAfterMerge,0);
    std::vector<size_t> right_idx_stop(num_vecAfterMerge,0);
    std::vector<size_t> out_idx(num_vecAfterMerge,0);
    std::vector<size_t> out_idx_save(num_vecAfterMerge,0);
    std::vector<size_t> out_idx_save_stop(num_vecAfterMerge,0);
    std::vector<size_t> vec_idx_stop(num_inputVecs,0);

    vec_idx_stop[0] = num_elem_per_inputVecs_ptr[0];
            #pragma _NEC ivdep 
            #pragma cdir nodep
    for(size_t i=1; i<num_inputVecs-1; i++){
        vec_idx_stop[i] = vec_idx_ptr[i+1];
    }    
    vec_idx_stop[num_inputVecs-1] = max_num_elem;
    
    size_t idx_left = 0;
    size_t idx_right = 0;

  
    #pragma _NEC ivdep 
    #pragma cdir nodep
    #pragma cdir on_adb(vec_idx_ptr)
    #pragma cdir on_adb(vec_idx_stop)    
    #pragma cdir on_adb(left_idx)
    #pragma cdir on_adb(left_idx_stop)
    #pragma cdir on_adb(out_idx)     
    for(size_t i=0; i<num_inputVecs; i+=2){    //set pointers for left vectors
        left_idx[idx_left] = vec_idx_ptr[i];
        left_idx_stop[idx_left] = vec_idx_stop[i];
        out_idx[idx_left] = vec_idx_ptr[i];
        idx_left++;
    }
    #pragma _NEC ivdep 
    #pragma cdir nodep    
    #pragma cdir on_adb(vec_idx_ptr)
    #pragma cdir on_adb(vec_idx_stop)      
    #pragma cdir on_adb(right_idx)
    #pragma cdir on_adb(right_idx_stop)     
    for(size_t i=1; i<num_inputVecs; i+=2){    //set pointers for right vectors     
        right_idx[idx_right] = vec_idx_ptr[i];
        right_idx_stop[idx_right] = vec_idx_stop[i];
        idx_right++;            
    }

    while(num_vecToMerge > 1){ 

        num_mergeIters ++;
        #pragma _NEC ivdep 
        #pragma cdir nodep        
        #pragma cdir on_adb(out_idx) 
        #pragma cdir on_adb(out_idx_save) 
        for(size_t j = 0; j < num_vecAfterMerge; j++) {        
            out_idx_save[j] = out_idx[j];
        }
        
        size_t real_num_vecAfterMerge = num_vecAfterMerge - num_vecToMerge%2; //odd and even number of vectorToMerge 
        size_t upper_bound = merge_th;        
        size_t num_par_each = frovedis::ceil_div((size_t)upper_bound,real_num_vecAfterMerge);
        size_t num_par_total = num_par_each * real_num_vecAfterMerge;
        
        if(num_par_each == 1 || max_num_elem < upper_bound){

            std::vector<size_t> valid_vec_idx(real_num_vecAfterMerge,0);
            size_t valid_num_vec = real_num_vecAfterMerge; 
            size_t iter_idx = 0;
            while(1){    
                if(iter_idx%check_valid_period == 0){ //check valid periodically, not every time               
                    valid_num_vec = 0;
                        #pragma cdir on_adb(valid_vec_idx)
                        #pragma cdir on_adb(left_idx)
                        #pragma cdir on_adb(left_idx_stop)
                        #pragma cdir on_adb(right_idx)
                        #pragma cdir on_adb(right_idx_stop) 
                     for(size_t j = 0; j < real_num_vecAfterMerge; j++) {//Check which vectors are not finished yet and only compare those vectors.
                           if(left_idx[j] < left_idx_stop[j] && right_idx[j] < right_idx_stop[j]){
                               valid_vec_idx[valid_num_vec++] = j;
                           }
                     }
                }     
                 if(valid_num_vec < upper_bound){
                     break;
                 }          

                #pragma cdir nodep
                #pragma _NEC ivdep
                #pragma cdir on_adb(buffer_merge_next_ptr)
                #pragma cdir on_adb(buffer_merge_cur_ptr)
                #pragma cdir on_adb(left_idx)
                #pragma cdir on_adb(left_idx_stop)
                #pragma cdir on_adb(right_idx)
                #pragma cdir on_adb(right_idx_stop) 
                #pragma cdir on_adb(out_idx)                
                #pragma cdir on_adb(valid_vec_idx)
                    for(size_t j = 0; j < valid_num_vec; j++) {
                        if(left_idx[valid_vec_idx[j]] < left_idx_stop[valid_vec_idx[j]] 
                                && right_idx[valid_vec_idx[j]] < right_idx_stop[valid_vec_idx[j]]){                       
                            int eq = buffer_merge_cur_ptr[left_idx[valid_vec_idx[j]]]  == buffer_merge_cur_ptr[right_idx[valid_vec_idx[j]]];
                            int lt = buffer_merge_cur_ptr[left_idx[valid_vec_idx[j]]]  < buffer_merge_cur_ptr[right_idx[valid_vec_idx[j]]];


                           if(eq || lt) {
                             buffer_merge_next_ptr[out_idx[valid_vec_idx[j]]++] = buffer_merge_cur_ptr[left_idx[valid_vec_idx[j]]];
                             left_idx[valid_vec_idx[j]]++;
                           } 

                           else{
                             buffer_merge_next_ptr[out_idx[valid_vec_idx[j]]++] = buffer_merge_cur_ptr[right_idx[valid_vec_idx[j]]];
                             right_idx[valid_vec_idx[j]]++;
                           }

                           if(eq) {
                             right_idx[valid_vec_idx[j]]++;
                           }
                        }

                    }                
                    iter_idx++;
                }

  if(valid_num_vec > 0){

        num_par_each = frovedis::ceil_div((size_t)upper_bound*partition_factor,valid_num_vec);    
        num_par_total = num_par_each * valid_num_vec;

        size_t left_idx_each[num_par_total];
        size_t left_idx_stop_each[num_par_total];
        size_t right_idx_each[num_par_total];
        size_t right_idx_stop_each[num_par_total];        
        size_t out_idx_each[num_par_total];
        size_t out_idx_each_save[num_par_total];
        
        std::vector<size_t> valid_vec_idx_part(num_par_total,0);
        size_t num_valid_part = num_par_total;
 
        AdaptPartition_2D_valid(buffer_merge_cur, 
                valid_num_vec,
                num_par_each,
                valid_vec_idx,                    
                left_idx, 
                left_idx_stop, 
                right_idx, 
                right_idx_stop,
                left_idx_each, 
                left_idx_stop_each, 
                right_idx_each, 
                right_idx_stop_each);
      
            #pragma _NEC ivdep 
            #pragma cdir nodep  
                 #pragma cdir on_adb(left_idx_each)
                 #pragma cdir on_adb(out_idx_each) 
                for(size_t m=0; m<valid_num_vec; m++){ 
                   out_idx_each[m*num_par_each + 0] = left_idx_each[m*num_par_each + 0];
                }      
            #pragma _NEC ivdep 
            #pragma cdir nodep        
            for(size_t n=1; n<num_par_each; n++){
            #pragma _NEC ivdep                 
            #pragma cdir nodep                
                for(size_t m=0; m<valid_num_vec; m++){           //TODO: UNROLL 
                 #pragma cdir on_adb(out_idx_each) 
                 #pragma cdir on_adb(left_idx_each) 
                 #pragma cdir on_adb(right_idx_stop_each) 
                 #pragma cdir on_adb(right_idx_each) 
                 #pragma cdir on_adb(left_idx_stop_each) 
                 #pragma cdir on_adb(out_idx_each_save)                 
                    out_idx_each[m*num_par_each + n] = left_idx_stop_each[m*num_par_each + n-1] - left_idx_each[m*num_par_each + n-1] 
                            + right_idx_stop_each[m*num_par_each + n-1] - right_idx_each[m*num_par_each + n-1] 
                            + out_idx_each[m*num_par_each + n-1];
                }
            }
                 #pragma cdir nodep            
                #pragma _NEC ivdep 
                #pragma cdir on_adb(out_idx_each_save) 
                 #pragma cdir on_adb(out_idx_each)             
                for(size_t n=0; n<num_par_total; n++){
                    out_idx_each_save[n] = out_idx_each[n];
                }              
        iter_idx = 0;
        
        while(1){   


            if(iter_idx%check_valid_period == 0){ //check valid periodically, not every time              
                num_valid_part = 0;
                    #pragma cdir on_adb(valid_vec_idx)
                    #pragma cdir on_adb(left_idx)
                    #pragma cdir on_adb(left_idx_stop)
                    #pragma cdir on_adb(right_idx)
                    #pragma cdir on_adb(right_idx_stop) 
                 for(size_t j = 0; j < num_par_total; j++) {//Check which vectors are not finished yet and only compare those vectors.
                       if(left_idx_each[j] < left_idx_stop_each[j] && right_idx_each[j] < right_idx_stop_each[j]){
                           valid_vec_idx_part[num_valid_part++] = j;
                       }
                 }
            }     
            if(num_valid_part == 0){
                break;
            }


            #pragma _NEC ivdep 
            #pragma cdir nodep
            #pragma cdir on_adb(buffer_merge_temp_ptr)
            #pragma cdir on_adb(buffer_merge_cur_ptr)
            #pragma cdir on_adb(right_idx_each)
            #pragma cdir on_adb(right_idx_stop_each)
            #pragma cdir on_adb(left_idx_each)
            #pragma cdir on_adb(left_idx_stop_each) 
            #pragma cdir on_adb(out_idx_each)                
            #pragma cdir on_adb(isFinished)                            
            for(size_t j=0;j< num_valid_part;j++){
                if(left_idx_each[valid_vec_idx_part[j]] < left_idx_stop_each[valid_vec_idx_part[j]] 
                        && right_idx_each[valid_vec_idx_part[j]] < right_idx_stop_each[valid_vec_idx_part[j]]){
                    int eq = buffer_merge_cur_ptr[left_idx_each[valid_vec_idx_part[j]]]  == buffer_merge_cur_ptr[right_idx_each[valid_vec_idx_part[j]]];
                    int lt = buffer_merge_cur_ptr[left_idx_each[valid_vec_idx_part[j]]]  < buffer_merge_cur_ptr[right_idx_each[valid_vec_idx_part[j]]];


                    if(eq || lt) {
                      buffer_merge_temp_ptr[out_idx_each[valid_vec_idx_part[j]]++] = buffer_merge_cur_ptr[left_idx_each[valid_vec_idx_part[j]]];
                        left_idx_each[valid_vec_idx_part[j]]++;
                    } 

                    else{
                      buffer_merge_temp_ptr[out_idx_each[valid_vec_idx_part[j]]++] = buffer_merge_cur_ptr[right_idx_each[valid_vec_idx_part[j]]];
                      right_idx_each[valid_vec_idx_part[j]]++;
                    }

                    if(eq) {
                      right_idx_each[valid_vec_idx_part[j]]++;
                    }

                }                
            }
            iter_idx ++;
        }
        

        #pragma cdir nodep
          for(size_t j=0;j< num_par_total;j++){
              size_t pair_idx = valid_vec_idx[j/num_par_each];
              #pragma _NEC ivdep 
              #pragma cdir nodep
              #pragma cdir on_adb(buffer_merge_next_ptr)
              #pragma cdir on_adb(buffer_merge_temp_ptr) 
              #pragma cdir on_adb(out_idx_each)       
              #pragma cdir on_adb(out_idx_each_save)                 
              for(size_t k= 0 ;k< out_idx_each[j] - out_idx_each_save[j];k++){
                  buffer_merge_next_ptr[out_idx[pair_idx]+k] = buffer_merge_temp_ptr[out_idx_each_save[j] + k];              
              }

              out_idx[pair_idx] +=  out_idx_each[j] - out_idx_each_save[j];

              #pragma _NEC ivdep                     
              #pragma cdir nodep
              #pragma cdir on_adb(buffer_merge_next_ptr)
              #pragma cdir on_adb(buffer_merge_cur_ptr) 
              #pragma cdir on_adb(left_idx_each)       
              #pragma cdir on_adb(left_idx_stop_each)      
              for(size_t k= 0 ;k<left_idx_stop_each[j] - left_idx_each[j] ;k++){
                  buffer_merge_next_ptr[out_idx[pair_idx] +k] = buffer_merge_cur_ptr[left_idx_each[j] + k];              
              }

              out_idx[pair_idx] += left_idx_stop_each[j] - left_idx_each[j];

              #pragma _NEC ivdep                     
              #pragma cdir nodep
              #pragma cdir on_adb(buffer_merge_next_ptr)
              #pragma cdir on_adb(buffer_merge_cur_ptr) 
              #pragma cdir on_adb(right_idx_each)       
              #pragma cdir on_adb(right_idx_stop_each)                 
              for(size_t k= 0 ;k<right_idx_stop_each[j] - right_idx_each[j];k++){
                  buffer_merge_next_ptr[out_idx[pair_idx] +k] = buffer_merge_cur_ptr[right_idx_each[j] + k];              
              }
              out_idx[pair_idx] += right_idx_stop_each[j] - right_idx_each[j];
          }
              #pragma _NEC ivdep                     
              #pragma cdir nodep  
          for(size_t j = 0; j<valid_num_vec; j++){
              left_idx[valid_vec_idx[j]] = left_idx_stop[valid_vec_idx[j]];
              right_idx[valid_vec_idx[j]] = right_idx_stop[valid_vec_idx[j]];
              out_idx_save_stop[valid_vec_idx[j]] = out_idx[valid_vec_idx[j]];
          }
    }
   
  std::vector<size_t> num_leftover_left(real_num_vecAfterMerge,0);
  std::vector<size_t> num_leftover_right(real_num_vecAfterMerge,0);
  
                #pragma _NEC ivdep 
                #pragma cdir nodep    
                #pragma cdir on_adb(left_idx)
                #pragma cdir on_adb(left_idx_stop)
                #pragma cdir on_adb(right_idx)
                #pragma cdir on_adb(right_idx_stop)
                #pragma cdir on_adb(num_leftover_left)
                #pragma cdir on_adb(num_leftover_right)
            for(size_t j = 0; j < real_num_vecAfterMerge; j++) {
                num_leftover_left[j] = left_idx_stop[j] - left_idx[j];
                num_leftover_right[j] = right_idx_stop[j] - right_idx[j];
            } 
  
  
            #pragma _NEC ivdep 
            #pragma cdir nodep
            for(size_t j = 0; j < real_num_vecAfterMerge; j++) {             
            #pragma cdir nodep
            #pragma cdir on_adb(buffer_merge_next_ptr)
            #pragma cdir on_adb(buffer_merge_cur_ptr)  
                for(size_t k= 0 ;k<num_leftover_left[j];k++){
                    buffer_merge_next_ptr[out_idx[j] + k] =   buffer_merge_cur_ptr[left_idx[j] + k];              
                }

            #pragma cdir nodep
            #pragma cdir on_adb(buffer_merge_next_ptr)
            #pragma cdir on_adb(buffer_merge_cur_ptr)  
                for(size_t k= 0 ;k<num_leftover_right[j];k++){
                    buffer_merge_next_ptr[out_idx[j] + k] =   buffer_merge_cur_ptr[right_idx[j] + k];                              
                }
            }
            #pragma _NEC ivdep 
            #pragma cdir nodep  
            #pragma cdir on_adb(out_idx) 
            #pragma cdir on_adb(num_leftover_left)  
            for(size_t j = 0; j < real_num_vecAfterMerge; j++) {                
                out_idx[j] = out_idx[j] + num_leftover_left[j];
            }  
            #pragma _NEC ivdep 
            #pragma cdir nodep  
            #pragma cdir on_adb(out_idx) 
            #pragma cdir on_adb(num_leftover_right) 
            for(size_t j = 0; j < real_num_vecAfterMerge; j++) {                
                out_idx[j] = out_idx[j] + num_leftover_right[j];
            }
            #pragma _NEC ivdep 
            #pragma cdir nodep 
            #pragma cdir on_adb(out_idx)  
            #pragma cdir on_adb(out_idx_save_stop)   
            for(size_t j = 0; j < real_num_vecAfterMerge; j++) {
                out_idx_save_stop[j] = out_idx[j];
            }
        }
        else{ //num_vec_pair < UpperBound
            num_par_each = frovedis::ceil_div((size_t)upper_bound*partition_factor,real_num_vecAfterMerge);    
            num_par_total = num_par_each * real_num_vecAfterMerge;

            size_t left_idx_each[num_par_total];
            size_t left_idx_stop_each[num_par_total];
            size_t right_idx_each[num_par_total];
            size_t right_idx_stop_each[num_par_total];        
            size_t out_idx_each[num_par_total];
            size_t out_idx_each_save[num_par_total];

            AdaptPartition_2D(buffer_merge_cur, 
                    real_num_vecAfterMerge,
                    num_par_each,
                    left_idx, 
                    left_idx_stop, 
                    right_idx, 
                    right_idx_stop,
                    left_idx_each, 
                    left_idx_stop_each, 
                    right_idx_each, 
                    right_idx_stop_each);
        
            #pragma _NEC ivdep 
            #pragma cdir nodep  
            #pragma cdir on_adb(left_idx_each)
            #pragma cdir on_adb(out_idx_each) 
            for(size_t m=0; m<real_num_vecAfterMerge; m++){ 
               out_idx_each[m*num_par_each + 0] = left_idx_each[m*num_par_each + 0];

            }      
            #pragma _NEC ivdep 
            #pragma cdir nodep        
            for(size_t n=1; n<num_par_each; n++){
            #pragma _NEC ivdep                 
            #pragma cdir nodep                
                for(size_t m=0; m<real_num_vecAfterMerge; m++){           //TODO: UNROLL 
                 #pragma cdir on_adb(out_idx_each) 
                 #pragma cdir on_adb(left_idx_each) 
                 #pragma cdir on_adb(right_idx_stop_each) 
                 #pragma cdir on_adb(right_idx_each) 
                 #pragma cdir on_adb(left_idx_stop_each) 
                 #pragma cdir on_adb(out_idx_each_save)                 
                    out_idx_each[m*num_par_each + n] = left_idx_stop_each[m*num_par_each + n-1] - left_idx_each[m*num_par_each + n-1] 
                            + right_idx_stop_each[m*num_par_each + n-1] - right_idx_each[m*num_par_each + n-1] 
                            + out_idx_each[m*num_par_each + n-1];
                }
            }
            #pragma cdir nodep            
            #pragma _NEC ivdep 
            #pragma cdir on_adb(out_idx_each_save) 
            #pragma cdir on_adb(out_idx_each)             
            for(size_t n=0; n<num_par_total; n++){
               out_idx_each_save[n] = out_idx_each[n];
            }             
  
        size_t iter_idx = 0;
        size_t num_valid_part = num_par_total;
        std::vector<size_t> valid_vec_idx_part(num_par_total,0);
        while(1){
            if(iter_idx%check_valid_period == 0){ //check valid periodically, not every time                
                num_valid_part = 0;
                #pragma cdir on_adb(valid_vec_idx)
                #pragma cdir on_adb(left_idx)
                #pragma cdir on_adb(left_idx_stop)
                #pragma cdir on_adb(right_idx)
                #pragma cdir on_adb(right_idx_stop) 
                 for(size_t j = 0; j < num_par_total; j++) {//Check which vectors are not finished yet and only compare those vectors.
                       if(left_idx_each[j] < left_idx_stop_each[j] && right_idx_each[j] < right_idx_stop_each[j]){
                           valid_vec_idx_part[num_valid_part++] = j;
                       }
                 }
            }     
            if(num_valid_part == 0){
                break;
            }     

                #pragma _NEC ivdep 
                #pragma cdir nodep
                #pragma cdir on_adb(buffer_merge_temp_ptr)
                #pragma cdir on_adb(buffer_merge_cur_ptr)
                #pragma cdir on_adb(right_idx_each)
                #pragma cdir on_adb(right_idx_stop_each)
                #pragma cdir on_adb(left_idx_each)
                #pragma cdir on_adb(left_idx_stop_each) 
                #pragma cdir on_adb(out_idx_each)                
                #pragma cdir on_adb(isFinished)                            
                for(size_t j=0;j< num_valid_part;j++){
                    if(left_idx_each[valid_vec_idx_part[j]] < left_idx_stop_each[valid_vec_idx_part[j]] 
                            && right_idx_each[valid_vec_idx_part[j]] < right_idx_stop_each[valid_vec_idx_part[j]]){
                        int eq = buffer_merge_cur_ptr[left_idx_each[valid_vec_idx_part[j]]]  == buffer_merge_cur_ptr[right_idx_each[valid_vec_idx_part[j]]];
                        int lt = buffer_merge_cur_ptr[left_idx_each[valid_vec_idx_part[j]]]  < buffer_merge_cur_ptr[right_idx_each[valid_vec_idx_part[j]]];


                        if(eq || lt) {
                          buffer_merge_temp_ptr[out_idx_each[valid_vec_idx_part[j]]++] = buffer_merge_cur_ptr[left_idx_each[valid_vec_idx_part[j]]];
                            left_idx_each[valid_vec_idx_part[j]]++;

                        } 

                        else{
                            buffer_merge_temp_ptr[out_idx_each[valid_vec_idx_part[j]]++] = buffer_merge_cur_ptr[right_idx_each[valid_vec_idx_part[j]]];
                            right_idx_each[valid_vec_idx_part[j]]++;

                        }

                        if(eq) {
                          right_idx_each[valid_vec_idx_part[j]]++;
                        }

                    }                
                }
        }
   
        #pragma cdir nodep
        for(size_t j=0;j< num_par_total;j++){
            size_t pair_idx = j/num_par_each;


            #pragma _NEC ivdep 
            #pragma cdir nodep
            #pragma cdir on_adb(buffer_merge_next_ptr)
            #pragma cdir on_adb(buffer_merge_temp_ptr) 
            #pragma cdir on_adb(out_idx_each)       
            #pragma cdir on_adb(out_idx_each_save)                 
            for(size_t k= 0 ;k< out_idx_each[j] - out_idx_each_save[j];k++){
                buffer_merge_next_ptr[out_idx[pair_idx]+k] = buffer_merge_temp_ptr[out_idx_each_save[j] + k];              
            }

            out_idx[pair_idx] +=  out_idx_each[j] - out_idx_each_save[j];

            #pragma _NEC ivdep                     
            #pragma cdir nodep
            #pragma cdir on_adb(buffer_merge_next_ptr)
            #pragma cdir on_adb(buffer_merge_cur_ptr) 
            #pragma cdir on_adb(left_idx_each)       
            #pragma cdir on_adb(left_idx_stop_each)      
            for(size_t k= 0 ;k<left_idx_stop_each[j] - left_idx_each[j] ;k++){
                buffer_merge_next_ptr[out_idx[pair_idx] +k] = buffer_merge_cur_ptr[left_idx_each[j] + k];              
            }

            out_idx[pair_idx] += left_idx_stop_each[j] - left_idx_each[j];

            #pragma _NEC ivdep                     
            #pragma cdir nodep
            #pragma cdir on_adb(buffer_merge_next_ptr)
            #pragma cdir on_adb(buffer_merge_cur_ptr) 
            #pragma cdir on_adb(right_idx_each)       
            #pragma cdir on_adb(right_idx_stop_each)                 
            for(size_t k= 0 ;k<right_idx_stop_each[j] - right_idx_each[j];k++){
                buffer_merge_next_ptr[out_idx[pair_idx] +k] = buffer_merge_cur_ptr[right_idx_each[j] + k];              
            }
            out_idx[pair_idx] += right_idx_stop_each[j] - right_idx_each[j];
        }
        #pragma _NEC ivdep                     
        #pragma cdir nodep  
        for(size_t j = 0; j<real_num_vecAfterMerge; j++){
            out_idx_save_stop[j] = out_idx[j];
        }
        }
  
        for(size_t p=real_num_vecAfterMerge; p<num_vecAfterMerge; p++){//copy the last vector to buffer_next if it is leftover (odd)
            #pragma _NEC ivdep                     
            #pragma cdir nodep            
            for(size_t k= 0 ; k<left_idx_stop[p] - left_idx[p]; k++){ //copy the last vector to the buffer_merge_next
                buffer_merge_next_ptr[out_idx[p]+k] = buffer_merge_cur_ptr[left_idx[p] + k];
            }
            out_idx[p] += left_idx_stop[p]- left_idx[p];
            out_idx_save_stop[p] = out_idx[p];
        }
          
        idx_left = 0;
        idx_right = 0;
        #pragma _NEC ivdep                     
        #pragma cdir nodep        
        #pragma cdir on_adb(left_idx) 
        #pragma cdir on_adb(left_idx_stop)
        #pragma cdir on_adb(out_idx) 
        #pragma cdir on_adb(out_idx_save_p)  
        #pragma cdir on_adb(out_idx_save_stop_p)        
        for(size_t i = 0; i<num_vecAfterMerge;i+=2){
            left_idx[idx_left] = out_idx_save[i];
            left_idx_stop[idx_left] = out_idx_save_stop[i];
            out_idx[idx_left] = out_idx_save[i];
            idx_left ++;
        }
            #pragma _NEC ivdep                     
            #pragma cdir nodep        
            #pragma cdir on_adb(out_idx_save_p)  
            #pragma cdir on_adb(out_idx_save_stop_p) 
            #pragma cdir on_adb(right_idx_stop)        
            #pragma cdir on_adb(right_idx)            
        for(size_t i = 1; i<num_vecAfterMerge;i+=2){            
            right_idx[idx_right] = out_idx_save[i];
            right_idx_stop[idx_right] = out_idx_save_stop[i];
            idx_right++;
        }
    
        num_vecToMerge = num_vecAfterMerge;
        num_vecAfterMerge = std::ceil((double)num_vecToMerge/2); 
        #pragma cdir on_adb(buffer_merge_next_ptr) 
        #pragma cdir on_adb(buffer_merge_cur_ptr)   
        for(size_t i = 0;i < max_num_elem; i++){
            buffer_merge_cur_ptr[i] = buffer_merge_next_ptr[i];
        }
    
    } 
    size_t num_elem = left_idx_stop[0] - left_idx[0];

    std::vector<T> ret_final(num_elem,0);
    auto* retp = &ret_final[0];
    #pragma cdir on_adb(buffer_merge_cur_ptr)
    #pragma cdir on_adb(retp)    
    for(size_t i=0; i<num_elem; i++){
        retp[i] = buffer_merge_cur_ptr[i];
    }

    return ret_final;
   
}

template <class T>
sparse_vector_tri<T> set_union_horizontal_relax(sparse_vector<T>& inputVecs_sp, 
                                                std::vector<size_t>& vec_start,
                                                std::vector<size_t>& vpred, 
                                                size_t merge_th) {//for sssp Bellman-Ford
    //Using parallel merging to merge multiple sorted short vectors
    //Optimized for vector processors
    auto* vpredp = &vpred[0];
    size_t num_inputVecs = vec_start.size();
    auto* vec_start_ptr = &vec_start[0];
    auto* inputVecs_idx_ptr = &inputVecs_sp.idx[0];
    auto* inputVecs_val_ptr = &inputVecs_sp.val[0];
    size_t max_num_elem = inputVecs_sp.idx.size();
    size_t num_mergeIters = 0;

    sparse_vector_tri< T > buffer_merge_next(max_num_elem);
    auto* buffer_merge_next_idx_ptr = &buffer_merge_next.idx[0];
    auto* buffer_merge_next_val_ptr = &buffer_merge_next.val[0];
    auto* buffer_merge_next_pred_ptr = &buffer_merge_next.pred[0];
   
    sparse_vector_tri< T > buffer_merge_cur(max_num_elem);
    auto* buffer_merge_cur_idx_ptr = &buffer_merge_cur.idx[0];
    auto* buffer_merge_cur_val_ptr = &buffer_merge_cur.val[0];    
    auto* buffer_merge_cur_pred_ptr = &buffer_merge_cur.pred[0];


    size_t num_vecToMerge = num_inputVecs; 
    size_t num_vecAfterMerge = std::ceil((double)num_vecToMerge/2);
   
    size_t left_idx[num_vecAfterMerge];
    size_t right_idx[num_vecAfterMerge];
    size_t left_idx_stop[num_vecAfterMerge];
    size_t right_idx_stop[num_vecAfterMerge];
    size_t out_idx[num_vecAfterMerge];
    size_t out_idx_save[num_vecAfterMerge];
    size_t out_idx_save_stop[num_vecAfterMerge];

    size_t vec_idx[num_inputVecs];
    size_t vec_idx_stop[num_inputVecs];
    vec_idx[0] = 0;
#pragma cdir nodep   
        #pragma cdir on_adb(vec_idx)
        #pragma cdir on_adb(vec_idx_stop)
        #pragma cdir on_adb(vec_start_ptr) 
    for(size_t i=0; i<num_inputVecs-1; i++){
        vec_idx[i] = vec_start_ptr[i];
        vec_idx_stop[i] = vec_start_ptr[i+1];
    }    
    vec_idx[num_inputVecs-1] =  vec_start_ptr[num_inputVecs-1];
    vec_idx_stop[num_inputVecs-1] = max_num_elem;
    
    size_t idx_left = 0;
    size_t idx_right = 0;
    #pragma cdir nodep     
    #pragma cdir on_adb(buffer_merge_cur_idx_ptr)
    #pragma cdir on_adb(buffer_merge_cur_val_ptr)
    #pragma cdir on_adb(inputVecs_idx_ptr)
    #pragma cdir on_adb(inputVecs_val_ptr)
    for(size_t i=0; i<max_num_elem; i++){    //copy input vector into buffer
        buffer_merge_cur_idx_ptr[i] = inputVecs_idx_ptr[i];
        buffer_merge_cur_val_ptr[i] = inputVecs_val_ptr[i]; 
    } 
    #pragma cdir nodep
    #pragma cdir on_adb(buffer_merge_cur_pred_ptr)
    #pragma cdir on_adb(vpredp)
    #pragma cdir on_adb(vec_idx)
    #pragma cdir on_adb(vec_idx_stop)
    for(size_t i=0; i<num_inputVecs; i++){    //copy pred into buffer
        #pragma cdir nodep
        for(size_t j=0;j<vec_idx_stop[i]-vec_idx[i];j++){
            buffer_merge_cur_pred_ptr[vec_idx[i] + j] = vpredp[i];
        }
    } 
  
    #pragma cdir nodep 
    #pragma cdir on_adb(vec_idx)
    #pragma cdir on_adb(vec_idx_stop)
    #pragma cdir on_adb(left_idx)
    #pragma cdir on_adb(left_idx_stop)
    #pragma cdir on_adb(right_idx)
    #pragma cdir on_adb(right_idx_stop) 
    #pragma cdir on_adb(out_idx) 
    #pragma cdir on_adb(out_idx_save) 
    #pragma cdir on_adb(out_idx_save_stop)   
    for(size_t i=0; i<num_inputVecs; i++){    //set pointers
       if(i%2==0){
            left_idx[idx_left] = vec_idx[i];
            out_idx_save[idx_left] = vec_idx[i];
            left_idx_stop[idx_left] = vec_idx_stop[i];
            out_idx_save_stop[idx_left] = vec_idx_stop[i];
            out_idx[idx_left] = vec_idx[i];
            idx_left++;
       }
       else{
           right_idx[idx_right] = vec_idx[i];
           right_idx_stop[idx_right] = vec_idx_stop[i];
           idx_right++;            
       }   
    } 

    while(num_vecToMerge > 1){ 
        num_mergeIters ++;
        if(num_vecToMerge <= merge_th){
            break; //vector length longer than num of vectors, use sequential merging
        }
        int isFinished[num_vecAfterMerge];
        bool isFinished_left = false;
        bool isFinished_right = false;  
                    
        for(size_t i=0;i<num_vecAfterMerge;i++){
            isFinished[i] = false;
        }
        
        for(size_t j = 0; j < num_vecAfterMerge; j++) {        
            out_idx_save[j] = out_idx[j];
        }
        
        size_t real_num_vecAfterMerge = num_vecAfterMerge - num_vecToMerge%2; //odd and even number of vectorToMerge 
        while(1){
                bool all_finished = true;
                for(size_t m = 0; m<real_num_vecAfterMerge; m++){
                    if(isFinished[m] == false){
                        all_finished = false;
                        break;
                    }
                }
                if(all_finished == true){
                    break;
                }

                #pragma cdir nodep
                #pragma _NEC ivdep  
                #pragma cdir on_adb(buffer_merge_next_idx_ptr)  
                #pragma cdir on_adb(buffer_merge_next_val_ptr) 
                #pragma cdir on_adb(buffer_merge_next_pred_ptr)      
                #pragma cdir on_adb(buffer_merge_cur_idx_ptr)  
                #pragma cdir on_adb(buffer_merge_cur_val_ptr) 
                #pragma cdir on_adb(buffer_merge_cur_pred_ptr) 
                #pragma cdir on_adb(left_idx)
                #pragma cdir on_adb(left_idx_stop)
                #pragma cdir on_adb(right_idx)
                #pragma cdir on_adb(right_idx_stop) 
                #pragma cdir on_adb(out_idx) 
                for(size_t j = 0; j < real_num_vecAfterMerge; j++) {
                    isFinished_left = left_idx[j] >= left_idx_stop[j];
                    isFinished_right = right_idx[j] >= right_idx_stop[j];  
                    
                    if((isFinished_left) && (isFinished_right)) {
                        isFinished[j] = true;
                        out_idx_save_stop[j] = out_idx[j];
                    }  
                    
                    if(isFinished[j] == false) {

                        bool idx_lt = buffer_merge_cur_idx_ptr[left_idx[j]]  < buffer_merge_cur_idx_ptr[right_idx[j]];
                        bool idx_gt = buffer_merge_cur_idx_ptr[left_idx[j]]  > buffer_merge_cur_idx_ptr[right_idx[j]];
                        bool eq = buffer_merge_cur_idx_ptr[left_idx[j]] == buffer_merge_cur_idx_ptr[right_idx[j]];
                        
                        
                        if(idx_lt && !isFinished_left && !isFinished_left) {
                          buffer_merge_next_idx_ptr[out_idx[j]] = buffer_merge_cur_idx_ptr[left_idx[j]];
                          buffer_merge_next_val_ptr[out_idx[j]] = buffer_merge_cur_val_ptr[left_idx[j]];
                          buffer_merge_next_pred_ptr[out_idx[j]] = buffer_merge_cur_pred_ptr[left_idx[j]];
                          out_idx[j]++;
                          left_idx[j]++;
                        } 

                        else if(idx_gt && !isFinished_left && !isFinished_right){
                          buffer_merge_next_idx_ptr[out_idx[j]] = buffer_merge_cur_idx_ptr[right_idx[j]];
                          buffer_merge_next_val_ptr[out_idx[j]] = buffer_merge_cur_val_ptr[right_idx[j]];
                          buffer_merge_next_pred_ptr[out_idx[j]] = buffer_merge_cur_pred_ptr[right_idx[j]];                          
                          out_idx[j]++;
                          right_idx[j]++;
                        }
                        else if(isFinished_left && !isFinished_right){
                          buffer_merge_next_idx_ptr[out_idx[j]] = buffer_merge_cur_idx_ptr[right_idx[j]];
                          buffer_merge_next_val_ptr[out_idx[j]] = buffer_merge_cur_val_ptr[right_idx[j]];
                          buffer_merge_next_pred_ptr[out_idx[j]] = buffer_merge_cur_pred_ptr[right_idx[j]];                          
                          out_idx[j]++;
                          right_idx[j]++;                            
                        }
                        else if(!isFinished_left && isFinished_right){
                            buffer_merge_next_idx_ptr[out_idx[j]] = buffer_merge_cur_idx_ptr[left_idx[j]];
                            buffer_merge_next_val_ptr[out_idx[j]] = buffer_merge_cur_val_ptr[left_idx[j]];
                            buffer_merge_next_pred_ptr[out_idx[j]] = buffer_merge_cur_pred_ptr[left_idx[j]];
                            out_idx[j]++;
                            left_idx[j]++; 
                        }
                        
                        else if(eq && !isFinished_right && !isFinished_left){ // same idx, should compare the val
                           auto  val_lt = buffer_merge_cur_val_ptr[left_idx[j]] < buffer_merge_cur_val_ptr[right_idx[j]]; 
                           auto  val_eq = buffer_merge_cur_val_ptr[left_idx[j]] == buffer_merge_cur_val_ptr[right_idx[j]];
                           if(val_lt || val_eq){
                               buffer_merge_next_idx_ptr[out_idx[j]] = buffer_merge_cur_idx_ptr[left_idx[j]];
                               buffer_merge_next_val_ptr[out_idx[j]] = buffer_merge_cur_val_ptr[left_idx[j]];
                               buffer_merge_next_pred_ptr[out_idx[j]] = buffer_merge_cur_pred_ptr[left_idx[j]];                              
                           }
                           else{
                                buffer_merge_next_idx_ptr[out_idx[j]] = buffer_merge_cur_idx_ptr[right_idx[j]]; 
                                buffer_merge_next_val_ptr[out_idx[j]] = buffer_merge_cur_val_ptr[right_idx[j]]; 
                                buffer_merge_next_pred_ptr[out_idx[j]] = buffer_merge_cur_pred_ptr[right_idx[j]];                                
                           }
                           out_idx[j]++; 
                           left_idx[j]++;
                           right_idx[j]++;
                        }
                    }             
                }
            }
  
        if(num_vecToMerge%2 == 1){//even num of vectors to merge
            
            for(size_t k=left_idx[num_vecAfterMerge-1]; k<left_idx_stop[num_vecAfterMerge-1]; k++){ //copy the last vector to the buffer_merge_next
                buffer_merge_next_idx_ptr[out_idx[num_vecAfterMerge-1]] = buffer_merge_cur_idx_ptr[k];
                buffer_merge_next_val_ptr[out_idx[num_vecAfterMerge-1]] = buffer_merge_cur_val_ptr[k];
                buffer_merge_next_pred_ptr[out_idx[num_vecAfterMerge-1]] = buffer_merge_cur_pred_ptr[k];
                out_idx[num_vecAfterMerge-1]++;
            }
            out_idx_save_stop[num_vecAfterMerge-1] = out_idx[num_vecAfterMerge-1];
            isFinished[num_vecAfterMerge-1] = true;
        }
        

        idx_left = 0;
        idx_right = 0;
        #pragma cdir on_adb(left_idx)
        #pragma cdir on_adb(left_idx_stop)
        #pragma cdir on_adb(right_idx)
        #pragma cdir on_adb(right_idx_stop) 
        #pragma cdir on_adb(out_idx) 
        #pragma cdir on_adb(out_idx_save) 
        #pragma cdir on_adb(out_idx_save_stop) 
        for(size_t i = 0; i<num_vecAfterMerge;i++){
            if(i%2 ==0){
                left_idx[idx_left] = out_idx_save[i];
                left_idx_stop[idx_left] = out_idx_save_stop[i];
                out_idx[idx_left] = out_idx_save[i];
                idx_left ++;
            }
            else{
                right_idx[idx_right] = out_idx_save[i];
                right_idx_stop[idx_right] = out_idx_save_stop[i];
                idx_right++;
            }
        }
         
        num_vecToMerge = num_vecAfterMerge;
        num_vecAfterMerge = std::ceil((double)num_vecToMerge/2); 

        buffer_merge_cur = buffer_merge_next;
    }
    size_t num_elem = out_idx_save_stop[0] - out_idx_save[0];
    sparse_vector_tri<T> ret;
    ret.pred.resize(num_elem);    
    ret.val.resize(num_elem);
    ret.idx.resize(num_elem);
    
    auto* ret_idxp = &ret.idx[0];
    auto* ret_valp = &ret.val[0];
    auto* ret_predp = &ret.pred[0];
    
    
    #pragma cdir on_adb(ret_idxp)
    #pragma cdir on_adb(ret_valp)
    #pragma cdir on_adb(ret_predp)
    #pragma cdir on_adb(buffer_merge_cur_idx_ptr)  
    #pragma cdir on_adb(buffer_merge_cur_val_ptr) 
    #pragma cdir on_adb(buffer_merge_cur_pred_ptr)    
    for(size_t i=0; i<num_elem; i++){
        ret_idxp[i] = buffer_merge_cur_idx_ptr[i];
        ret_valp[i] = buffer_merge_cur_val_ptr[i];
        ret_predp[i] = buffer_merge_cur_pred_ptr[i];
    }
    return ret;
   
}

}
#endif	/* GRAPH_SET_UNION_MULTIVEC_HPP*/

