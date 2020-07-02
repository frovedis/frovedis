#ifndef _GRADIENT_DESCENT_HPP_
#define _GRADIENT_DESCENT_HPP_

#include <iostream>
#include <cmath>

//#define NORMALIZE_GRADIENT

namespace frovedis {

template <class T>
void normalize_gradient_inplace (std::vector<T>& grad_vector) { 
  auto mag = nrm2<T>(grad_vector);
  if (mag != 0) {
    auto one_by_mag = 1.0 / mag;
    auto gradvecp = grad_vector.data();
    for(size_t i = 0; i < grad_vector.size(); ++i) gradvecp[i] *= one_by_mag;
  }
}

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
    size_t iterCount,
    double& loss);

  template <class T, class DATA_MATRIX, class MODEL, class GRADIENT>
  void optimize(
    std::vector<DATA_MATRIX>& data, 
    std::vector<std::vector<T>>& target,
    MODEL& model,
    GRADIENT& gradient,
    size_t iterCount,
    double& loss);

  template <class T, class DATA_MATRIX, class MODEL, class GRADIENT>
  std::vector<T>
  compute_gradient(
    DATA_MATRIX& data,
    MODEL& model,    
    std::vector<T>& target,
    GRADIENT& gradient,
    T& bias, double& loss);

  template <class T, class DATA_MATRIX, class MODEL, class GRADIENT>
  std::vector<T>
  compute_gradient(
    DATA_MATRIX& data,
    MODEL& model,    
    std::vector<T>& target,
    GRADIENT& gradient) {
    T bias = 0.0;
    double loss = 0.0;
    auto v = compute_gradient(data, model, target, gradient, bias, loss);
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
    T& bias, double& loss);

  template <class T, class DATA_MATRIX, class TRANS_MATRIX, 
            class MODEL, class GRADIENT>
  std::vector<T>
  compute_gradient(
    DATA_MATRIX& data,
    TRANS_MATRIX& trans,
    MODEL& model,    
    std::vector<T>& target,
    GRADIENT& gradient) {
    T bias = 0.0; 
    double loss = 0.0;
    auto v = compute_gradient(data, trans, model, target, gradient, bias, loss);
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
  size_t iterCount,
  double& loss) {
  T bias = 0.0; 
  loss = 0.0;
  for(size_t i = 0; i < data.size(); i++) {
    auto grad_vector = compute_gradient<T>(data[i], trans[i], model, target[i], 
                                           gradient, bias, loss);
    update_model<T>(model, grad_vector, iterCount, bias);
  }
}

template <class T, class DATA_MATRIX, class MODEL, class GRADIENT>
void gradient_descent::optimize(
  std::vector<DATA_MATRIX>& data, 
  std::vector<std::vector<T>>& target,
  MODEL& model,
  GRADIENT& gradient,
  size_t iterCount,
  double& loss) {
  T bias = 0.0; 
  loss = 0.0;
  for(size_t i = 0; i < data.size(); i++) {
    auto grad_vector = compute_gradient<T>(data[i], model, target[i], 
                                           gradient, bias, loss);
    update_model<T>(model, grad_vector, iterCount, bias);
  }
}

template <class T, class DATA_MATRIX, class MODEL, class GRADIENT>
std::vector<T> 
gradient_descent::compute_gradient(
  DATA_MATRIX& data,
  MODEL& model,    
  std::vector<T>& target,
  GRADIENT& gradient, 
  T& bias, double& loss) {
  auto wtx = compute_wtx<T>(data, model);
  auto dloss = gradient.compute_dloss(target, wtx, loss);
  if(isIntercept) {
    bias = 0.0;
    auto dlossp = dloss.data();
    for(size_t i = 0; i < dloss.size(); i++) bias += dlossp[i];
  }
  auto grad_vector = trans_mv(data, dloss);
  //auto gradv = grad_vector.data();
  //double one_by_nsamples = 1.0 / data.local_num_row;
  //for(size_t i = 0; i < grad_vector.size(); ++i) gradv[i] *= one_by_nsamples;
  if (get_loglevel() == TRACE && get_selfid() == 0) {
    std::cout << "[rank 0] loss: " << loss << std::endl;
    std::cout << "[rank 0] dloss: "; debug_print_vector(dloss, 3);
    std::cout << "[rank 0] computed grad: "; debug_print_vector(grad_vector, 3);
  }
#ifdef NORMALIZE_GRADIENT
  normalize_gradient_inplace(grad_vector, bias, isIntercept);
  //std::cout << "[rank 0] normalize grad: "; debug_print_vector(grad_vector, 3);
#endif
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
  T& bias, double& loss) {
  auto wtx = compute_wtx<T>(data, model);
  auto dloss = gradient.compute_dloss(target, wtx, loss);
  if(isIntercept) {
    bias = 0.0;
    auto dlossp = dloss.data();
    for(size_t i = 0; i < dloss.size(); i++) bias += dlossp[i];
  }
  auto grad_vector = trans * dloss;
  //auto gradv = grad_vector.data();
  //double one_by_nsamples = 1.0 / data.local_num_row;
  //for(size_t i = 0; i < grad_vector.size(); ++i) gradv[i] *= one_by_nsamples;
  if (get_loglevel() == TRACE && get_selfid() == 0) {
    std::cout << "[rank 0] loss: " << loss << std::endl;
    std::cout << "[rank 0] dloss: "; debug_print_vector(dloss, 3);
    std::cout << "[rank 0] computed grad: "; debug_print_vector(grad_vector, 3);
  }
#ifdef NORMALIZE_GRADIENT
  normalize_gradient_inplace(grad_vector, bias, isIntercept);
  //std::cout << "[rank 0] normalize grad: "; debug_print_vector(grad_vector, 3);
#endif
  return grad_vector;
}

template <class T, class DATA_MATRIX, class MODEL>
inline std::vector<T>
gradient_descent::compute_wtx (DATA_MATRIX& data,
                               MODEL& model) {
  auto wtx = data * model.weight;
  if (get_loglevel() == TRACE && get_selfid() == 0) {
    std::cout << "[rank 0] wtx: "; debug_print_vector(wtx, 3);
  }
  auto wtxp = wtx.data();
  if(isIntercept) {
    for(size_t i = 0; i < wtx.size(); i++) wtxp[i] += model.intercept;
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
  auto weightp = model.weight.data();
  auto gradp = grad_vector.data();
  if (get_loglevel() == TRACE && get_selfid() == 0) {
    std::cout << "[rank 0] (before update) weight: "; debug_print_vector(model.weight, 3);
    std::cout << "[rank 0] (before update) intercept: " << model.intercept 
              << ", bias: " << bias
              << ", eta: " << reducedAlpha << std::endl;
  }
  for(size_t i = 0; i < n; i++) weightp[i] -= reducedAlpha * gradp[i];
  if(isIntercept)               model.intercept -= reducedAlpha * bias;
}

}
#endif
