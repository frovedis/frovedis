#ifndef _COMMON_HPP_
#define _COMMON_HPP_

#include "../../core/exceptions.hpp"
#include "linear_model.hpp"
#include "gradient_descent.hpp"
#include "gradient.hpp"
#include "regularizer.hpp"
#include "rms.hpp"
#include "../utility/matrix_conversion.hpp"

#include <boost/lexical_cast.hpp>

#define ITOS boost::lexical_cast<std::string>
#define _ALLOW_CONV_RATE_CHECK_
#define LR_VLEN 1024 // made it same as JDS/ELL

namespace frovedis {

enum MatType {
  CRS = 0,
  JDS,
  ELL,
  HYBRID
};

template <class T>
std::vector<crs_matrix_local<T>>
divide_data_to_minibatch(crs_matrix_local<T>& data,
                         double miniBatchFraction) {
  size_t numSamples = data.local_num_row;
  size_t tmp = static_cast<size_t>(ceil(numSamples * miniBatchFraction));
  size_t tmp2 = ceil_div<size_t>(tmp, LR_VLEN) * LR_VLEN;
  size_t miniBatchSize = tmp2 < numSamples ? tmp2 : numSamples;
  size_t numBatches = ceil_div(numSamples, miniBatchSize);
  std::vector<crs_matrix_local<T>> ret(numBatches);
  T* datavalp = &data.val[0];
  size_t* dataidxp = &data.idx[0];
  size_t* dataoffp = &data.off[0];
  for(size_t i = 0; i < numBatches; i++) {
    ret[i].local_num_col = data.local_num_col;
    size_t start_row = i * miniBatchSize;
    size_t end_row =  (i + 1) * miniBatchSize;
    if(end_row > data.local_num_row) end_row = data.local_num_row;
    ret[i].local_num_row = end_row - start_row;
    size_t start_off = dataoffp[start_row];
    size_t end_off = dataoffp[end_row];
    size_t off_size = end_off - start_off;
    ret[i].val.resize(off_size);
    ret[i].idx.resize(off_size);
    ret[i].off.resize(end_row - start_row + 1); // off[0] == 0 by ctor
    T* valp = &ret[i].val[0];
    size_t* idxp = &ret[i].idx[0];
    size_t* offp = &ret[i].off[0];
    for(size_t j = 0; j < off_size; j++) {
      valp[j] = datavalp[j + start_off];
      idxp[j] = dataidxp[j + start_off];
    }
    for(size_t j = 0; j < end_row - start_row; j++) {
      offp[j+1] = offp[j] + (dataoffp[start_row + j + 1] -
                             dataoffp[start_row + j]);
    }
  }
  return ret;
}

template <class T>
std::vector<std::vector<T>>
divide_label_to_minibatch(std::vector<T>& label,
                          double miniBatchFraction) {
  size_t numSamples = label.size();
  size_t tmp = static_cast<size_t>(ceil(numSamples * miniBatchFraction));
  size_t tmp2 = ceil_div<size_t>(tmp, LR_VLEN) * LR_VLEN;
  size_t miniBatchSize = tmp2 < numSamples ? tmp2 : numSamples;
  size_t numBatches = ceil_div(numSamples, miniBatchSize);
  std::vector<std::vector<T>> ret(numBatches);
  T* labelp = &label[0];
  size_t cnt = 0;
  for(size_t i = 0; i < numBatches; i++) {
    size_t tmp = numSamples - i * miniBatchSize;
    size_t size = (tmp >= miniBatchSize) ? miniBatchSize : tmp;
    ret[i].resize(size);
    T* retp = &ret[i][0];
    for(size_t j = 0; j < size; j++) {
      retp[j] = labelp[cnt+j];
    }    
    cnt += size;
  }
  return ret; 
}

template <class T>
inline void release_memory_vector(std::vector<T>& vec) {
  std::vector<T> tmp;
  tmp.swap(vec);
}

template <class T>
inline void clear_data(crs_matrix_local<T>& data) {
  release_memory_vector<T>(data.val);
  release_memory_vector<size_t>(data.idx);
  release_memory_vector<size_t>(data.off);
}

template <class T>
inline void clear_data_vector(std::vector<crs_matrix_local<T>>& data) {
  for(size_t i = 0; i < data.size(); i++) {
    clear_data(data[i]);
  }
  release_memory_vector<crs_matrix_local<T>>(data);
}

template <class MODEL>
MODEL calc_diff (MODEL& left_m, MODEL& right_m) {
  MODEL diff;
  diff = left_m - right_m;
  return diff;
}

template <class MODEL>
void calc_diff_inplace (MODEL& left_m, MODEL& right_m) {
  left_m -= right_m;
}

template <class MODEL, class T>
void get_weight(MODEL& m, std::vector<T>& v) {
  v.swap(m.weight);
}

template <class T, class MODEL>
T get_intercept(MODEL& m) {
  return m.intercept;
}

template <class MODEL, class T>
void get_weight_intercept(MODEL& m, std::vector<T>& v) {
  size_t size = m.weight.size();
  v.resize(size + 1);
  T* vp = &v[0];
  T* weightp = &m.weight[0];
  for(size_t i = 0; i < size; i++) vp[i] = weightp[i];
  v[size] = m.intercept;
}

template <class T>
void debug_vector_print(const std::vector<T>& vec) {
  for(int i=0; i<vec.size(); i++) std::cout << " " << vec[i];
  std::cout << std::endl;
}

template <class T>
std::vector<T>
operator-(const std::vector<T>& v1,
          const std::vector<T>& v2) {
  //debug_vector_print(v1);
  //debug_vector_print(v2);
  if (v1.size() != v2.size()) REPORT_ERROR(USER_ERROR,"incompatible vector size!\n");
  size_t size = v1.size();
  std::vector<T> ret(size);
  const T* v1p = &v1[0];
  const T* v2p = &v2[0];
  T* retp = &ret[0];
  for(size_t i=0; i<size; i++) {
    retp[i] = v1p[i] - v2p[i];
  }
  return ret;
}

template <class T, class MODEL>
inline bool is_converged(MODEL& prev_model,
                         MODEL& cur_model,
                         T convergenceTol,
                         size_t iterCount) {

  frovedis::time_spent t(TRACE);
  T RMSE = get_model_rms_error<T,MODEL>(cur_model,prev_model);

  std::string msg = "[Iteration: " + ITOS(iterCount) + "]" +
                    " RMS error: " + ITOS(RMSE) + " elapsed-time: ";
  t.show(msg);

  if (RMSE <= convergenceTol) {
    std::string msg;
    msg = "Convergence achieved in " + ITOS(iterCount) + " iterations.\n";
    REPORT_INFO(msg);
    return true;
  }
  else
    return false;
}

}

#endif
