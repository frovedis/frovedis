#ifndef _LR_SOFTMAX_HPP_
#define _LR_SOFTMAX_HPP_

#include "softmax_parallelizer.hpp"

namespace frovedis {

class multinomial_logistic_regression {
public:
  template <class T, class I, class O>
  static multinomial_logistic_regression_model<T> train(
    crs_matrix<T,I,O>& data,
    dvector<T>& label,
    size_t numIteration=1000, 
    double alpha=0.01, 
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
  static multinomial_logistic_regression_model<T> train(
    crs_matrix<T,I,O>& data,
    dvector<T>& label,
    std::vector<T>& sample_weight,
    size_t& n_iter,
    size_t numIteration=1000,
    double alpha=0.01,
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
  static multinomial_logistic_regression_model<T> train (
    crs_matrix<T,I,O>&& data,
    dvector<T>& label,
    size_t numIteration=1000, 
    double alpha=0.01, 
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
  static multinomial_logistic_regression_model<T> train (
    crs_matrix<T,I,O>&& data,
    dvector<T>& label,
    std::vector<T>& sample_weight,
    size_t& n_iter,
    size_t numIteration=1000,
    double alpha=0.01,
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
  static multinomial_logistic_regression_model<T> train(
    crs_matrix<T,I,O>&& data,
    dvector<T>& label,
    multinomial_logistic_regression_model<T>& lrm,
    std::vector<T>& sample_weight,
    size_t& n_iter,
    size_t numIteration=1000, 
    double alpha=0.01, 
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
  static multinomial_logistic_regression_model<T> train(
    crs_matrix<T,I,O>& data,
    dvector<T>& label,
    multinomial_logistic_regression_model<T>& lrm,
    std::vector<T>& sample_weight,
    size_t& n_iter,
    size_t numIteration=1000, 
    double alpha=0.01, 
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

  template <class T>
  static multinomial_logistic_regression_model<T> train(
    colmajor_matrix<T>& data,
    dvector<T>& label,
    size_t numIteration=1000,
    double alpha=0.01,
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
  static multinomial_logistic_regression_model<T> train(
    colmajor_matrix<T>& data,
    dvector<T>& label,
    std::vector<T>& sample_weight,
    size_t& n_iter,
    size_t numIteration=1000,
    double alpha=0.01,
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
  static multinomial_logistic_regression_model<T> train(
    colmajor_matrix<T>&& data,
    dvector<T>& label,
    size_t numIteration=1000,
    double alpha=0.01,
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
  static multinomial_logistic_regression_model<T> train(
    colmajor_matrix<T>&& data,
    dvector<T>& label,
    std::vector<T>& sample_weight,
    size_t& n_iter,
    size_t numIteration=1000,
    double alpha=0.01,
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
  static multinomial_logistic_regression_model<T> train(
    colmajor_matrix<T>&& data,
    dvector<T>& label,
    multinomial_logistic_regression_model<T>& lrm,
    std::vector<T>& sample_weight,
    size_t& n_iter,
    size_t numIteration=1000,
    double alpha=0.01,
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
  static multinomial_logistic_regression_model<T> train(
    colmajor_matrix<T>& data,
    dvector<T>& label,
    multinomial_logistic_regression_model<T>& lrm,
    std::vector<T>& sample_weight,
    size_t& n_iter,
    size_t numIteration=1000,
    double alpha=0.01,
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
};

template <class T>
size_t compute_nclasses (dvector<T>& label) {
  return vector_unique(label.gather()).size();
}

template <class T, class I, class O>
multinomial_logistic_regression_model<T>
multinomial_logistic_regression::train (crs_matrix<T,I,O>& data,
                                     dvector<T>& label,
                                     size_t numIteration,
                                     double alpha,
                                     double regParam,
                                     RegType regTyp,
                                     bool isIntercept,
                                     double convergenceTol,
                                     MatType mType) {
  size_t nfeatures = data.num_col;
  size_t nclasses = compute_nclasses(label);
  multinomial_logistic_regression_model<T> initModel(nfeatures,nclasses,isIntercept);
  size_t n_iter = 0;
  std::vector<T> sample_weight;
  return train<T,I,O>(data,label,initModel,sample_weight,n_iter,numIteration,alpha,
                  regParam,regTyp,isIntercept,convergenceTol,mType,false);
}

template <class T, class I, class O>
multinomial_logistic_regression_model<T>
multinomial_logistic_regression::train (crs_matrix<T,I,O>& data,
                                     dvector<T>& label,
                                     std::vector<T>& sample_weight,
                                     size_t& n_iter,
                                     size_t numIteration,
                                     double alpha,
                                     double regParam,
                                     RegType regTyp,
                                     bool isIntercept,
                                     double convergenceTol,
                                     MatType mType) {
  size_t nfeatures = data.num_col;
  size_t nclasses = compute_nclasses(label);
  multinomial_logistic_regression_model<T> initModel(nfeatures,nclasses,isIntercept);
  return train<T,I,O>(data,label,initModel,sample_weight,n_iter,numIteration,alpha,
                  regParam,regTyp,isIntercept,convergenceTol,mType,false);
}

template <class T, class I, class O>
multinomial_logistic_regression_model<T>
multinomial_logistic_regression::train (crs_matrix<T,I,O>&& data,
                                     dvector<T>& label,
                                     size_t numIteration,
                                     double alpha,
                                     double regParam,
                                     RegType regTyp,
                                     bool isIntercept,
                                     double convergenceTol,
                                     MatType mType) {
  size_t nfeatures = data.num_col;
  size_t nclasses = compute_nclasses(label);
  multinomial_logistic_regression_model<T> initModel(nfeatures,nclasses,isIntercept);
  size_t n_iter = 0;
  std::vector<T> sample_weight;
  return train<T,I,O>(data,label,initModel,sample_weight,n_iter,numIteration,alpha,
                  regParam,regTyp,isIntercept,convergenceTol,mType,true);
}

template <class T, class I, class O>
multinomial_logistic_regression_model<T>
multinomial_logistic_regression::train (crs_matrix<T,I,O>&& data,
                                     dvector<T>& label,
                                     std::vector<T>& sample_weight,
                                     size_t& n_iter,
                                     size_t numIteration,
                                     double alpha,
                                     double regParam,
                                     RegType regTyp,
                                     bool isIntercept,
                                     double convergenceTol,
                                     MatType mType) {
  size_t nfeatures = data.num_col;
  size_t nclasses = compute_nclasses(label);
  multinomial_logistic_regression_model<T> initModel(nfeatures,nclasses,isIntercept);
  return train<T,I,O>(data,label,initModel,sample_weight,n_iter,numIteration,alpha,
                  regParam,regTyp,isIntercept,convergenceTol,mType,true);
}

template <class T, class I, class O>
multinomial_logistic_regression_model<T>
multinomial_logistic_regression::train (crs_matrix<T,I,O>&& data,
                                     dvector<T>& label,
                                     multinomial_logistic_regression_model<T>& initModel,
                                     std::vector<T>& sample_weight,
                                     size_t& n_iter,
                                     size_t numIteration,
                                     double alpha,
                                     double regParam,
                                     RegType regTyp,
                                     bool isIntercept,
                                     double convergenceTol,
                                     MatType mType) {
  return train<T,I,O>(data,label,initModel,sample_weight,n_iter,numIteration,alpha,
                  regParam,regTyp,isIntercept,convergenceTol,mType,true);
}

// --- main api with sparse data support ---
template <class T, class I, class O>
multinomial_logistic_regression_model<T>
multinomial_logistic_regression::train (crs_matrix<T,I,O>& data,
                                     dvector<T>& label,
                                     multinomial_logistic_regression_model<T>& initModel,
                                     std::vector<T>& sample_weight,
                                     size_t& n_iter,
                                     size_t numIteration,
                                     double alpha,
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
  softmax_parallelizer par;
  multinomial_logistic_regression_model<T> ret;

  if (regTyp == ZERO)
  
    ret = par.template parallelize<T,I,O,multinomial_logistic_regression_model<T>,
                                   zero_regularizer<T>> 
          (data,label,initModel,sample_weight,n_iter,numIteration,alpha,regParam,
           isIntercept,convergenceTol,mType,inputMovable);
  else if (regTyp == L1)
    ret = par.template parallelize<T,I,O,multinomial_logistic_regression_model<T>,
                                   l1_regularizer<T>> 
          (data,label,initModel,sample_weight,n_iter,numIteration,alpha,regParam,
           isIntercept,convergenceTol,mType,inputMovable);
  else if (regTyp == L2)
    ret = par.template parallelize<T,I,O,multinomial_logistic_regression_model<T>,
                                   l2_regularizer<T>> 
          (data,label,initModel,sample_weight,n_iter,numIteration,alpha,regParam,
           isIntercept,convergenceTol,mType,inputMovable);
  return ret;
}

template <class T>
multinomial_logistic_regression_model<T>
multinomial_logistic_regression::train (colmajor_matrix<T>& data,
                                     dvector<T>& label,
                                     size_t numIteration,
                                     double alpha,
                                     double regParam,
                                     RegType regTyp,
                                     bool isIntercept,
                                     double convergenceTol,
                                     MatType mType) {
  size_t nfeatures = data.num_col;
  size_t nclasses = compute_nclasses(label);
  multinomial_logistic_regression_model<T> initModel(nfeatures,nclasses,isIntercept);
  size_t n_iter = 0;
  std::vector<T> sample_weight;
  return train<T>(data,label,initModel,sample_weight,n_iter,numIteration,alpha,
                  regParam,regTyp,isIntercept,convergenceTol,mType);
}

template <class T>
multinomial_logistic_regression_model<T>
multinomial_logistic_regression::train (colmajor_matrix<T>& data,
                                     dvector<T>& label,
                                     std::vector<T>& sample_weight,
                                     size_t& n_iter,
                                     size_t numIteration,
                                     double alpha,
                                     double regParam,
                                     RegType regTyp,
                                     bool isIntercept,
                                     double convergenceTol,
                                     MatType mType) {
  size_t nfeatures = data.num_col;
  size_t nclasses = compute_nclasses(label);
  multinomial_logistic_regression_model<T> initModel(nfeatures,nclasses,isIntercept);
  return train<T>(data,label,initModel,sample_weight,n_iter,numIteration,alpha,
                  regParam,regTyp,isIntercept,convergenceTol,mType);
}

template <class T>
multinomial_logistic_regression_model<T>
multinomial_logistic_regression::train (colmajor_matrix<T>&& data,
                                     dvector<T>& label,
                                     size_t numIteration,
                                     double alpha,
                                     double regParam,
                                     RegType regTyp,
                                     bool isIntercept,
                                     double convergenceTol,
                                     MatType mType) {
  size_t nfeatures = data.num_col;
  size_t nclasses = compute_nclasses(label);
  multinomial_logistic_regression_model<T> initModel(nfeatures,nclasses,isIntercept);
  size_t n_iter = 0;
  std::vector<T> sample_weight;
  return train<T>(data,label,initModel,sample_weight,n_iter,numIteration,alpha,
                  regParam,regTyp,isIntercept,convergenceTol,mType,true);
}

template <class T>
multinomial_logistic_regression_model<T>
multinomial_logistic_regression::train (colmajor_matrix<T>&& data,
                                     dvector<T>& label,
                                     std::vector<T>& sample_weight,
                                     size_t& n_iter,
                                     size_t numIteration,
                                     double alpha,
                                     double regParam,
                                     RegType regTyp,
                                     bool isIntercept,
                                     double convergenceTol,
                                     MatType mType) {
  size_t nfeatures = data.num_col;
  size_t nclasses = compute_nclasses(label);
  multinomial_logistic_regression_model<T> initModel(nfeatures,nclasses,isIntercept);
  return train<T>(data,label,initModel,sample_weight,n_iter,numIteration,alpha,
                  regParam,regTyp,isIntercept,convergenceTol,mType,true);
}

template <class T>
multinomial_logistic_regression_model<T>
multinomial_logistic_regression::train (colmajor_matrix<T>&& data,
                                     dvector<T>& label,
                                     multinomial_logistic_regression_model<T>& initModel,
                                     std::vector<T>& sample_weight,
                                     size_t& n_iter,
                                     size_t numIteration,
                                     double alpha,
                                     double regParam,
                                     RegType regTyp,
                                     bool isIntercept,
                                     double convergenceTol,
                                     MatType mType) {
  return train<T>(data,label,initModel,sample_weight,n_iter,numIteration,alpha,
                  regParam,regTyp,isIntercept,convergenceTol,mType,true);
}

template <class T>
multinomial_logistic_regression_model<T>
multinomial_logistic_regression::train (colmajor_matrix<T>& data,
                                     dvector<T>& label,
                                     multinomial_logistic_regression_model<T>& initModel,
                                     std::vector<T>& sample_weight,
                                     size_t& n_iter,
                                     size_t numIteration,
                                     double alpha,
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
  softmax_parallelizer par;
  multinomial_logistic_regression_model<T> ret;

  if (regTyp == ZERO)
    ret = par.template parallelize<T,multinomial_logistic_regression_model<T>,
                                   zero_regularizer<T>>
          (data,label,initModel,sample_weight,n_iter,numIteration,alpha,regParam,
           isIntercept,convergenceTol);
  else if (regTyp == L1)
    ret = par.template parallelize<T,multinomial_logistic_regression_model<T>,
                                   l1_regularizer<T>>
          (data,label,initModel,sample_weight,n_iter,numIteration,alpha,regParam,
           isIntercept,convergenceTol);
  else if (regTyp == L2)
    ret = par.template parallelize<T,multinomial_logistic_regression_model<T>,
                                   l2_regularizer<T>>
          (data,label,initModel,sample_weight,n_iter,numIteration,alpha,
           regParam,isIntercept,convergenceTol);
  return ret;
}

}
#endif
