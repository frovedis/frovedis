#ifndef _SVM_SGD_HPP_
#define _SVM_SGD_HPP_

#include "sgd_parallelizer.hpp"

namespace frovedis {

class svm_with_sgd {
public:
  template <class T, class I, class O>
  static svm_model<T> train (
    crs_matrix<T,I,O>& data,
    dvector<T>& label,
    size_t numIteration=1000, 
    double alpha=0.01, 
    double miniBatchFraction=1.0, 
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
    std::vector<T>& sample_weight,
    size_t& n_iter,
    size_t numIteration=1000,
    double alpha=0.01,
    double miniBatchFraction=1.0,
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
    double miniBatchFraction=1.0, 
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
    std::vector<T>& sample_weight,
    size_t& n_iter,
    size_t numIteration=1000,
    double alpha=0.01,
    double miniBatchFraction=1.0,
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
    svm_model<T>& initModel,
    std::vector<T>& sample_weight,
    size_t& n_iter,
    size_t numIteration=1000, 
    double alpha=0.01, 
    double miniBatchFraction=1.0, 
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
    std::vector<T>& sample_weight,
    size_t& n_iter,
    size_t numIteration=1000, 
    double alpha=0.01, 
    double miniBatchFraction=1.0, 
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
    rowmajor_matrix<T>& data,
    dvector<T>& label,
    size_t numIteration=1000,
    double alpha=0.01,
    double miniBatchFraction=1.0,
    double regParam=0.01,
    RegType regTyp=ZERO,
    bool isIntercept=false,
    double convergenceTol=0.001);

  template <class T>
  static svm_model<T> train (
    rowmajor_matrix<T>& data,
    dvector<T>& label,
    std::vector<T>& sample_weight,
    size_t& n_iter,
    size_t numIteration=1000,
    double alpha=0.01,
    double miniBatchFraction=1.0,
    double regParam=0.01,
    RegType regTyp=ZERO,
    bool isIntercept=false,
    double convergenceTol=0.001);

  template <class T>
  static svm_model<T> train (
    const colmajor_matrix<T>& data,
    dvector<T>& label,
    size_t numIteration=1000, 
    double alpha=0.01, 
    double miniBatchFraction=1.0, 
    double regParam=0.01, 
    RegType regTyp=ZERO, 
    bool isIntercept=false,
    double convergenceTol=0.001); 

  template <class T>
  static svm_model<T> train (
    const colmajor_matrix<T>& data,
    dvector<T>& label,
    std::vector<T>& sample_weight,
    size_t& n_iter,
    size_t numIteration=1000,
    double alpha=0.01,
    double miniBatchFraction=1.0,
    double regParam=0.01,
    RegType regTyp=ZERO,
    bool isIntercept=false,
    double convergenceTol=0.001);

  template <class T>
  static svm_model<T> train (
    const colmajor_matrix<T>& data,
    dvector<T>& label,
    svm_model<T>& initModel,
    std::vector<T>& sample_weight,
    size_t& n_iter,
    size_t numIteration=1000, 
    double alpha=0.01, 
    double miniBatchFraction=1.0, 
    double regParam=0.01, 
    RegType regTyp=ZERO, 
    bool isIntercept=false,
    double convergenceTol=0.001); 
};

template <class T, class I, class O>
svm_model<T>
svm_with_sgd::train (crs_matrix<T,I,O>& data,
                     dvector<T>& label,
                     size_t numIteration,
                     double alpha,
                     double miniBatchFraction,
                     double regParam,
                     RegType regTyp,
                     bool isIntercept,
                     double convergenceTol,
                     MatType mType) {
  size_t numFeatures = data.num_col;
  svm_model<T> initModel(numFeatures);
  size_t n_iter = 0;
  std::vector<T> sample_weight;
  return train<T>(data,label,initModel,sample_weight,n_iter,numIteration,alpha,miniBatchFraction,
                  regParam,regTyp,isIntercept,convergenceTol,mType,false);
}

template <class T, class I, class O>
svm_model<T>
svm_with_sgd::train (crs_matrix<T,I,O>& data,
                     dvector<T>& label,
                     std::vector<T>& sample_weight,
                     size_t& n_iter,
                     size_t numIteration,
                     double alpha,
                     double miniBatchFraction,
                     double regParam,
                     RegType regTyp,
                     bool isIntercept,
                     double convergenceTol,
                     MatType mType) {
  size_t numFeatures = data.num_col;
  svm_model<T> initModel(numFeatures);
  return train<T>(data,label,initModel,sample_weight,n_iter,numIteration,alpha,miniBatchFraction,
                  regParam,regTyp,isIntercept,convergenceTol,mType,false);
}

template <class T, class I, class O>
svm_model<T>
svm_with_sgd::train (crs_matrix<T,I,O>&& data,
                     dvector<T>& label,
                     size_t numIteration,
                     double alpha,
                     double miniBatchFraction,
                     double regParam,
                     RegType regTyp,
                     bool isIntercept,
                     double convergenceTol,
                     MatType mType) {
  size_t numFeatures = data.num_col;
  svm_model<T> initModel(numFeatures);
  size_t n_iter = 0;
  std::vector<T> sample_weight;
  return train<T>(data,label,initModel,sample_weight,n_iter,numIteration,alpha,miniBatchFraction,
                  regParam,regTyp,isIntercept,convergenceTol,mType,true);
}

template <class T, class I, class O>
svm_model<T>
svm_with_sgd::train (crs_matrix<T,I,O>&& data,
                     dvector<T>& label,
                     std::vector<T>& sample_weight,
                     size_t& n_iter,
                     size_t numIteration,
                     double alpha,
                     double miniBatchFraction,
                     double regParam,
                     RegType regTyp,
                     bool isIntercept,
                     double convergenceTol,
                     MatType mType) {
  size_t numFeatures = data.num_col;
  svm_model<T> initModel(numFeatures);
  return train<T>(data,label,initModel,sample_weight,n_iter,numIteration,alpha,miniBatchFraction,
                  regParam,regTyp,isIntercept,convergenceTol,mType,true);
}

template <class T, class I, class O>
svm_model<T>
svm_with_sgd::train (crs_matrix<T,I,O>&& data,
                     dvector<T>& label,
                     svm_model<T>& initModel,
                     std::vector<T>& sample_weight,
                     size_t& n_iter,
                     size_t numIteration,
                     double alpha,
                     double miniBatchFraction,
                     double regParam,
                     RegType regTyp,
                     bool isIntercept,
                     double convergenceTol,
                     MatType mType) {
  return train<T>(data,label,initModel,sample_weight,n_iter,numIteration,alpha,miniBatchFraction,
                  regParam,regTyp,isIntercept,convergenceTol,mType,true);
}

// --- main api with sparse data support ---
template <class T, class I, class O>
svm_model<T>
svm_with_sgd::train (crs_matrix<T,I,O>& data,
                     dvector<T>& label,
                     svm_model<T>& initModel,
                     std::vector<T>& sample_weight,
                     size_t& n_iter,
                     size_t numIteration,
                     double alpha,
                     double miniBatchFraction,
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
  sgd_parallelizer par(miniBatchFraction);
  svm_model<T> ret;
  hinge_gradient<T> grad;

  if(regTyp == ZERO) {
    zero_regularizer<T> rType(regParam);
    ret = par.template parallelize<T,I,O,svm_model<T>,
                                   hinge_gradient<T>, zero_regularizer<T>>
         (data,label,initModel,grad,rType,sample_weight,n_iter,numIteration,alpha,
          isIntercept,convergenceTol,mType,inputMovable);
  }
  else if(regTyp == L1) {
    l1_regularizer<T> rType(regParam);
    ret = par.template parallelize<T,I,O,svm_model<T>,
                                   hinge_gradient<T>, l1_regularizer<T>>
         (data,label,initModel,grad,rType,sample_weight,n_iter,numIteration,alpha,
          isIntercept,convergenceTol,mType,inputMovable);
  }
  else if(regTyp == L2) {
    l2_regularizer<T> rType(regParam);
    ret = par.template parallelize<T,I,O,svm_model<T>,
                                   hinge_gradient<T>, l2_regularizer<T>>
         (data,label,initModel,grad,rType,sample_weight,n_iter,numIteration,alpha,
          isIntercept,convergenceTol,mType,inputMovable);
  }
  else REPORT_ERROR(USER_ERROR, "Unsupported regularizer!\n");
  return ret;
}

template <class T>
svm_model<T>
svm_with_sgd::train (rowmajor_matrix<T>& data,
                     dvector<T>& label,
                     size_t numIteration,
                     double alpha,
                     double miniBatchFraction,
                     double regParam,
                     RegType regTyp,
                     bool isIntercept,
                     double convergenceTol) {
  return train<T>(colmajor_matrix<T>(data),label,
                  numIteration,alpha,miniBatchFraction,
                  regParam,regTyp,isIntercept,convergenceTol);
}

template <class T>
svm_model<T>
svm_with_sgd::train (const colmajor_matrix<T>& data,
                     dvector<T>& label,
                     size_t numIteration,
                     double alpha,
                     double miniBatchFraction,
                     double regParam,
                     RegType regTyp,
                     bool isIntercept,
                     double convergenceTol) {
  size_t numFeatures = data.num_col;
  svm_model<T> initModel(numFeatures);
  size_t n_iter = 0;
  std::vector<T> sample_weight;
  return train<T>(data,label,initModel,sample_weight,n_iter,numIteration,alpha,miniBatchFraction,
                  regParam,regTyp,isIntercept,convergenceTol);
}

template <class T>
svm_model<T>
svm_with_sgd::train (const colmajor_matrix<T>& data,
                     dvector<T>& label,
                     std::vector<T>& sample_weight,
                     size_t& n_iter,
                     size_t numIteration,
                     double alpha,
                     double miniBatchFraction,
                     double regParam,
                     RegType regTyp,
                     bool isIntercept,
                     double convergenceTol) {
  size_t numFeatures = data.num_col;
  svm_model<T> initModel(numFeatures);
  return train<T>(data,label,initModel,sample_weight,n_iter,numIteration,alpha,miniBatchFraction,
                  regParam,regTyp,isIntercept,convergenceTol);
}

// --- main api with dense data support ---
template <class T>
svm_model<T>
svm_with_sgd::train (const colmajor_matrix<T>& data,
                     dvector<T>& label,
                     svm_model<T>& initModel,
                     std::vector<T>& sample_weight,
                     size_t& n_iter,
                     size_t numIteration,
                     double alpha,
                     double miniBatchFraction,
                     double regParam,
                     RegType regTyp,
                     bool isIntercept,
                     double convergenceTol) {
#ifdef _DEBUG_
  std::cout << "Initial model: \n";
  initModel.debug_print(); std::cout << "\n";
#endif

  if(sample_weight.empty()) sample_weight = vector_full<T>(data.num_row, 1);
  auto& dmat = const_cast<colmajor_matrix<T>&> (data);
  sgd_parallelizer par(miniBatchFraction);
  hinge_gradient<T> grad;
  svm_model<T> ret;

  if(regTyp == ZERO) {
    zero_regularizer<T> rType(regParam);
    ret = par.template parallelize<T,svm_model<T>,
                                   hinge_gradient<T>, zero_regularizer<T>>
         (dmat,label,initModel,grad,rType,sample_weight,n_iter,numIteration,alpha,
          isIntercept,convergenceTol);
  }
  else if(regTyp == L1) {
    l1_regularizer<T> rType(regParam);
    ret = par.template parallelize<T,svm_model<T>,
                                   hinge_gradient<T>, l1_regularizer<T>>
         (dmat,label,initModel,grad,rType,sample_weight,n_iter,numIteration,alpha,
          isIntercept,convergenceTol);
  }
  else if(regTyp == L2) {
    l2_regularizer<T> rType(regParam);
    ret = par.template parallelize<T,svm_model<T>,
                                   hinge_gradient<T>, l2_regularizer<T>>
         (dmat,label,initModel,grad,rType,sample_weight,n_iter,numIteration,alpha,
          isIntercept,convergenceTol);
  }
  else REPORT_ERROR(USER_ERROR, "Unsupported regularizer!\n");
  return ret;
}

}
#endif
