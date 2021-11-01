#ifndef _LBFGS_HPP_
#define _LBFGS_HPP_

#include "../../matrix/cir_array_list.hpp"

namespace frovedis {

template <class T>
struct lbfgs {
  explicit lbfgs(double al=0.01, size_t size=10, bool intercpt=false): 
    alpha(al), isIntercept(intercpt), 
    rho(size), model_hist(size), grad_hist(size) {
      checkAssumption(al > 0.0); 
  }
  lbfgs(const lbfgs& opt) {
    alpha = opt.alpha;
    isIntercept = opt.isIntercept;
    old_model = opt.old_model;
    old_gradient = opt.old_gradient;
    rho = opt.rho;
    model_hist = opt.model_hist;
    grad_hist = opt.grad_hist;
  }
  lbfgs(lbfgs&& opt) {
    alpha = opt.alpha;
    isIntercept = opt.isIntercept;
    old_model.swap(opt.old_model);
    old_gradient.swap(opt.old_gradient);
    rho = std::move(opt.rho);
    model_hist = std::move(opt.model_hist);
    grad_hist = std::move(opt.grad_hist);
    opt.clear();
  }
  lbfgs& operator=(const lbfgs& opt) {
    alpha = opt.alpha;
    isIntercept = opt.isIntercept;
    old_model = opt.old_model;
    old_gradient = opt.old_gradient;
    rho = opt.rho;
    model_hist = opt.model_hist;
    grad_hist = opt.grad_hist;
    return *this;
  }
  lbfgs& operator=(lbfgs&& opt) {
    alpha = opt.alpha;
    isIntercept = opt.isIntercept;
    old_model.swap(opt.old_model);
    old_gradient.swap(opt.old_gradient);
    rho = std::move(opt.rho);
    model_hist = std::move(opt.model_hist);
    grad_hist = std::move(opt.grad_hist);
    opt.clear();
    return *this;
  }
  void clear() {
    alpha = 0.0; isIntercept = false;
    old_model.clear();  old_gradient.clear(); 
    rho.clear();
    model_hist.clear(); grad_hist.clear();
  }
 
  std::vector<T> compute_hkgk(const std::vector<T>& grad_vector);

  template <class MODEL>
  void update_history(const MODEL& cur_model, 
                      std::vector<T>& cur_gradient);

  double alpha;
  bool isIntercept;
  std::vector<T> old_model, old_gradient;
  cir_array_list<T> rho;
  cir_array_list<std::vector<T>> model_hist, grad_hist;
  SERIALIZE(alpha,isIntercept,old_model,old_gradient,
            rho,model_hist,grad_hist)
};

// https://en.wikipedia.org/wiki/Limited-memory_BFGS#Algorithm
// https://github.com/scipy/scipy/blob/v1.6.0/scipy/optimize/lbfgsb.py#L458
template <class T>
std::vector<T>
lbfgs<T>::compute_hkgk(const std::vector<T>& grad_vector) {
  auto gsz = grad_vector.size();
  int n_elem = model_hist.totActiveElements;
  std::vector<T> z;
  // https://github.com/js850/lbfgs_cpp/blob/master/lbfgs.cpp#L167
  if (n_elem == 0) { // no history...
    z.resize(gsz);
    T H0 = 0.1;
    auto gnorm = vector_norm(grad_vector);
    if (gnorm > 1) gnorm = 1.0 / gnorm;
    for(size_t i = 0; i < gsz; ++i) z[i] = -gnorm * H0 * grad_vector[i];
  } else {
    auto q = grad_vector;
    std::vector<T> al(n_elem);
    for(int i = n_elem - 1; i >= 0; --i) {
      al[i] = rho[i] * vector_dot(model_hist[i], q);
      q = q - (grad_hist[i] * al[i]);
    }
    // https://github.com/js850/lbfgs_cpp/blob/master/lbfgs.cpp#L145
    int i = n_elem - 1; // k-1
    auto yy = vector_dot(grad_hist[i], grad_hist[i]);
    if (yy == 0) yy = 1.0;
    auto ys = 1.0 / rho[i];
    T H0 = ys / yy;
    z = q * H0; 
    for(int i = 0; i < n_elem; ++i) {
      auto beta = rho[i] * vector_dot(grad_hist[i], z);
      z = z + (model_hist[i] * (al[i] - beta));
    }
  }
  return z * static_cast<T>(-1.0); // invert the step to point downhill
}

template <class T>
template <class MODEL>
void lbfgs<T>::update_history(const MODEL& cur_model,
                              std::vector<T>& cur_gradient) {
  auto cur_model_vec = cur_model.weight;
  cur_model_vec.push_back(cur_model.intercept);
  auto model_diff = cur_model_vec - old_model;
  auto grad_diff  = cur_gradient - old_gradient;
  // --- to avoid numerical error on ve ---
  auto sz = grad_diff.size();
  auto dptr = grad_diff.data();
  for (size_t i = 0; i < sz; ++i) if (ABS(dptr[i]) < IGNORABLE_DIFF) dptr[i] = 0;
  // --------------------------------------
  T dotval = vector_dot(model_diff, grad_diff);
  // https://github.com/scipy/scipy/blob/v1.7.0/scipy/optimize/optimize.py#L1268
  //if (dotval == 0) dotval = 1000.0;
  // https://github.com/js850/lbfgs_cpp/blob/master/lbfgs.cpp#L139
  if (dotval == 0) dotval = 1;
  T one_by_dotval = static_cast<T>(1.0) / dotval;   // Pi := 1/Si.Yi
  rho.push_back(one_by_dotval);
  model_hist.push_back(std::move(model_diff));
  grad_hist.push_back(std::move(grad_diff));
  old_model.swap(cur_model_vec);
  old_gradient.swap(cur_gradient);
  /*
  std::cout << "grad-diff: \n"; grad_hist.debug_print();
  std::cout << "model-diff: \n"; model_hist.debug_print();
  std::cout << "rho: \n"; rho.debug_print();
  */
}

}
#endif
