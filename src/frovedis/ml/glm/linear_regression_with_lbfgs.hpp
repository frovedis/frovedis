#ifndef _LNR_LBFGS_HPP_
#define _LNR_LBFGS_HPP_

#include "lbfgs_parallelizer.hpp"

namespace frovedis {

class linear_regression_with_lbfgs {
public:
  template <class T>
  static linear_regression_model<T> train (
    crs_matrix<T>& data,
    dvector<T>& label,
    size_t numIteration=1000, 
    T alpha=0.01, 
    size_t hist_size=10, 
    bool isIntercept=false,
    T convergenceTol=0.001, 
#if defined(_SX) || defined(__ve__)
    MatType mType = HYBRID 
#else
    MatType mType = CRS
#endif 
  );

  template <class T>
  static linear_regression_model<T> train (
    crs_matrix<T>&& data,
    dvector<T>& label,
    size_t numIteration=1000, 
    T alpha=0.01, 
    size_t hist_size=10, 
    bool isIntercept=false,
    T convergenceTol=0.001, 
#if defined(_SX) || defined(__ve__)
    MatType mType = HYBRID 
#else
    MatType mType = CRS
#endif 
  );

  template <class T>
  static linear_regression_model<T> train (
    crs_matrix<T>& data,
    dvector<T>& label,
    linear_regression_model<T>& lrm,
    size_t numIteration=1000, 
    T alpha=0.01, 
    size_t hist_size=10, 
    bool isIntercept=false,
    T convergenceTol=0.001, 
#if defined(_SX) || defined(__ve__)
    MatType mType = HYBRID, 
#else
    MatType mType = CRS,
#endif 
    bool inputMovable=false
  );
};

template <class T>
linear_regression_model<T>
linear_regression_with_lbfgs::train (crs_matrix<T>& data,
                                     dvector<T>& label,
                                     size_t numIteration,
                                     T alpha,
                                     size_t hist_size,
                                     bool isIntercept,
                                     T convergenceTol,
                                     MatType mType) {
  size_t numFeatures = data.num_col;
 
  if(!numFeatures)
    REPORT_ERROR(USER_ERROR,
                 "Incompatible Test Vector with only Single Column!!\n");
    
  T intercept = isIntercept ? 1.0 : 0.0;
  linear_regression_model<T> initModel(numFeatures,intercept);
  return train<T>(data,label,initModel,numIteration,alpha,hist_size,
                  isIntercept,convergenceTol,mType,false);
}

template <class T>
linear_regression_model<T>
linear_regression_with_lbfgs::train (crs_matrix<T>&& data,
                                     dvector<T>& label,
                                     size_t numIteration,
                                     T alpha,
                                     size_t hist_size,
                                     bool isIntercept,
                                     T convergenceTol,
                                     MatType mType) {
  size_t numFeatures = data.num_col;
 
  if(!numFeatures)
    REPORT_ERROR(USER_ERROR,
                 "Incompatible Test Vector with only Single Column!!\n");
    
  T intercept = isIntercept ? 1.0 : 0.0;
  linear_regression_model<T> initModel(numFeatures,intercept);
  return train<T>(data,label,initModel,numIteration,alpha,hist_size,
                  isIntercept,convergenceTol,mType,true);
}

template <class T>
linear_regression_model<T>
linear_regression_with_lbfgs::train (crs_matrix<T>& data,
                                     dvector<T>& label,
                                     linear_regression_model<T>& initModel,
                                     size_t numIteration,
                                     T alpha,
                                     size_t hist_size,
                                     bool isIntercept,
                                     T convergenceTol,
                                     MatType mType,
                                     bool inputMovable) {
#ifdef _DEBUG_
  std::cout << "Initial model: \n";
  initModel.debug_print(); std::cout << "\n";
#endif

  lbfgs_parallelizer<T> par(hist_size);
  return par.template parallelize<linear_regression_model<T>,
                                  linear_gradient<T>, zero_regularizer<T>>
         (data,label,initModel,numIteration,alpha,0.0,
          isIntercept,convergenceTol,mType,inputMovable);
}

}
#endif
