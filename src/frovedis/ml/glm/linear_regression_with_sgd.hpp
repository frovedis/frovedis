#ifndef _LNR_SGD_HPP_
#define _LNR_SGD_HPP_

#include "sgd_parallelizer.hpp"

namespace frovedis {

class linear_regression_with_sgd {
public:
  template <class T, class I, class O>
  static linear_regression_model<T> train (
    crs_matrix<T,I,O>& data,
    dvector<T>& label,
    size_t numIteration=1000, 
    double alpha=0.01, 
    double miniBatchFraction=1.0, 
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
    const colmajor_matrix<T>& data,
    dvector<T>& label,
    size_t numIteration=1000, 
    double alpha=0.01, 
    double miniBatchFraction=1.0, 
    bool isIntercept=false,
    double convergenceTol=0.001);

  template <class T>
  static linear_regression_model<T> train (
    const colmajor_matrix<T>& data,
    dvector<T>& label,
    linear_regression_model<T>& lrm,
    size_t numIteration=1000, 
    double alpha=0.01, 
    double miniBatchFraction=1.0, 
    bool isIntercept=false,
    double convergenceTol=0.001);
};

template <class T, class I, class O>
linear_regression_model<T>
linear_regression_with_sgd::train (crs_matrix<T,I,O>& data,
                                   dvector<T>& label,
                                   size_t numIteration,
                                   double alpha,
                                   double miniBatchFraction,
                                   bool isIntercept,
                                   double convergenceTol,
                                   MatType mType) {
  size_t numFeatures = data.num_col;
  linear_regression_model<T> initModel(numFeatures);
  return train<T>(data,label,initModel,numIteration,alpha,miniBatchFraction,
                  isIntercept,convergenceTol,mType,false);
}

template <class T, class I, class O>
linear_regression_model<T>
linear_regression_with_sgd::train (crs_matrix<T,I,O>&& data,
                                   dvector<T>& label,
                                   size_t numIteration,
                                   double alpha,
                                   double miniBatchFraction,
                                   bool isIntercept,
                                   double convergenceTol,
                                   MatType mType) {
  size_t numFeatures = data.num_col;
  linear_regression_model<T> initModel(numFeatures);
  return train<T>(data,label,initModel,numIteration,alpha,miniBatchFraction,
                  isIntercept,convergenceTol,mType,true);
}

template <class T, class I, class O>
linear_regression_model<T>
linear_regression_with_sgd::train (crs_matrix<T,I,O>&& data,
                                   dvector<T>& label,
                                   linear_regression_model<T>& initModel,
                                   size_t numIteration,
                                   double alpha,
                                   double miniBatchFraction,
                                   bool isIntercept,
                                   double convergenceTol,
                                   MatType mType) {
  return train<T>(data,label,initModel,numIteration,alpha,miniBatchFraction,
                  isIntercept,convergenceTol,mType,true);
}

// --- main api with sparse data support ---
template <class T, class I, class O>
linear_regression_model<T>
linear_regression_with_sgd::train (crs_matrix<T,I,O>& data,
                                   dvector<T>& label,
                                   linear_regression_model<T>& initModel,
                                   size_t numIteration,
                                   double alpha,
                                   double miniBatchFraction,
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
  zero_regularizer<T> rType(0.0);
  return par.template parallelize<T,I,O,linear_regression_model<T>,
                                  linear_gradient<T>, zero_regularizer<T>>
         (data,label,initModel,grad,rType,numIteration,alpha,
          isIntercept,convergenceTol,mType,inputMovable);
}

template <class T>
linear_regression_model<T>
linear_regression_with_sgd::train (const colmajor_matrix<T>& data,
                                   dvector<T>& label,
                                   size_t numIteration,
                                   double alpha,
                                   double miniBatchFraction,
                                   bool isIntercept,
                                   double convergenceTol) {
  size_t numFeatures = data.num_col;
  linear_regression_model<T> initModel(numFeatures);
  return train<T>(data,label,initModel,numIteration,alpha,miniBatchFraction,
                  isIntercept,convergenceTol);
}

// --- main api with dense data support ---
template <class T>
linear_regression_model<T>
linear_regression_with_sgd::train (const colmajor_matrix<T>& data,
                                   dvector<T>& label,
                                   linear_regression_model<T>& initModel,
                                   size_t numIteration,
                                   double alpha,
                                   double miniBatchFraction,
                                   bool isIntercept,
                                   double convergenceTol) {
#ifdef _DEBUG_
  std::cout << "Initial model: \n";
  initModel.debug_print(); std::cout << "\n";
#endif

  auto& dmat = const_cast<colmajor_matrix<T>&> (data);
  linear_gradient<T> grad;
  sgd_parallelizer par(miniBatchFraction);
  zero_regularizer<T> rType(0.0);
  return par.template parallelize<T,linear_regression_model<T>,
                                  linear_gradient<T>, zero_regularizer<T>>
         (dmat,label,initModel,grad,rType,numIteration,alpha,
          isIntercept,convergenceTol);
}

}
#endif
