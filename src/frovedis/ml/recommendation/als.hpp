#ifndef _ALS_H_
#define _ALS_H_

#include <iostream>
#include <string>

#include "../../matrix/crs_matrix.hpp"
#include "../metrics.hpp"
#include "matrix_factorization_model.hpp"
#include "optimizer.hpp"

//#define ALS_DLOG

namespace frovedis {

class matrix_factorization_using_als {
public:
  template <class T, class I, class O>
  static matrix_factorization_model<T> 
  train (crs_matrix<T,I,O>& data, 
         size_t factor, 
         int numIter = 100, 
         double alpha = 0.01, 
         double regParam = 0.01, 
         size_t seed = 0,
         double similarity_factor = 0.1);
};

template <class T, class I, class O>
std::vector<T>
dtrain(crs_matrix_local<T,I,O>& data, 
       std::vector<T>& modelMat,
       optimizer& alsOPT) {
  return alsOPT.optimize(data, modelMat);
}

template <class T, class I, class O>
std::vector<T>
dtrain_partial(crs_matrix_local<T,I,O>& data, 
               std::vector<T>& modelMat,
               optimizer& alsOPT,
               std::vector<I>& required_rowids) {
  return alsOPT.optimize(data, modelMat, required_rowids);
}

template <class T, class I, class O>
std::vector<I> 
group_als_identical_rows(crs_matrix_local<T,I,O>& mat, 
                     std::vector<size_t>& sep) {
  size_t nrows = mat.local_num_row; 
  auto off_vtr = mat.off;  
  auto idxv = mat.idx;
  auto valp = mat.val;  
  
  I MAX = std::numeric_limits<I>::max(); // Used as a flag  
  std::vector<I> s(nrows);
  auto sz = s.size();
  auto s_ptr = s.data();  
  for(size_t i = 0; i < sz; ++i) s_ptr[i] = MAX;
    
  for(I current_row = 0; current_row < nrows; current_row++) {  
    // ignore if already updated
    if(s_ptr[current_row] != MAX) continue; 
    size_t current_width = off_vtr[current_row + 1] - off_vtr[current_row];    
    for(I other_row = current_row + 1; 
          other_row < nrows; 
          other_row++) {  
      if(s_ptr[other_row] == MAX) { // proceed if not set 
        size_t other_width = off_vtr[other_row + 1] - off_vtr[other_row];  
        if(current_width == other_width) {    
          const I* current_pos = &idxv[off_vtr[current_row]];
          const T* current_val = &valp[off_vtr[current_row]];  
          const I* other_pos = &idxv[off_vtr[other_row]];
          const T* other_val = &valp[off_vtr[other_row]];  
          bool matched = true;
          for(size_t i = 0; i < current_width; i++) {  
            if((current_pos[i] != other_pos[i]) || (current_val[i] != other_val[i])) { 
              matched = false; 
              break;
            }            
          }
          if(matched)
            s_ptr[current_row] = s_ptr[other_row] = current_row;          
        }      
      }
    }
    //If no match is found, update with same row index
    if(s_ptr[current_row] == MAX) s_ptr[current_row] = current_row;  
  }
    
  std::vector<I> s_pos;  
  s = vector_sort(s, s_pos);  
  sep = set_separate(s);
  return s_pos;  
      
}

template <class T, class I, class O>
std::vector<I> 
group_als_identical_rows(crs_matrix<T,I,O>& mat, 
                     std::vector<size_t>& sep) {
  auto lmat = mat.gather();
  return group_als_identical_rows(lmat, sep);
}    
    
template <class I>
int check_similarity(I x, I y, 
                     double similarity_factor,
                     const std::string& target ) {
  auto frac = 1.0 - (static_cast<double>(x) / static_cast<double>(y));
    
  if (frac >= similarity_factor) {
    RLOG(INFO) << frac * 100.0 << "% of " << target 
               << " data sample is found to be identical!\n";
    RLOG(INFO) << "computation for identical " << target
               << " sample would be reused for performance speed-up!\n";
    return true;
  }
  else return false;
}

template <class I>
std::vector<I>
get_unique_row_ids(const std::vector<I>& group_ids,
                   const std::vector<size_t>& sep) {
  auto group_size = sep.size() - 1;
  std::vector<I> ret(group_size);
  auto rptr = ret.data();
  auto sep_ptr = sep.data();
  auto gptr = group_ids.data();
  for(size_t i = 0; i < group_size; ++i) rptr[i] = gptr[sep_ptr[i]];
  return ret;
}

template <class T, class I>
void copy_impl(std::vector<T>& model, 
               const std::vector<I>& group_ids,
               const std::vector<size_t>& sep,
               size_t factor) {
  auto gptr = group_ids.data();
  auto sep_ptr = sep.data();
  auto group_size = sep.size() - 1;

  for(size_t i = 0; i < group_size; ++i) {  
    const T* src = &model[gptr[sep_ptr[i]] * factor];
    for(size_t j = sep_ptr[i] + 1; j < sep_ptr[i+1]; ++j) {
      T* dst = &model[gptr[j]*factor];
      for(size_t k = 0; k < factor; ++k) dst[k] = src[k];  
    } 
  }   
}    
    
template <class T, class I, class O>
void als_train_11(crs_matrix<T,I,O>& data, 
                  crs_matrix<T,I,O>& transData,
                  matrix_factorization_model<T>& initModel,
                  node_local<optimizer>& distOPT,
                  const std::vector<I>& user_group_ids,
                  const std::vector<size_t>& sep_user, 
                  lvec<I>& required_X_ids,
                  const std::vector<I>& item_group_ids,
                  const std::vector<size_t>& sep_item,
                  lvec<I>& required_Y_ids,
                  int numIter) {  
  RLOG(DEBUG) << "als version: als_train_11\n";
    
  size_t factor = initModel.factor;
    
  frovedis::time_spent t(DEBUG);
  for (int i = 1; i <= numIter; i++) { 
    initModel.X = data.data.map(dtrain_partial<T,I,O>, 
                            broadcast(initModel.Y),
                            distOPT, required_X_ids)
                       .template moveto_dvector<T>().gather();
    //Copy rows of X  
    copy_impl(initModel.X, user_group_ids, sep_user, factor);
    initModel.Y = transData.data.map(dtrain_partial<T,I,O>, 
                                 broadcast(initModel.X), 
                                 distOPT, required_Y_ids)
                            .template moveto_dvector<T>().gather();
    //Copy rows of Y    
    copy_impl(initModel.Y, item_group_ids, sep_item, factor);
      
    t.show("one iteration: ");
  }
}

template <class T, class I, class O>
void als_train_10(crs_matrix<T,I,O>& data,
                  crs_matrix<T,I,O>& transData,
                  matrix_factorization_model<T>& initModel,
                  node_local<optimizer>& distOPT,
                  const std::vector<I>& user_group_ids,
                  const std::vector<size_t>& sep_user,
                  lvec<I>& required_X_ids,
                  int numIter) { 
  RLOG(DEBUG) << "als version: als_train_10\n";
    
  size_t factor = initModel.factor;
    
  frovedis::time_spent t(DEBUG);
  for (int i = 1; i <= numIter; i++) {
    initModel.X = data.data.map(dtrain_partial<T,I,O>,
                            broadcast(initModel.Y),
                            distOPT, required_X_ids)
                       .template moveto_dvector<T>().gather();
      
    //Copy rows of X
    copy_impl(initModel.X, user_group_ids, sep_user, factor);
      
    initModel.Y = transData.data.map(dtrain<T,I,O>, 
                                     broadcast(initModel.X), distOPT)
                                .template moveto_dvector<T>().gather();
    t.show("one iteration: ");
  }
}

template <class T, class I, class O>
void als_train_01(crs_matrix<T,I,O>& data,
                  crs_matrix<T,I,O>& transData,
                  matrix_factorization_model<T>& initModel,
                  node_local<optimizer>& distOPT,
                  const std::vector<I>& item_group_ids,
                  const std::vector<size_t>& sep_item, 
                  lvec<I>& required_Y_ids,
                  int numIter) { 
  RLOG(DEBUG) << "als version: als_train_01\n";
         
  size_t factor = initModel.factor;
    
  frovedis::time_spent t(DEBUG);
  for (int i = 1; i <= numIter; i++) {
    initModel.X = data.data.map(dtrain<T,I,O>, 
                                broadcast(initModel.Y), distOPT)
                           .template moveto_dvector<T>().gather();
    initModel.Y = transData.data.map(dtrain_partial<T,I,O>, 
                                 broadcast(initModel.X), 
                                 distOPT, required_Y_ids)
                            .template moveto_dvector<T>().gather();

    //Copy rows of Y
    copy_impl(initModel.Y, item_group_ids, sep_item, factor);
      
    t.show("one iteration: ");
  }
}

// same as original implementation -> similarity_factor 1.0
template <class T, class I, class O>
void als_train_00(crs_matrix<T,I,O>& data,
                  crs_matrix<T,I,O>& transData,
                  matrix_factorization_model<T>& initModel,
                  node_local<optimizer>& distOPT,
                  int numIter) {  
  RLOG(DEBUG) << "als version: als_train_00\n";
  frovedis::time_spent t(DEBUG);
  for (int i = 1; i <= numIter; i++) {
    initModel.X = data.data.map(dtrain<T,I,O>,
                                broadcast(initModel.Y), distOPT)
                           .template moveto_dvector<T>().gather();
    initModel.Y = transData.data.map(dtrain<T,I,O>, 
                                     broadcast(initModel.X), distOPT)
                                .template moveto_dvector<T>().gather();
    t.show("one iteration: ");
  }
}

template <class T, class I, class O>
matrix_factorization_model<T>
matrix_factorization_using_als::train(crs_matrix<T,I,O>& data,
                                      size_t factor,  
                                      int numIter,
                                      double alpha,
                                      double regParam,
                                      size_t seed,
                                      double similarity_factor) { 
  checkAssumption(numIter > 0 && alpha > 0.0 && regParam > 0.0);
  require(similarity_factor >= 0.0 && similarity_factor <= 1.0,
  "similarity_factor must be in between 0.0 and 1.0\n");

  size_t num_user = data.num_row;
  size_t num_item = data.num_col;
  auto transData = data.transpose();

  matrix_factorization_model<T> initModel(num_user, num_item, factor, seed);
  optimizer alsOpt(factor, alpha, regParam);
  auto distOPT = frovedis::make_node_local_broadcast(alsOpt);

  frovedis::time_spent t(DEBUG);
  lvec<I> required_X_ids, required_Y_ids;
  std::vector<I> user_group_ids, item_group_ids;
  std::vector<size_t> sep_user, sep_item;
  int reuse_X = false, reuse_Y = false;
    
  if (similarity_factor != 1.0) { // if similatrity check is required
    user_group_ids = group_als_identical_rows(data, sep_user);
    item_group_ids = group_als_identical_rows(transData, sep_item);
    auto unq_user = sep_user.size() - 1;
    auto unq_item = sep_item.size() - 1;
    reuse_X = check_similarity(unq_user, num_user, similarity_factor, "user");
    reuse_Y = check_similarity(unq_item, num_item, similarity_factor, "item");
    if(reuse_X) {
      auto ids = get_unique_row_ids(user_group_ids, sep_user);
      required_X_ids = align_global_index(data, ids); // defined in rowmajor_matrix.hpp
    }
    if(reuse_Y) {
      auto ids = get_unique_row_ids(item_group_ids, sep_item);
      required_Y_ids = align_global_index(transData, ids);
    }
    t.show("group construction: ");
  }

  if (reuse_X && reuse_Y) als_train_11(data, transData, initModel, distOPT, 
                                       user_group_ids, sep_user,
                                       required_X_ids,
                                       item_group_ids, sep_item,
                                       required_Y_ids,
                                       numIter);
  else if (reuse_X && !reuse_Y) als_train_10(data, transData, initModel, distOPT,
                                             user_group_ids, sep_user,
                                             required_X_ids, 
                                             numIter);
  else if (!reuse_X && reuse_Y) als_train_01(data, transData, initModel, distOPT,
                                             item_group_ids, sep_item,
                                             required_Y_ids, 
                                             numIter);
  else als_train_00(data, transData, initModel, distOPT, numIter);

  t.show("all iteration: ");
  return initModel;
}

}
#endif
