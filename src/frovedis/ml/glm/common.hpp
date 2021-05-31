#ifndef _COMMON_HPP_
#define _COMMON_HPP_

#include "../../core/exceptions.hpp"
#include "../../matrix/blas_wrapper.hpp"
#include "linear_model.hpp"
#include "gradient_descent.hpp"
#include "gradient.hpp"
#include "multinomial_logistic_regression_model.hpp"
#include "softmax_gradient_descent.hpp"
#include "regularizer.hpp"
#include "rms.hpp"
#include "../utility/matrix_conversion.hpp"
#include "../utility/mattype.hpp"

#include <boost/lexical_cast.hpp>

#define LR_VLEN 1024 // made it same as JDS/ELL
#define ITOS boost::lexical_cast<std::string>

#define _CONV_RATE_CHECK_
//#define _LOSS_CHECK_
#define _RMSE_CONV_RATE_CHECK_

#ifdef _LOSS_CHECK_
#define NITER_NO_CHANGE 10
#endif 

namespace frovedis {

struct glm_config {
  glm_config() {
    numIteration = 1000;
    alpha = 0.01;
    solver = "sgd";
    isIntercept = false;
    miniBatchFraction = 1.0;
    convergenceTol = 0.001;
    warmStart = false;
    regParam = 0.01;
    regType = "ZERO";
    isMult = false;
    histSize = 10;
    lossType = "EPS";
    epsilon = 0.0;
  }

  glm_config& 
  set_max_iter(int max_iter) {
    std::string msg = "expected a positive max_iter; received: " + STR(max_iter) + "\n";
    require(max_iter > 0, msg);
    this->numIteration = max_iter;
    return *this;  
  }
  glm_config& 
  set_alpha(double alpha) {
    std::string msg = "expected a positive alpha (learning rate); received: " + STR(alpha) + "\n";
    require(alpha > 0, msg);
    this->alpha = alpha;
    return *this;  
  }
  glm_config& 
  set_solver(const std::string& solver) {
    std::string msg = "expected sgd, shrink-sgd, lbfgs, lapack or scalapack; received: "
                       + solver + "\n";
    require(solver == "sgd" || solver == "shrink-sgd" || solver == "lbfgs" 
            || solver == "lapack" || solver == "scalapack", msg);
    this->solver = solver;
    return *this;  
  }
  glm_config& 
  set_intercept(bool is_intercept) {
    this->isIntercept = is_intercept;
    return *this;  
  }
  glm_config& 
  set_mini_batch_fraction(double mbf) {
    std::string msg = "expected a positive mini-batch fraction; received: " + STR(mbf) + "\n";
    require(mbf > 0, msg);
    this->miniBatchFraction = mbf;
    return *this;  
  }
  glm_config& 
  set_tol(double tol) {
    std::string msg = "expected a positive convergence tolerance; received: " + STR(tol) + "\n";
    require(tol > 0, msg);
    this->convergenceTol = tol;
    return *this;  
  }
  glm_config&
  set_warm_start(bool warm_start) {
    this->warmStart = warm_start;
    return *this;
  }
  glm_config&
  set_reg_param(double reg_param) {
    std::string msg = "expected a positive regularization parameter; received: " + STR(reg_param) + "\n";
    require(reg_param > 0, msg);
    this->regParam = reg_param;
    return *this;  
  }
  glm_config& 
  set_reg_type(const std::string& reg_type) {
    std::string msg = "expected ZERO, L1 or L2; received: " + reg_type + "\n";
    require(reg_type == "ZERO" || reg_type == "L1" || reg_type == "L2", msg);
    this->regType = reg_type;
    return *this;  
  }
  glm_config& 
  set_is_mult(bool is_mult) {
    this->isMult = is_mult;
    return *this;  
  }
  glm_config& 
  set_hist_size(int hs) {
    std::string msg = "expected a positive history size; received: " + STR(hs) + "\n";
    require(hs > 0, msg);
    this->histSize = hs;
    return *this;  
  }
  glm_config&
  set_loss_type(const std::string& loss_type) {
    std::string msg = "expected EPS (epsilon-insensitive) or ";
    msg += "SQEPS (squared-epsilon-insensitive); received: " + loss_type + "\n";
    require(loss_type == "EPS" || loss_type == "SQEPS", msg);
    this->lossType = loss_type;
    return *this;
  }
  glm_config&
  set_epsilon(double epsilon) {
    std::string msg = "expected a positive epsilon; received: " + STR(epsilon) + "\n";
    require(epsilon >= 0, msg);
    this->epsilon = epsilon;
    return *this;
  }
  void debug_print() {
    std::cout<<"numIteration: "<<numIteration<<std::endl;
    std::cout<<"alpha: "<<alpha<<std::endl;
    std::cout<<"solver: "<<solver<<std::endl;
    std::cout<<"isIntercept: "<<isIntercept<<std::endl;
    std::cout<<"miniBatchFraction: "<<miniBatchFraction<<std::endl;
    std::cout<<"convergenceTol: "<<convergenceTol<<std::endl;
    std::cout<<"warmStart: "<<warmStart<<std::endl;
    std::cout<<"regParam: "<<regParam<<std::endl;
    std::cout<<"regType: "<<regType<<std::endl;
    std::cout<<"isMult: "<<isMult<<std::endl;
    std::cout<<"histSize: "<<histSize<<std::endl;
    std::cout<<"lossType: "<<lossType<<std::endl;
    std::cout<<"epsilon: "<<epsilon<<std::endl;
  }

  size_t numIteration;
  double alpha;
  std::string solver;
  bool isIntercept;
  double miniBatchFraction;
  double convergenceTol;
  bool warmStart;
  double regParam;
  std::string regType;
  bool isMult;
  int histSize;
  std::string lossType;
  double epsilon;
  SERIALIZE(numIteration, alpha, solver, isIntercept, miniBatchFraction,
            convergenceTol, warmStart, regParam, regType, isMult, histSize,
            lossType, epsilon);
};

template<class GRADIENT, class REGULARIZER>
struct sgd_config {
  sgd_config() {
    numIteration = 1000;
    alpha = 0.01;
    convergenceTol = 0.001;
    isIntercept = false;
    numSamples = 0;
  }
  sgd_config(size_t niter, double lr,
             double tol, bool icpt, size_t numSamples, GRADIENT& grad,
             REGULARIZER& rType):
    numIteration(niter), alpha(lr),
    convergenceTol(tol), isIntercept(icpt), numSamples(numSamples),
    grad(grad), rType(rType) {}

  size_t numIteration;
  double alpha, convergenceTol;
  bool isIntercept;
  size_t numSamples;
  GRADIENT grad;
  REGULARIZER rType;
  SERIALIZE(numIteration, alpha, convergenceTol, isIntercept, numSamples,
            grad, rType)
};

template <class T>
std::vector<sliced_colmajor_matrix_local<T>>
divide_data_to_minibatch_colmajor(colmajor_matrix_local<T>& data,
                                  double miniBatchFraction) {
  size_t numSamples = data.local_num_row;
  size_t tmp = static_cast<size_t>(ceil(numSamples * miniBatchFraction));
  size_t tmp2 = ceil_div<size_t>(tmp, LR_VLEN) * LR_VLEN;
  size_t miniBatchSize = tmp2 < numSamples ? tmp2 : numSamples;
  size_t numBatches = ceil_div(numSamples, miniBatchSize);
  std::vector<sliced_colmajor_matrix_local<T>> ret(numBatches);
  for(size_t i = 0; i < numBatches; i++) {
    size_t start_row = i * miniBatchSize;
    size_t end_row =  (i + 1) * miniBatchSize;
    if(end_row > data.local_num_row) end_row = data.local_num_row;
    size_t nrow = end_row - start_row;
    size_t ncol = data.local_num_col;
    // the minibatch data would be needed for only gemv() calculation
    // thus simply slicing the input matrix to minibatches, instead of copying
    ret[i] = make_sliced_colmajor_matrix_local<T>(data,start_row,0,nrow,ncol);
  }  
  return ret;
}

template <class T, class I, class O>
std::vector<crs_matrix_local<T,I,O>>
divide_data_to_minibatch_crs(crs_matrix_local<T,I,O>& data,
                             double miniBatchFraction) {
  size_t numSamples = data.local_num_row;
  size_t tmp = static_cast<size_t>(ceil(numSamples * miniBatchFraction));
  size_t tmp2 = ceil_div<size_t>(tmp, LR_VLEN) * LR_VLEN;
  size_t miniBatchSize = tmp2 < numSamples ? tmp2 : numSamples;
  size_t numBatches = ceil_div(numSamples, miniBatchSize);
  std::vector<crs_matrix_local<T,I,O>> ret(numBatches);
  T* datavalp = &data.val[0];
  I* dataidxp = &data.idx[0];
  O* dataoffp = &data.off[0];
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
    I* idxp = &ret[i].idx[0];
    O* offp = &ret[i].off[0];
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
std::vector<std::vector<T>>
divide_sample_weight_to_minibatch(std::vector<T>& sample_weight,
                          double miniBatchFraction) {
  size_t numSamples = sample_weight.size();
  size_t tmp = static_cast<size_t>(ceil(numSamples * miniBatchFraction));
  size_t tmp2 = ceil_div<size_t>(tmp, LR_VLEN) * LR_VLEN;
  size_t miniBatchSize = tmp2 < numSamples ? tmp2 : numSamples;
  size_t numBatches = ceil_div(numSamples, miniBatchSize);
  std::vector<std::vector<T>> ret(numBatches);
  T* swp = &sample_weight[0];
  size_t cnt = 0;
  for(size_t i = 0; i < numBatches; i++) {
    size_t tmp = numSamples - i * miniBatchSize;
    size_t size = (tmp >= miniBatchSize) ? miniBatchSize : tmp;
    ret[i].resize(size);
    T* retp = &ret[i][0];
    for(size_t j = 0; j < size; j++) {
      retp[j] = swp[cnt+j];
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

template <class T, class I, class O>
inline void clear_data(crs_matrix_local<T,I,O>& data) {
  release_memory_vector<T>(data.val);
  release_memory_vector<I>(data.idx);
  release_memory_vector<O>(data.off);
}

template <class T, class I, class O>
inline void clear_data_vector(std::vector<crs_matrix_local<T,I,O>>& data) {
  for(size_t i = 0; i < data.size(); i++) {
    clear_data(data[i]);
  }
  release_memory_vector<crs_matrix_local<T,I,O>>(data);
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

template <class T, class MODEL>
void get_weight(MODEL& m, std::vector<T>& v) {
  v.swap(m.weight);
}

template <class T, class MODEL>
T get_intercept(MODEL& m) {
  return m.intercept;
}

template <class T, class MODEL>
void get_weight_intercept(MODEL& m, std::vector<T>& v) {
  size_t size = m.weight.size();
  v.resize(size + 1);
  T* vp = &v[0];
  T* weightp = &m.weight[0];
  for(size_t i = 0; i < size; i++) vp[i] = weightp[i];
  v[size] = m.intercept;
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
