#ifndef _RIDGE_LBFGS_HPP_
#define _RIDGE_LBFGS_HPP_

#include "lbfgs_parallelizer.hpp"

namespace frovedis {

class ridge_regression_with_lbfgs {
public:
  template <class T, class I, class O>
  static linear_regression_model<T> train (
    crs_matrix<T,I,O>& data,
    dvector<T>& label,
    size_t numIteration=1000, 
    double alpha=0.01, 
    size_t hist_size=10, 
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
    std::vector<T>& sample_weight,
    size_t& n_iter,
    size_t numIteration=1000,
    double alpha=0.01,
    size_t hist_size=10,
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
    size_t hist_size=10, 
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
    std::vector<T>& sample_weight,
    size_t& n_iter,
    size_t numIteration=1000,
    double alpha=0.01,
    size_t hist_size=10,
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
    std::vector<T>& sample_weight,
    size_t& n_iter,
    size_t numIteration=1000, 
    double alpha=0.01, 
    size_t hist_size=10, 
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
    std::vector<T>& sample_weight,
    size_t& n_iter,
    size_t numIteration=1000, 
    double alpha=0.01, 
    size_t hist_size=10, 
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
    size_t hist_size=10,
    double regParam=0.01,
    bool isIntercept=false,
    double convergenceTol=0.001, 
#if defined(_SX) || defined(__ve__)
    MatType mType = HYBRID 
#else
    MatType mType = CRS
#endif 
  );

  template <class T>
  static linear_regression_model<T> train (
    rowmajor_matrix<T>& data,
    dvector<T>& label,
    std::vector<T>& sample_weight,
    size_t& n_iter,
    size_t numIteration=1000,
    double alpha=0.01,
    size_t hist_size=10,
    double regParam=0.01,
    bool isIntercept=false,
    double convergenceTol=0.001, 
#if defined(_SX) || defined(__ve__)
    MatType mType = HYBRID 
#else
    MatType mType = CRS
#endif 
  );

  template <class T>
  static linear_regression_model<T> train (
    const colmajor_matrix<T>& data,
    dvector<T>& label,
    size_t numIteration=1000, 
    double alpha=0.01, 
    size_t hist_size=10, 
    double regParam=0.01, 
    bool isIntercept=false,
    double convergenceTol=0.001, 
#if defined(_SX) || defined(__ve__)
    MatType mType = HYBRID 
#else
    MatType mType = CRS
#endif 
  );

  template <class T>
  static linear_regression_model<T> train (
    const colmajor_matrix<T>& data,
    dvector<T>& label,
    std::vector<T>& sample_weight,
    size_t& n_iter,
    size_t numIteration=1000,
    double alpha=0.01,
    size_t hist_size=10,
    double regParam=0.01,
    bool isIntercept=false,
    double convergenceTol=0.001, 
#if defined(_SX) || defined(__ve__)
    MatType mType = HYBRID 
#else
    MatType mType = CRS
#endif 
  );

  template <class T>
  static linear_regression_model<T> train (
    const colmajor_matrix<T>& data,
    dvector<T>& label,
    linear_regression_model<T>& lrm,
    std::vector<T>& sample_weight,
    size_t& n_iter,
    size_t numIteration=1000, 
    double alpha=0.01, 
    size_t hist_size=10, 
    double regParam=0.01, 
    bool isIntercept=false,
    double convergenceTol=0.001, 
#if defined(_SX) || defined(__ve__)
    MatType mType = HYBRID, 
#else
    MatType mType = CRS,
#endif
    bool inputMovable = false 
  );
};

template <class T, class I, class O>
linear_regression_model<T>
ridge_regression_with_lbfgs::train (crs_matrix<T,I,O>& data,
                                    dvector<T>& label,
                                    size_t numIteration,
                                    double alpha,
                                    size_t hist_size,
                                    double regParam,
                                    bool isIntercept,
                                    double convergenceTol,
                                    MatType mType) {
  size_t numFeatures = data.num_col;
  T intercept = isIntercept ? 1.0 : 0.0;
  linear_regression_model<T> initModel(numFeatures,intercept);
  size_t n_iter = 0;
  std::vector<T> sample_weight;
  return train<T>(data,label,initModel,sample_weight,n_iter,numIteration,alpha,hist_size,
                  regParam,isIntercept,convergenceTol,mType,false);
}

template <class T, class I, class O>
linear_regression_model<T>
ridge_regression_with_lbfgs::train (crs_matrix<T,I,O>& data,
                                    dvector<T>& label,
                                    std::vector<T>& sample_weight,
                                    size_t& n_iter,
                                    size_t numIteration,
                                    double alpha,
                                    size_t hist_size,
                                    double regParam,
                                    bool isIntercept,
                                    double convergenceTol,
                                    MatType mType) {
  size_t numFeatures = data.num_col;
  T intercept = isIntercept ? 1.0 : 0.0;
  linear_regression_model<T> initModel(numFeatures,intercept);
  return train<T>(data,label,initModel,sample_weight,n_iter,numIteration,alpha,hist_size,
                  regParam,isIntercept,convergenceTol,mType,false);
}

template <class T, class I, class O>
linear_regression_model<T>
ridge_regression_with_lbfgs::train (crs_matrix<T,I,O>&& data,
                                    dvector<T>& label,
                                    size_t numIteration,
                                    double alpha,
                                    size_t hist_size,
                                    double regParam,
                                    bool isIntercept,
                                    double convergenceTol,
                                    MatType mType) {
  size_t numFeatures = data.num_col;
  T intercept = isIntercept ? 1.0 : 0.0;
  linear_regression_model<T> initModel(numFeatures,intercept);
  size_t n_iter = 0;
  std::vector<T> sample_weight;
  return train<T>(data,label,initModel,sample_weight,n_iter,numIteration,alpha,hist_size,
                  regParam,isIntercept,convergenceTol,mType,true);
}

template <class T, class I, class O>
linear_regression_model<T>
ridge_regression_with_lbfgs::train (crs_matrix<T,I,O>&& data,
                                    dvector<T>& label,
                                    std::vector<T>& sample_weight,
                                    size_t& n_iter,
                                    size_t numIteration,
                                    double alpha,
                                    size_t hist_size,
                                    double regParam,
                                    bool isIntercept,
                                    double convergenceTol,
                                    MatType mType) {
  size_t numFeatures = data.num_col;
  T intercept = isIntercept ? 1.0 : 0.0;
  linear_regression_model<T> initModel(numFeatures,intercept);
  return train<T>(data,label,initModel,sample_weight,n_iter,numIteration,alpha,hist_size,
                  regParam,isIntercept,convergenceTol,mType,true);
}

template <class T, class I, class O>
linear_regression_model<T>
ridge_regression_with_lbfgs::train (crs_matrix<T,I,O>&& data,
                                    dvector<T>& label,
                                    linear_regression_model<T>& initModel,
                                    std::vector<T>& sample_weight,
                                    size_t& n_iter,
                                    size_t numIteration,
                                    double alpha,
                                    size_t hist_size,
                                    double regParam,
                                    bool isIntercept,
                                    double convergenceTol,
                                    MatType mType) {
  return train<T>(data,label,initModel,sample_weight,n_iter,numIteration,alpha,hist_size,
                  regParam,isIntercept,convergenceTol,mType,true);
}

// --- main api with sparse data support ---
template <class T, class I, class O>
linear_regression_model<T>
ridge_regression_with_lbfgs::train (crs_matrix<T,I,O>& data,
                                    dvector<T>& label,
                                    linear_regression_model<T>& initModel,
                                    std::vector<T>& sample_weight,
                                    size_t& n_iter,
                                    size_t numIteration,
                                    double alpha,
                                    size_t hist_size,
                                    double regParam,
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
  return par.template parallelize<T,I,O,linear_regression_model<T>,
                                  linear_gradient<T>, l2_regularizer<T>>
         (data,label,initModel,sample_weight,n_iter,numIteration,alpha,regParam,
          isIntercept,convergenceTol,mType,inputMovable);
}

template <class T>
linear_regression_model<T>
ridge_regression_with_lbfgs::train (rowmajor_matrix<T>& data,
                                    dvector<T>& label,
                                    size_t numIteration,
                                    double alpha,
                                    size_t hist_size,
                                    double regParam,
                                    bool isIntercept,
                                    double convergenceTol,
                                    MatType mType) {
  return train<T>(colmajor_matrix<T>(data),label,
                  numIteration,alpha,hist_size,
                  regParam,isIntercept,convergenceTol,mType);
}

template <class T>
linear_regression_model<T>
ridge_regression_with_lbfgs::train (const colmajor_matrix<T>& data,
                                    dvector<T>& label,
                                    size_t numIteration,
                                    double alpha,
                                    size_t hist_size,
                                    double regParam,
                                    bool isIntercept,
                                    double convergenceTol,
                                    MatType mType) {
  size_t numFeatures = data.num_col;
  T intercept = isIntercept ? 1.0 : 0.0;
  linear_regression_model<T> initModel(numFeatures,intercept);
  size_t n_iter = 0;
  std::vector<T> sample_weight;
  return train<T>(data,label,initModel,sample_weight,n_iter,numIteration,alpha,hist_size,
                  regParam,isIntercept,convergenceTol,mType);
}

template <class T>
linear_regression_model<T>
ridge_regression_with_lbfgs::train (const colmajor_matrix<T>& data,
                                    dvector<T>& label,
                                    std::vector<T>& sample_weight,
                                    size_t& n_iter,
                                    size_t numIteration,
                                    double alpha,
                                    size_t hist_size,
                                    double regParam,
                                    bool isIntercept,
                                    double convergenceTol,
                                    MatType mType) {
  size_t numFeatures = data.num_col;
  T intercept = isIntercept ? 1.0 : 0.0;
  linear_regression_model<T> initModel(numFeatures,intercept);
  return train<T>(data,label,initModel,sample_weight,n_iter,numIteration,alpha,hist_size,
                  regParam,isIntercept,convergenceTol,mType);
}

// --- main api with dense data support ---
template <class T>
linear_regression_model<T>
ridge_regression_with_lbfgs::train (const colmajor_matrix<T>& data,
                                    dvector<T>& label,
                                    linear_regression_model<T>& initModel,
                                    std::vector<T>& sample_weight,
                                    size_t& n_iter,
                                    size_t numIteration,
                                    double alpha,
                                    size_t hist_size,
                                    double regParam,
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
  return par.template parallelize<T,linear_regression_model<T>,
                                  linear_gradient<T>, l2_regularizer<T>>
         (dmat,label,initModel,sample_weight,n_iter,numIteration,alpha,regParam,
          isIntercept,convergenceTol);
}

}
#endif
