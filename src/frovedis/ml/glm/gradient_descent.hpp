#ifndef _GRADIENT_DESCENT_HPP_
#define _GRADIENT_DESCENT_HPP_

#include <iostream>
#include <cmath>

namespace frovedis {

class gradient_descent {
public:
  gradient_descent (double al = 0.01, bool intercept = false) {
    checkAssumption(al > 0.0);
    alpha = al;
    isIntercept = intercept;
  }

  void debug_print() {
    std::cout << "alpha: " << alpha
              << ", isIntercept: " << isIntercept << "\n";
  } 

  template <class T, class DATA_MATRIX, class TRANS_MATRIX, 
            class MODEL, class GRADIENT>
  void optimize(
    std::vector<DATA_MATRIX>& data, 
    std::vector<TRANS_MATRIX>& trans,
    std::vector<std::vector<T>>& target,
    MODEL& model,
    GRADIENT& gradient,
    size_t iterCount);

  template <class T, class DATA_MATRIX, class MODEL, class GRADIENT>
  void optimize(
    std::vector<DATA_MATRIX>& data, 
    std::vector<std::vector<T>>& target,
    MODEL& model,
    GRADIENT& gradient,
    size_t iterCount);

  template <class T, class DATA_MATRIX, class MODEL, class GRADIENT>
  std::vector<T>
  compute_gradient(
    DATA_MATRIX& data,
    MODEL& model,    
    std::vector<T>& target,
    GRADIENT& gradient,
    T& bias);

  template <class T, class DATA_MATRIX, class MODEL, class GRADIENT>
  std::vector<T>
  compute_gradient(
    DATA_MATRIX& data,
    MODEL& model,    
    std::vector<T>& target,
    GRADIENT& gradient) {
    T bias;
    auto v = compute_gradient(data, model, target, gradient, bias);
    v.push_back(bias);
    return v;
  }

  template <class T, class DATA_MATRIX, class TRANS_MATRIX, 
            class MODEL, class GRADIENT>
  std::vector<T>
  compute_gradient(
    DATA_MATRIX& data,
    TRANS_MATRIX& trans,
    MODEL& model,    
    std::vector<T>& target,
    GRADIENT& gradient,
    T& bias);

  template <class T, class DATA_MATRIX, class TRANS_MATRIX, 
            class MODEL, class GRADIENT>
  std::vector<T>
  compute_gradient(
    DATA_MATRIX& data,
    TRANS_MATRIX& trans,
    MODEL& model,    
    std::vector<T>& target,
    GRADIENT& gradient) {
    T bias;
    auto v = compute_gradient(data, trans, model, target, gradient, bias);
    v.push_back(bias);
    return v;
  }

private:
  template <class T, class DATA_MATRIX, class MODEL>
  std::vector<T>
  compute_wtx (DATA_MATRIX& data,
               MODEL& model);

  template <class T, class MODEL>
  void update_model (MODEL& model,
                     const std::vector<T>& grad_vector,
                     size_t iterCount, T bias);
  
  double alpha;
  bool isIntercept;
  SERIALIZE(alpha, isIntercept)
};

template <class T, class DATA_MATRIX, class TRANS_MATRIX, 
          class MODEL, class GRADIENT>
void gradient_descent::optimize(
  std::vector<DATA_MATRIX>& data, 
  std::vector<TRANS_MATRIX>& trans,
  std::vector<std::vector<T>>& target,
  MODEL& model,
  GRADIENT& gradient,
  size_t iterCount) {

  for(size_t i = 0; i < data.size(); i++) {
    T bias;
    auto grad_vector = compute_gradient<T>(data[i], trans[i], 
                                           model, target[i], gradient, bias);
    update_model<T>(model, grad_vector, iterCount, bias);
#ifdef _DEBUG_
    std::cout <<"updatedModel: \n"; model.debug_print(); std::cout << "\n";
#endif
  }
}

template <class T, class DATA_MATRIX, class MODEL, class GRADIENT>
void gradient_descent::optimize(
  std::vector<DATA_MATRIX>& data, 
  std::vector<std::vector<T>>& target,
  MODEL& model,
  GRADIENT& gradient,
  size_t iterCount) {

  for(size_t i = 0; i < data.size(); i++) {
    T bias;
    auto grad_vector = compute_gradient<T>(data[i], model, target[i], gradient,
                                           bias);
    update_model<T>(model, grad_vector, iterCount, bias);
#ifdef _DEBUG_
    std::cout <<"updatedModel: \n"; model.debug_print(); std::cout << "\n";
#endif
  }
}

template <class T, class DATA_MATRIX, class MODEL, class GRADIENT>
std::vector<T> 
gradient_descent::compute_gradient(
  DATA_MATRIX& data,
  MODEL& model,    
  std::vector<T>& target,
  GRADIENT& gradient, 
  T& sum) {

  sum = 0.0;
  auto wtx = compute_wtx<T>(data, model);
  auto scalar_grad = gradient.compute(target, wtx);

  if(isIntercept) {
    T* sgradp = &scalar_grad[0];
    for(size_t i = 0; i < scalar_grad.size(); i++) sum += sgradp[i];
  }

  auto grad_vector = trans_mv(data, scalar_grad);
  return grad_vector;
}

template <class T, class DATA_MATRIX, class TRANS_MATRIX, 
          class MODEL, class GRADIENT>
std::vector<T> 
gradient_descent::compute_gradient(
  DATA_MATRIX& data,
  TRANS_MATRIX& trans,
  MODEL& model,    
  std::vector<T>& target,
  GRADIENT& gradient,
  T& sum) {

  sum = 0.0;
  auto wtx = compute_wtx<T>(data, model);
  auto scalar_grad = gradient.compute(target, wtx);

  if(isIntercept) {
    T* sgradp = &scalar_grad[0];
    for(size_t i = 0; i < scalar_grad.size(); i++) sum += sgradp[i];
  }

  auto grad_vector = trans * scalar_grad;
  return grad_vector;
}

template <class T, class DATA_MATRIX, class MODEL>
inline std::vector<T>
gradient_descent::compute_wtx (DATA_MATRIX& data,
                               MODEL& model) {
  auto wtx = data * model.weight;
  T* wtxp = &wtx[0];
  if(isIntercept) {
    for(size_t i = 0; i < wtx.size(); i++) {
      wtxp[i] += model.intercept;
    }
  }
  return wtx;
}

template <class T, class MODEL>
inline void gradient_descent::update_model(MODEL& model,
                                           const std::vector<T>& grad_vector,
                                           size_t iterCount,
                                           T bias) {
  T reducedAlpha = alpha/sqrt(iterCount);
  size_t n = model.weight.size();

  T* weightp = &model.weight[0];
  const T* gradp = &grad_vector[0];
  for(size_t i = 0; i < n; i++) {
    weightp[i] -= reducedAlpha * gradp[i];
  }

  if(isIntercept)  model.intercept -= reducedAlpha * bias;
}

}
#endif
