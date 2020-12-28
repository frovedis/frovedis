#ifndef _LASSO_SGD_HPP_
#define _LASSO_SGD_HPP_

#include "sgd_parallelizer.hpp"

namespace frovedis {

class lasso_with_sgd {
public:
  template <class T, class I, class O>
  static linear_regression_model<T> train (
    crs_matrix<T,I,O>& data,
    dvector<T>& label,
    size_t numIteration=1000, 
    double alpha=0.01, 
    double miniBatchFraction=1.0, 
    double regParam=0.01, 
    bool isIntercept=false,
    double convergenceTol=0.001, 
#if defined(_SX) || defined(__ve__)
    MatType mType = HYBRID 
#else
    MatType mType = CRS
#endif 
  );

  template <class T, class I, class O>
  static linear_regression_model<T> train (
    crs_matrix<T,I,O>&& data,
    dvector<T>& label,
    size_t numIteration=1000, 
    double alpha=0.01, 
    double miniBatchFraction=1.0, 
    double regParam=0.01, 
    bool isIntercept=false,
    double convergenceTol=0.001, 
#if defined(_SX) || defined(__ve__)
    MatType mType = HYBRID 
#else
    MatType mType = CRS
#endif 
  );

  template <class T, class I, class O>
  static linear_regression_model<T> train (
    crs_matrix<T,I,O>&& data,
    dvector<T>& label,
    linear_regression_model<T>& lrm,
    size_t numIteration=1000, 
    double alpha=0.01, 
    double miniBatchFraction=1.0, 
    double regParam=0.01, 
    bool isIntercept=false,
    double convergenceTol=0.001, 
#if defined(_SX) || defined(__ve__)
    MatType mType = HYBRID 
#else
    MatType mType = CRS
#endif 
  );

  template <class T, class I, class O>
  static linear_regression_model<T> train (
    crs_matrix<T,I,O>& data,
    dvector<T>& label,
    linear_regression_model<T>& lrm,
    size_t numIteration=1000, 
    double alpha=0.01, 
    double miniBatchFraction=1.0, 
    double regParam=0.01, 
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
  static linear_regression_model<T> train (
    rowmajor_matrix<T>& data,
    dvector<T>& label,
    size_t numIteration=1000,
    double alpha=0.01,
    double miniBatchFraction=1.0,
    double regParam=0.01,
    bool isIntercept=false,
    double convergenceTol=0.001);

  template <class T>
  static linear_regression_model<T> train (
    const colmajor_matrix<T>& data,
    dvector<T>& label,
    size_t numIteration=1000, 
    double alpha=0.01, 
    double miniBatchFraction=1.0, 
    double regParam=0.01, 
    bool isIntercept=false,
    double convergenceTol=0.001);

  template <class T>
  static linear_regression_model<T> train (
    const colmajor_matrix<T>& data,
    dvector<T>& label,
    linear_regression_model<T>& initModel,
    size_t numIteration=1000, 
    double alpha=0.01, 
    double miniBatchFraction=1.0, 
    double regParam=0.01, 
    bool isIntercept=false,
    double convergenceTol=0.001);
};

template <class T, class I, class O>
linear_regression_model<T>
lasso_with_sgd::train (crs_matrix<T,I,O>& data,
                       dvector<T>& label,
                       size_t numIteration,
                       double alpha,
                       double miniBatchFraction,
                       double regParam,
                       bool isIntercept,
                       double convergenceTol,
                       MatType mType) {
  size_t numFeatures = data.num_col;
  linear_regression_model<T> initModel(numFeatures);
  return train<T>(data,label,initModel,numIteration,alpha,miniBatchFraction,
                  regParam,isIntercept,convergenceTol,mType,false);
}

template <class T, class I, class O>
linear_regression_model<T>
lasso_with_sgd::train (crs_matrix<T,I,O>&& data,
                       dvector<T>& label,
                       size_t numIteration,
                       double alpha,
                       double miniBatchFraction,
                       double regParam,
                       bool isIntercept,
                       double convergenceTol,
                       MatType mType) {
  size_t numFeatures = data.num_col;
  linear_regression_model<T> initModel(numFeatures);
  return train<T>(data,label,initModel,numIteration,alpha,miniBatchFraction,
                  regParam,isIntercept,convergenceTol,mType,true);
}

template <class T, class I, class O>
linear_regression_model<T>
lasso_with_sgd::train (crs_matrix<T,I,O>&& data,
                       dvector<T>& label,
                       linear_regression_model<T>& initModel,
                       size_t numIteration,
                       double alpha,
                       double miniBatchFraction,
                       double regParam,
                       bool isIntercept,
                       double convergenceTol,
                       MatType mType) {
  return train<T>(data,label,initModel,numIteration,alpha,miniBatchFraction,
                  regParam,isIntercept,convergenceTol,mType,true);
}

// --- main api with sparse data suppoort ---
template <class T, class I, class O>
linear_regression_model<T>
lasso_with_sgd::train (crs_matrix<T,I,O>& data,
                       dvector<T>& label,
                       linear_regression_model<T>& initModel,
                       size_t numIteration,
                       double alpha,
                       double miniBatchFraction,
                       double regParam,
                       bool isIntercept,
                       double convergenceTol,
                       MatType mType,
                       bool inputMovable) {
#ifdef _DEBUG_
  std::cout << "Initial model: \n";
  initModel.debug_print(); std::cout << "\n";
#endif

  sgd_parallelizer par(miniBatchFraction);
  linear_gradient<T> grad;
  l1_regularizer<T> rType(regParam);
  return par.template parallelize<T,I,O,linear_regression_model<T>,
                                  linear_gradient<T>, l1_regularizer<T>>
         (data,label,initModel,grad,rType,numIteration,alpha,
          isIntercept,convergenceTol,mType,inputMovable);
}

template <class T>
linear_regression_model<T>
lasso_with_sgd::train (rowmajor_matrix<T>& data,
                       dvector<T>& label,
                       size_t numIteration,
                       double alpha,
                       double miniBatchFraction,
                       double regParam,
                       bool isIntercept,
                       double convergenceTol) {
  return train<T>(colmajor_matrix<T>(data),label,
                  numIteration, alpha, miniBatchFraction,
                  regParam, isIntercept, convergenceTol);
}

template <class T>
linear_regression_model<T>
lasso_with_sgd::train (const colmajor_matrix<T>& data,
                       dvector<T>& label,
                       size_t numIteration,
                       double alpha,
                       double miniBatchFraction,
                       double regParam,
                       bool isIntercept,
                       double convergenceTol) {
  size_t numFeatures = data.num_col;
  linear_regression_model<T> initModel(numFeatures);
  return train<T>(data,label,initModel,numIteration,alpha,miniBatchFraction,
                  regParam,isIntercept,convergenceTol);
}

// --- main api with dense data suppoort ---
template <class T>
linear_regression_model<T>
lasso_with_sgd::train (const colmajor_matrix<T>& data,
                       dvector<T>& label,
                       linear_regression_model<T>& initModel,
                       size_t numIteration,
                       double alpha,
                       double miniBatchFraction,
                       double regParam,
                       bool isIntercept,
                       double convergenceTol) {
#ifdef _DEBUG_
  std::cout << "Initial model: \n";
  initModel.debug_print(); std::cout << "\n";
#endif

  auto& dmat = const_cast<colmajor_matrix<T>&> (data);
  sgd_parallelizer par(miniBatchFraction);
  linear_gradient<T> grad;
  l1_regularizer<T> rType(regParam);
  return par.template parallelize<T,linear_regression_model<T>,
                                  linear_gradient<T>, l1_regularizer<T>>
         (dmat,label,initModel,grad,rType,numIteration,alpha,
          isIntercept,convergenceTol);
}

}
#endif
