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

  template <class MODEL, class REGULARIZER>
  void update_model(MODEL& model, 
                    const std::vector<T>& hkgk,
                    REGULARIZER& rType,
                    size_t iterCount);

  template <class MODEL>
  void update_history(const MODEL& cur_model, 
                      std::vector<T>& cur_gradient,
                      size_t iterCount);

  template <class MODEL, class REGULARIZER>
  void optimize(std::vector<T>& grad_vector, 
                MODEL& model,
                size_t iterCount,
                REGULARIZER& rType);

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
  //checkAssumption(grad_vector.size() > 0);
  int n_elem = model_hist.totActiveElements;
  if (n_elem == 0) return grad_vector; 
  auto q = grad_vector;
  std::vector<T> al(n_elem);
  for(int i = n_elem - 1; i >= 0; --i) {
    al[i] = rho[i] * vector_dot(model_hist[i], q);
    q = q - (grad_hist[i] * al[i]);
  }
  int i = n_elem - 1; // k-1
  auto yk = rho[i] * vector_dot(grad_hist[i], grad_hist[i]);
  auto z = q * (static_cast<T>(1.0) / yk); // this scaling is not used in sklearn
  for(int i = 0; i < n_elem; ++i) {
    auto beta = rho[i] * vector_dot(grad_hist[i], z);
    z = z + (model_hist[i] * (al[i] - beta));
  }
  return z;
}

template <class T>
template <class MODEL, class REGULARIZER>
void lbfgs<T>::update_model(MODEL& model,
                            const std::vector<T>& hkgk,
                            REGULARIZER& rType,
                            size_t iterCount) {
  auto weightp = model.weight.data();
  auto hkgkp = hkgk.data();
  size_t nftr = model.weight.size();
  T reducedAlpha = alpha / sqrt(iterCount);
  for(size_t i = 0; i < nftr; ++i) weightp[i] -= reducedAlpha * hkgkp[i];

  // Updating bias (n+1 th) term of the model
  // n+1 th dimension of hkgk vector is for bias term 
  if(isIntercept) model.intercept -= reducedAlpha * hkgkp[nftr]; 

  // Regularizing model (if required)
  rType.regularize(model.weight);
}

template <class T>
template <class MODEL>
void lbfgs<T>::update_history(const MODEL& cur_model,
                              std::vector<T>& cur_gradient,
                              size_t iterCount) {
  auto cur_model_vec = cur_model.weight;
  cur_model_vec.push_back(cur_model.intercept);
  if (iterCount != 1) { //no history in first iteration
    auto model_diff = cur_model_vec - old_model;
    auto grad_diff  = cur_gradient - old_gradient;
    if (vector_count_nonzero(grad_diff) == 0) // all zeros 
      REPORT_INFO("No change in gradient, hence skipping history update!\n");
    else { 
      T dotval = vector_dot(model_diff, grad_diff);
      if(dotval > 0.0) {      // mandatory curvature condition for history update
        T one_by_dotval = static_cast<T>(1.0) / dotval;   // Pi := 1/Si.Yi
        rho.push_back(one_by_dotval);
        model_hist.push_back(std::move(model_diff));
        grad_hist.push_back(std::move(grad_diff));
      }
    }
  }
  old_model.swap(cur_model_vec);
  old_gradient.swap(cur_gradient);
}

template <class T>
template <class MODEL, class REGULARIZER>
void lbfgs<T>::optimize(std::vector<T>& grad_vector,
                        MODEL& model,
                        size_t iterCount,
                        REGULARIZER& rType) {
  frovedis::time_spent t(TRACE);
  std::vector<T> hkgk = compute_hkgk(grad_vector);
  t.show("compute hkgk: ");
  update_model(model,hkgk,rType,iterCount); // in-place model update using computed hkgk
  t.show("update model: ");
  update_history(model,grad_vector,iterCount); // 'model' is updated model
  t.show("update history: ");
#ifdef _LBFGS_DEBUG_
  std::cout << "iteration: " << iterCount << std::endl;
  std::cout << model_hist.totActiveElements << " "
            << grad_hist.totActiveElements  << " " 
            << rho.totActiveElements << std::endl;
  debug_print_vector(old_model, 10);
  debug_print_vector(old_gradient, 10);
#endif
}

}
#endif
