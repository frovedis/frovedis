#ifndef _LR_LBFGS_HPP_
#define _LR_LBFGS_HPP_

#include "lbfgs_parallelizer.hpp"

namespace frovedis {

class logistic_regression_with_lbfgs {
public:
  template <class T, class I, class O>
  static logistic_regression_model<T> train (
    crs_matrix<T,I,O>& data,
    dvector<T>& label,
    size_t numIteration=1000, 
    double alpha=0.01, 
    size_t hist_size=10, 
    double regParam=0.01, 
    RegType regTyp=ZERO, 
    bool isIntercept=false,
    double convergenceTol=0.001, 
#if defined(_SX) || defined(__ve__)
    MatType mType = HYBRID 
#else
    MatType mType = CRS
#endif 
  );

  template <class T, class I, class O>
  static logistic_regression_model<T> train (
    crs_matrix<T,I,O>& data,
    dvector<T>& label,
    std::vector<T>& sample_weight,
    size_t& n_iter,
    size_t numIteration=1000,
    double alpha=0.01,
    size_t hist_size=10,
    double regParam=0.01,
    RegType regTyp=ZERO,
    bool isIntercept=false,
    double convergenceTol=0.001,
#if defined(_SX) || defined(__ve__)
    MatType mType = HYBRID
#else
    MatType mType = CRS
#endif
  );

  template <class T, class I, class O>
  static logistic_regression_model<T> train (
    crs_matrix<T,I,O>&& data,
    dvector<T>& label,
    size_t numIteration=1000, 
    double alpha=0.01, 
    size_t hist_size=10, 
    double regParam=0.01, 
    RegType regTyp=ZERO, 
    bool isIntercept=false, 
    double convergenceTol=0.001, 
#if defined(_SX) || defined(__ve__)
    MatType mType = HYBRID 
#else
    MatType mType = CRS
#endif 
  );

  template <class T, class I, class O>
  static logistic_regression_model<T> train (
    crs_matrix<T,I,O>&& data,
    dvector<T>& label,
    std::vector<T>& sample_weight,
    size_t& n_iter,
    size_t numIteration=1000,
    double alpha=0.01,
    size_t hist_size=10,
    double regParam=0.01,
    RegType regTyp=ZERO,
    bool isIntercept=false,
    double convergenceTol=0.001,
#if defined(_SX) || defined(__ve__)
    MatType mType = HYBRID
#else
    MatType mType = CRS
#endif
  );

  template <class T, class I, class O>
  static logistic_regression_model<T> train (
    crs_matrix<T,I,O>&& data,
    dvector<T>& label,
    logistic_regression_model<T>& lrm,
    std::vector<T>& sample_weight,
    size_t& n_iter,
    size_t numIteration=1000,
    double alpha=0.01,
    size_t hist_size=10,
    double regParam=0.01,
    RegType regTyp=ZERO,
    bool isIntercept=false,
    double convergenceTol=0.001,
#if defined(_SX) || defined(__ve__)
    MatType mType = HYBRID
#else
    MatType mType = CRS
#endif
  );

  template <class T, class I, class O>
  static logistic_regression_model<T> train (
    crs_matrix<T,I,O>& data,
    dvector<T>& label,
    logistic_regression_model<T>& lrm,
    std::vector<T>& sample_weight,
    size_t& n_iter,
    size_t numIteration=1000,
    double alpha=0.01,
    size_t hist_size=10,
    double regParam=0.01,
    RegType regTyp=ZERO,
    bool isIntercept=false,
    double convergenceTol=0.001,
#if defined(_SX) || defined(__ve__)
    MatType mType = HYBRID,
#else
    MatType mType = CRS,
#endif
    bool inputMovable=false
  );

  // --- dense support ---
  template <class T>
  static logistic_regression_model<T> train (
    rowmajor_matrix<T>& data,
    dvector<T>& label,
    size_t numIteration=1000,
    double alpha=0.01,
    size_t hist_size=10,
    double regParam=0.01,
    RegType regTyp=ZERO,
    bool isIntercept=false,
    double convergenceTol=0.001,
#if defined(_SX) || defined(__ve__)
    MatType mType = HYBRID 
#else
    MatType mType = CRS
#endif 
  );

  template <class T>
  static logistic_regression_model<T> train (
    rowmajor_matrix<T>& data,
    dvector<T>& label,
    std::vector<T>& sample_weight,
    size_t& n_iter,
    size_t numIteration=1000,
    double alpha=0.01,
    size_t hist_size=10,
    double regParam=0.01,
    RegType regTyp=ZERO,
    bool isIntercept=false,
    double convergenceTol=0.001,
#if defined(_SX) || defined(__ve__)
    MatType mType = HYBRID 
#else
    MatType mType = CRS
#endif 
  );

  template <class T>
  static logistic_regression_model<T> train (
    const colmajor_matrix<T>& data,
    dvector<T>& label,
    size_t numIteration=1000, 
    double alpha=0.01, 
    size_t hist_size=10, 
    double regParam=0.01, 
    RegType regTyp=ZERO, 
    bool isIntercept=false,
    double convergenceTol=0.001,
#if defined(_SX) || defined(__ve__)
    MatType mType = HYBRID 
#else
    MatType mType = CRS
#endif 
  );

  template <class T>
  static logistic_regression_model<T> train (
    const colmajor_matrix<T>& data,
    dvector<T>& label,
    std::vector<T>& sample_weight,
    size_t& n_iter,
    size_t numIteration=1000,
    double alpha=0.01,
    size_t hist_size=10,
    double regParam=0.01,
    RegType regTyp=ZERO,
    bool isIntercept=false,
    double convergenceTol=0.001,
#if defined(_SX) || defined(__ve__)
    MatType mType = HYBRID 
#else
    MatType mType = CRS
#endif 
  );

  template <class T>
  static logistic_regression_model<T> train (
    colmajor_matrix<T>&& data,
    dvector<T>& label,
    size_t numIteration=1000, 
    double alpha=0.01, 
    size_t hist_size=10, 
    double regParam=0.01, 
    RegType regTyp=ZERO, 
    bool isIntercept=false,
    double convergenceTol=0.001,
#if defined(_SX) || defined(__ve__)
    MatType mType = HYBRID 
#else
    MatType mType = CRS
#endif 
  );

  template <class T>
  static logistic_regression_model<T> train (
    colmajor_matrix<T>&& data,
    dvector<T>& label,
    std::vector<T>& sample_weight,
    size_t& n_iter,
    size_t numIteration=1000,
    double alpha=0.01,
    size_t hist_size=10,
    double regParam=0.01,
    RegType regTyp=ZERO,
    bool isIntercept=false,
    double convergenceTol=0.001,
#if defined(_SX) || defined(__ve__)
    MatType mType = HYBRID 
#else
    MatType mType = CRS
#endif 
  );

  template <class T>
  static logistic_regression_model<T> train (
    const colmajor_matrix<T>&& data,
    dvector<T>& label,
    logistic_regression_model<T>& lrm,
    std::vector<T>& sample_weight,
    size_t& n_iter,
    size_t numIteration=1000, 
    double alpha=0.01, 
    size_t hist_size=10, 
    double regParam=0.01, 
    RegType regTyp=ZERO, 
    bool isIntercept=false,
    double convergenceTol=0.001,
#if defined(_SX) || defined(__ve__)
    MatType mType = HYBRID 
#else
    MatType mType = CRS
#endif 
  );

  template <class T>
  static logistic_regression_model<T> train (
    const colmajor_matrix<T>& data,
    dvector<T>& label,
    logistic_regression_model<T>& lrm,
    std::vector<T>& sample_weight,
    size_t& n_iter,
    size_t numIteration=1000, 
    double alpha=0.01, 
    size_t hist_size=10, 
    double regParam=0.01, 
    RegType regTyp=ZERO, 
    bool isIntercept=false,
    double convergenceTol=0.001,
#if defined(_SX) || defined(__ve__)
    MatType mType = HYBRID, 
#else
    MatType mType = CRS,
#endif 
    bool inputMovable=false);
};

template <class T, class I, class O>
logistic_regression_model<T>
logistic_regression_with_lbfgs::train (crs_matrix<T,I,O>& data,
                                       dvector<T>& label,
                                       size_t numIteration,
                                       double alpha,
                                       size_t hist_size,
                                       double regParam,
                                       RegType regTyp,
                                       bool isIntercept,
                                       double convergenceTol,
                                       MatType mType) {
  size_t numFeatures = data.num_col;
  T intercept = isIntercept ? 1.0 : 0.0;
  logistic_regression_model<T> initModel(numFeatures,intercept);
  size_t n_iter = 0;
  std::vector<T> sample_weight;
  return train<T>(data,label,initModel,sample_weight,n_iter,
                  numIteration,alpha,hist_size,
                  regParam,regTyp,isIntercept,convergenceTol,mType,false);
}

template <class T, class I, class O>
logistic_regression_model<T>
logistic_regression_with_lbfgs::train (crs_matrix<T,I,O>& data,
                                       dvector<T>& label,
                                       std::vector<T>& sample_weight,
                                       size_t& n_iter,
                                       size_t numIteration,
                                       double alpha,
                                       size_t hist_size,
                                       double regParam,
                                       RegType regTyp,
                                       bool isIntercept,
                                       double convergenceTol,
                                       MatType mType) {
  size_t numFeatures = data.num_col;
  T intercept = isIntercept ? 1.0 : 0.0;
  logistic_regression_model<T> initModel(numFeatures,intercept);
  return train<T>(data,label,initModel,sample_weight,n_iter,
                  numIteration,alpha,hist_size,
                  regParam,regTyp,isIntercept,convergenceTol,mType,false);
}

template <class T, class I, class O>
logistic_regression_model<T>
logistic_regression_with_lbfgs::train (crs_matrix<T,I,O>&& data,
                                       dvector<T>& label,
                                       size_t numIteration,
                                       double alpha,
                                       size_t hist_size,
                                       double regParam,
                                       RegType regTyp,
                                       bool isIntercept,
                                       double convergenceTol,
                                       MatType mType) {
  size_t numFeatures = data.num_col;
  T intercept = isIntercept ? 1.0 : 0.0;
  logistic_regression_model<T> initModel(numFeatures,intercept);
  size_t n_iter = 0;
  std::vector<T> sample_weight;
  return train<T>(data,label,initModel,sample_weight,n_iter,
                  numIteration,alpha,hist_size,
                  regParam,regTyp,isIntercept,convergenceTol,mType,true);
}

template <class T, class I, class O>
logistic_regression_model<T>
logistic_regression_with_lbfgs::train (crs_matrix<T,I,O>&& data,
                                       dvector<T>& label,
                                       std::vector<T>& sample_weight,
                                       size_t& n_iter,
                                       size_t numIteration,
                                       double alpha,
                                       size_t hist_size,
                                       double regParam,
                                       RegType regTyp,
                                       bool isIntercept,
                                       double convergenceTol,
                                       MatType mType) {
  size_t numFeatures = data.num_col;
  T intercept = isIntercept ? 1.0 : 0.0;
  logistic_regression_model<T> initModel(numFeatures,intercept);
  return train<T>(data,label,initModel,sample_weight,n_iter,
                  numIteration,alpha,hist_size,
                  regParam,regTyp,isIntercept,convergenceTol,mType,true);
}

template <class T, class I, class O>
logistic_regression_model<T>
logistic_regression_with_lbfgs::train (crs_matrix<T,I,O>&& data,
                                       dvector<T>& label,
                                       logistic_regression_model<T>& initModel,
                                       std::vector<T>& sample_weight,
                                       size_t& n_iter,
                                       size_t numIteration,
                                       double alpha,
                                       size_t hist_size,
                                       double regParam,
                                       RegType regTyp,
                                       bool isIntercept,
                                       double convergenceTol,
                                       MatType mType) {
  return train<T>(data,label,initModel,sample_weight,n_iter,
                  numIteration,alpha,hist_size,
                  regParam,regTyp,isIntercept,convergenceTol,mType,true);
}

// --- main api with sparse data support ---
template <class T, class I, class O>
logistic_regression_model<T>
logistic_regression_with_lbfgs::train (crs_matrix<T,I,O>& data,
                                       dvector<T>& label,
                                       logistic_regression_model<T>& initModel,
                                       std::vector<T>& sample_weight,
                                       size_t& n_iter,
                                       size_t numIteration,
                                       double alpha,
                                       size_t hist_size,
                                       double regParam,
                                       RegType regTyp,
                                       bool isIntercept,
                                       double convergenceTol,
                                       MatType mType,
                                       bool inputMovable) {
#ifdef _DEBUG_
  std::cout << "Initial model: \n";
  initModel.debug_print(); std::cout << "\n";
#endif

  if(sample_weight.empty()) sample_weight = vector_full<T>(data.num_row, 1);
  lbfgs_parallelizer par(hist_size);
  logistic_regression_model<T> ret;

  if (regTyp == ZERO)
    ret = par.template parallelize<T,I,O,logistic_regression_model<T>,
                                   logistic_gradient<T>, zero_regularizer<T>>
          (data,label,initModel,sample_weight,n_iter,numIteration,alpha,regParam,
           isIntercept,convergenceTol,mType,inputMovable);
  else if (regTyp == L1)
    ret = par.template parallelize<T,I,O,logistic_regression_model<T>,
                                   logistic_gradient<T>, l1_regularizer<T>>
          (data,label,initModel,sample_weight,n_iter,numIteration,alpha,regParam,
           isIntercept,convergenceTol,mType,inputMovable);
  else if (regTyp == L2)
    ret = par.template parallelize<T,I,O,logistic_regression_model<T>,
                                   logistic_gradient<T>, l2_regularizer<T>>
          (data,label,initModel,sample_weight,n_iter,numIteration,alpha,regParam,
           isIntercept,convergenceTol,mType,inputMovable);
  return ret;
}

template <class T>
logistic_regression_model<T>
logistic_regression_with_lbfgs::train (rowmajor_matrix<T>& data,
                                       dvector<T>& label,
                                       size_t numIteration,
                                       double alpha,
                                       size_t hist_size,
                                       double regParam,
                                       RegType regTyp,
                                       bool isIntercept,
                                       double convergenceTol,
                                       MatType mType) {
  return train<T>(colmajor_matrix<T>(data), label, 
                  numIteration, alpha, hist_size, 
                  regParam, regTyp, isIntercept, convergenceTol,mType); 
}

template <class T>
logistic_regression_model<T>
logistic_regression_with_lbfgs::train (rowmajor_matrix<T>& data,
                                       dvector<T>& label,
                                       std::vector<T>& sample_weight,
                                       size_t& n_iter,
                                       size_t numIteration,
                                       double alpha,
                                       size_t hist_size,
                                       double regParam,
                                       RegType regTyp,
                                       bool isIntercept,
                                       double convergenceTol,
                                       MatType mType) {
  return train<T>(colmajor_matrix<T>(data), label, sample_weight, n_iter,
                  numIteration, alpha, hist_size,
                  regParam, regTyp, isIntercept, convergenceTol,mType);
}

template <class T>
logistic_regression_model<T>
logistic_regression_with_lbfgs::train (const colmajor_matrix<T>& data,
                                       dvector<T>& label,
                                       size_t numIteration,
                                       double alpha,
                                       size_t hist_size,
                                       double regParam,
                                       RegType regTyp,
                                       bool isIntercept,
                                       double convergenceTol,
                                       MatType mType) {
  size_t numFeatures = data.num_col;
  T intercept = isIntercept ? 1.0 : 0.0;
  logistic_regression_model<T> initModel(numFeatures,intercept);
  size_t n_iter = 0;
  std::vector<T> sample_weight;
  return train<T>(data,label,initModel,sample_weight,n_iter,
                  numIteration,alpha,hist_size,
                  regParam,regTyp,isIntercept,convergenceTol,mType);
}

template <class T>
logistic_regression_model<T>
logistic_regression_with_lbfgs::train (const colmajor_matrix<T>& data,
                                       dvector<T>& label,
                                       std::vector<T>& sample_weight,
                                       size_t& n_iter,
                                       size_t numIteration,
                                       double alpha,
                                       size_t hist_size,
                                       double regParam,
                                       RegType regTyp,
                                       bool isIntercept,
                                       double convergenceTol,
                                       MatType mType) {
  size_t numFeatures = data.num_col;
  T intercept = isIntercept ? 1.0 : 0.0;
  logistic_regression_model<T> initModel(numFeatures,intercept);
  return train<T>(data,label,initModel,sample_weight,n_iter,
                  numIteration,alpha,hist_size,
                  regParam,regTyp,isIntercept,convergenceTol,mType);
}

template <class T>
logistic_regression_model<T>
logistic_regression_with_lbfgs::train (colmajor_matrix<T>&& data,
                                       dvector<T>& label,
                                       size_t numIteration,
                                       double alpha,
                                       size_t hist_size,
                                       double regParam,
                                       RegType regTyp,
                                       bool isIntercept,
                                       double convergenceTol,
                                       MatType mType) {
  size_t numFeatures = data.num_col;
  T intercept = isIntercept ? 1.0 : 0.0;
  logistic_regression_model<T> initModel(numFeatures,intercept);
  size_t n_iter = 0;
  std::vector<T> sample_weight;
  return train<T>(data,label,initModel,sample_weight,n_iter,
                  numIteration,alpha,hist_size,
                  regParam,regTyp,isIntercept,convergenceTol,mType,true);
}

template <class T>
logistic_regression_model<T>
logistic_regression_with_lbfgs::train (colmajor_matrix<T>&& data,
                                       dvector<T>& label,
                                       std::vector<T>& sample_weight,
                                       size_t& n_iter,
                                       size_t numIteration,
                                       double alpha,
                                       size_t hist_size,
                                       double regParam,
                                       RegType regTyp,
                                       bool isIntercept,
                                       double convergenceTol,
                                       MatType mType) {
  size_t numFeatures = data.num_col;
  T intercept = isIntercept ? 1.0 : 0.0;
  logistic_regression_model<T> initModel(numFeatures,intercept);
  return train<T>(data,label,initModel,sample_weight,n_iter,numIteration,
                  alpha,hist_size,
                  regParam,regTyp,isIntercept,convergenceTol,mType,true);
}
template <class T>
logistic_regression_model<T>
logistic_regression_with_lbfgs::train (const colmajor_matrix<T>&& data,
                                       dvector<T>& label,
                                       logistic_regression_model<T>& initModel,
                                       std::vector<T>& sample_weight,
                                       size_t& n_iter,
                                       size_t numIteration,
                                       double alpha,
                                       size_t hist_size,
                                       double regParam,
                                       RegType regTyp,
                                       bool isIntercept,
                                       double convergenceTol,
                                       MatType mType) {
  return train<T>(data,label,initModel,sample_weight,n_iter,numIteration,
                  alpha,hist_size,
                  regParam,regTyp,isIntercept,convergenceTol,mType,true);
}
// --- main api with dense data support ---
template <class T>
logistic_regression_model<T>
logistic_regression_with_lbfgs::train (const colmajor_matrix<T>& data,
                                       dvector<T>& label,
                                       logistic_regression_model<T>& initModel,
                                       std::vector<T>& sample_weight,
                                       size_t& n_iter,
                                       size_t numIteration,
                                       double alpha,
                                       size_t hist_size,
                                       double regParam,
                                       RegType regTyp,
                                       bool isIntercept,
                                       double convergenceTol,
                                       MatType mType,
                                       bool inputMovable) {

#ifdef _DEBUG_
  std::cout << "Initial model: \n";
  initModel.debug_print(); std::cout << "\n";
#endif

  if(sample_weight.empty()) sample_weight = vector_full<T>(data.num_row, 1);
  auto& dmat = const_cast<colmajor_matrix<T>&> (data);
  lbfgs_parallelizer par(hist_size);
  logistic_regression_model<T> ret;

  if (regTyp == ZERO)
    ret = par.template parallelize<T,logistic_regression_model<T>,
                                   logistic_gradient<T>, zero_regularizer<T>>
          (dmat,label,initModel,sample_weight,n_iter,numIteration,alpha,regParam,
           isIntercept,convergenceTol);
  else if (regTyp == L1)
    ret = par.template parallelize<T,logistic_regression_model<T>,
                                   logistic_gradient<T>, l1_regularizer<T>>
          (dmat,label,initModel,sample_weight,n_iter,numIteration,alpha,regParam,
           isIntercept,convergenceTol);
  else if (regTyp == L2)
    ret = par.template parallelize<T,logistic_regression_model<T>,
                                   logistic_gradient<T>, l2_regularizer<T>>
          (dmat,label,initModel,sample_weight,n_iter,numIteration,alpha,regParam,
           isIntercept,convergenceTol);
  return ret;
}

}

#endif
