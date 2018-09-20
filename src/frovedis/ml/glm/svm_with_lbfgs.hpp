#ifndef _SVM_LBFGS_HPP_
#define _SVM_LBFGS_HPP_

#include "lbfgs_parallelizer.hpp"

namespace frovedis {

class svm_with_lbfgs {
public:
  template <class T, class I, class O>
  static svm_model<T> train (
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
  static svm_model<T> train (
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
  static svm_model<T> train (
    crs_matrix<T,I,O>&& data,
    dvector<T>& label,
    svm_model<T>& lrm,
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
  static svm_model<T> train (
    crs_matrix<T,I,O>& data,
    dvector<T>& label,
    svm_model<T>& lrm,
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
  static svm_model<T> train (
    const colmajor_matrix<T>& data,
    dvector<T>& label,
    size_t numIteration=1000, 
    double alpha=0.01, 
    size_t hist_size=10, 
    double regParam=0.01, 
    RegType regTyp=ZERO, 
    bool isIntercept=false,
    double convergenceTol=0.001); 
 
  template <class T>
  static svm_model<T> train (
    const colmajor_matrix<T>& data,
    dvector<T>& label,
    svm_model<T>& lrm,
    size_t numIteration=1000, 
    double alpha=0.01, 
    size_t hist_size=10, 
    double regParam=0.01, 
    RegType regTyp=ZERO, 
    bool isIntercept=false,
    double convergenceTol=0.001); 
};

template <class T, class I, class O>
svm_model<T>
svm_with_lbfgs::train (crs_matrix<T,I,O>& data,
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
  svm_model<T> initModel(numFeatures,intercept);
  return train<T>(data,label,initModel,numIteration,alpha,hist_size,
                  regParam,regTyp,isIntercept,convergenceTol,mType,false);
}

template <class T, class I, class O>
svm_model<T>
svm_with_lbfgs::train (crs_matrix<T,I,O>&& data,
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
  svm_model<T> initModel(numFeatures,intercept);
  return train<T>(data,label,initModel,numIteration,alpha,hist_size,
                  regParam,regTyp,isIntercept,convergenceTol,mType,true);
}

template <class T, class I, class O>
svm_model<T>
svm_with_lbfgs::train (crs_matrix<T,I,O>&& data,
                       dvector<T>& label,
                       svm_model<T>& initModel,
                       size_t numIteration,
                       double alpha,
                       size_t hist_size,
                       double regParam,
                       RegType regTyp,
                       bool isIntercept,
                       double convergenceTol,
                       MatType mType) {
  return train<T>(data,label,initModel,numIteration,alpha,hist_size,
                  regParam,regTyp,isIntercept,convergenceTol,mType,true);
}

// --- main api with sparse data support ---
template <class T, class I, class O>
svm_model<T>
svm_with_lbfgs::train (crs_matrix<T,I,O>& data,
                       dvector<T>& label,
                       svm_model<T>& initModel,
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

  lbfgs_parallelizer par(hist_size);
  svm_model<T> ret;

  if(regTyp == ZERO)
    ret = par.template parallelize<T,I,O,svm_model<T>,
                                   hinge_gradient<T>, zero_regularizer<T>>
         (data,label,initModel,numIteration,alpha,regParam,
          isIntercept,convergenceTol,mType,inputMovable);
  else if(regTyp == L1)
    ret = par.template parallelize<T,I,O,svm_model<T>,
                                   hinge_gradient<T>, l1_regularizer<T>>
         (data,label,initModel,numIteration,alpha,regParam,
          isIntercept,convergenceTol,mType,inputMovable);
  else if(regTyp == L2)
    ret = par.template parallelize<T,I,O,svm_model<T>,
                                   hinge_gradient<T>, l2_regularizer<T>>
         (data,label,initModel,numIteration,alpha,regParam,
          isIntercept,convergenceTol,mType,inputMovable);
  return ret;
}

// --- main api with dense data support ---
template <class T>
svm_model<T>
svm_with_lbfgs::train (const colmajor_matrix<T>& data,
                       dvector<T>& label,
                       size_t numIteration,
                       double alpha,
                       size_t hist_size,
                       double regParam,
                       RegType regTyp,
                       bool isIntercept,
                       double convergenceTol) {
  size_t numFeatures = data.num_col;
  T intercept = isIntercept ? 1.0 : 0.0;
  svm_model<T> initModel(numFeatures,intercept);
  return train<T>(data,label,initModel,numIteration,alpha,hist_size,
                  regParam,regTyp,isIntercept,convergenceTol);
}

template <class T>
svm_model<T>
svm_with_lbfgs::train (const colmajor_matrix<T>& data,
                       dvector<T>& label,
                       svm_model<T>& initModel,
                       size_t numIteration,
                       double alpha,
                       size_t hist_size,
                       double regParam,
                       RegType regTyp,
                       bool isIntercept,
                       double convergenceTol) {
#ifdef _DEBUG_
  std::cout << "Initial model: \n";
  initModel.debug_print(); std::cout << "\n";
#endif

  auto& dmat = const_cast<colmajor_matrix<T>&> (data);
  lbfgs_parallelizer par(hist_size);
  svm_model<T> ret;

  if(regTyp == ZERO)
    ret = par.template parallelize<T,svm_model<T>,
                                   hinge_gradient<T>, zero_regularizer<T>>
         (dmat,label,initModel,numIteration,alpha,regParam,
          isIntercept,convergenceTol);
  else if(regTyp == L1)
    ret = par.template parallelize<T,svm_model<T>,
                                   hinge_gradient<T>, l1_regularizer<T>>
         (dmat,label,initModel,numIteration,alpha,regParam,
          isIntercept,convergenceTol);
  else if(regTyp == L2)
    ret = par.template parallelize<T,svm_model<T>,
                                   hinge_gradient<T>, l2_regularizer<T>>
         (dmat,label,initModel,numIteration,alpha,regParam,
          isIntercept,convergenceTol);
  return ret;
}

}
#endif
