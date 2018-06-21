#ifndef _LBFGS_HPP_
#define _LBFGS_HPP_

#include "../../matrix/pblas_wrapper.hpp"
#include "../../matrix/cir_array_list.hpp"

namespace frovedis {

template <class T>
struct lbfgs {
  explicit lbfgs(T al=0.01, size_t size=10, bool intercpt=false): 
    alpha(al), isIntercept(intercpt), 
    one_by_dot(size), model_hist(size), grad_hist(size)
      { old_model.resize(1); old_gradient.resize(1); checkAssumption(al>0.0 && size>0); }
  lbfgs(const lbfgs& opt) {
    alpha = opt.alpha;
    isIntercept = opt.isIntercept;
    old_model = opt.old_model;
    old_gradient = opt.old_gradient;
    one_by_dot = opt.one_by_dot;
    model_hist = opt.model_hist;
    grad_hist = opt.grad_hist;
  }
  lbfgs(lbfgs&& opt) {
    alpha = opt.alpha;
    isIntercept = opt.isIntercept;
    old_model.swap(opt.old_model);
    old_gradient.swap(opt.old_gradient);
    one_by_dot = std::move(opt.one_by_dot);
    model_hist = std::move(opt.model_hist);
    grad_hist = std::move(opt.grad_hist);
    opt.clear();
  }
  lbfgs& operator=(const lbfgs& opt) {
    alpha = opt.alpha;
    isIntercept = opt.isIntercept;
    old_model = opt.old_model;
    old_gradient = opt.old_gradient;
    one_by_dot = opt.one_by_dot;
    model_hist = opt.model_hist;
    grad_hist = opt.grad_hist;
    return *this;
  }
  lbfgs& operator=(lbfgs&& opt) {
    alpha = opt.alpha;
    isIntercept = opt.isIntercept;
    old_model.swap(opt.old_model);
    old_gradient.swap(opt.old_gradient);
    one_by_dot = std::move(opt.one_by_dot);
    model_hist = std::move(opt.model_hist);
    grad_hist = std::move(opt.grad_hist);
    opt.clear();
    return *this;
  }
  void clear() {
    alpha = 0.0; isIntercept = false;
    old_model.clear();  old_gradient.clear(); 
    one_by_dot.clear();
    model_hist.clear(); grad_hist.clear();
  }
 
  std::vector<T> compute_hkgk(const std::vector<T>& grad_vector);

  template <class MODEL, class REGULARIZER>
  void update_model(MODEL& model, const std::vector<T>& hkgk,
                    REGULARIZER& rType);

  template <class MODEL>
  void update_history(const MODEL& cur_model, std::vector<T>& cur_gradient,
                      size_t iterCount);

  template <class MODEL, class REGULARIZER>
  void optimize(std::vector<T>& grad_vector, MODEL& model,
                size_t iterCount,
                REGULARIZER& rType);

  T alpha;
  bool isIntercept;
  std::vector<T> old_model, old_gradient;
  cir_array_list<T> one_by_dot;
  cir_array_list<blockcyclic_matrix<T>> model_hist, grad_hist;
  // [Query] Do we need to serialize the lbfgs object??
  SERIALIZE(alpha,isIntercept,old_model,old_gradient,
            one_by_dot,model_hist,grad_hist)
};

template <class T>
std::vector<T>
lbfgs<T>::compute_hkgk(const std::vector<T>& grad_vector) {
  checkAssumption(grad_vector.size() > 0);
  //size_t n_elem = model_hist.totActiveElements;
  int n_elem = model_hist.totActiveElements;
  // quick return, if there are no history records
  if (!n_elem) return grad_vector; 

  std::vector<T> ai(n_elem,0);
  T* aip = &ai[0];
  blockcyclic_matrix<T> tmp_vec;
  blockcyclic_matrix<T> dist_q(grad_vector); // (lvalue) vec -> bcm
  // This scaling won't be required, since model update formula
  // takes care of this negative move (Wn+1 := Wn - HKGKn)
  //scal<T>(dist_q,-1.0); // q := -grad_vector

  // buggy: as 'size_t' will cause an infinite loop
  //for(size_t i = n_elem-1; i>=0; i--) {
  for(int i = n_elem-1; i>=0; i--) {
    tmp_vec = model_hist[i];         // tmp_vec := Si (copy op=)
    scal<T>(tmp_vec,one_by_dot[i]);  // Si*Pi (scaled_model)
    aip[i] = dot<T>(tmp_vec,dist_q); // Ai := Si*Pi.q   
    tmp_vec = grad_hist[i];          // tmp_vec := Yi (copy op=)
    scal<T>(tmp_vec,-aip[i]);        // Yi*(-Ai) (scaled_grad)
    geadd<T>(tmp_vec,dist_q);        // q := q - Yi*Ai
  }
  
  // potentially buggy: as 'i' will always be >=0, if it is 'size_t'
  //size_t i = n_elem - 1; // k-1
  int i = n_elem - 1; // k-1
  checkAssumption(i>=0);
  auto one_by_yk_sqr = 1/dot<T>(grad_hist[i],grad_hist[i]); // (1/Yk-1.Yk-1)
  tmp_vec = model_hist[i]; // tmp_vec := Sk-1 (copy op=)
  scal<T>(tmp_vec,one_by_yk_sqr); // Sk-1*(1/Yk-1.Yk-1)
  auto tmp = dot<T>(tmp_vec,grad_hist[i]); // Sk-1*(1/Yk-1.Yk-1).Yk-1
  scal<T>(dist_q,tmp); //q := (Sk-1.Yk-1/Yk-1.Yk-1)*q 

  //for(size_t i=0; i<n_elem; i++) {
  for(int i=0; i<n_elem; i++) {
    tmp_vec = grad_hist[i];             // tmp_vec := Yi (copy op=)
    scal<T>(tmp_vec,one_by_dot[i]);     // Yi*Pi (scaled_grad)
    auto beta = dot<T>(tmp_vec,dist_q); // Beta := Yi*Pi.q 
    tmp_vec = model_hist[i];            // tmp_vec := Si (copy op=)
    scal<T>(tmp_vec,aip[i]-beta);       // Si*(Ai-Beta) (scaled_model)
    geadd<T>(tmp_vec,dist_q);           // q := q + Si*(Ai-Beta)
  }

  return dist_q.to_vector();
}

template <class T>
template <class MODEL, class REGULARIZER>
void lbfgs<T>::update_model(MODEL& model,
                            const std::vector<T>& hkgk,
                            REGULARIZER& rType) {
  T* weightp = &model.weight[0];
  const T* hkgkp = &hkgk[0];
  size_t n = model.weight.size();
  for(size_t i=0; i<n; i++) {
    weightp[i] -= alpha * hkgkp[i];
  }

  // Updating bias (n+1 th) term of the model
  // n+1 th dimension of hkgk vector is for bias term 
  if(isIntercept) model.intercept -= alpha * hkgkp[n]; 

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
  if(iterCount != 1) { // if not the first iteration
    auto model_diff = cur_model_vec - old_model;
    auto grad_diff  = cur_gradient - old_gradient;
    auto dist_model_diff = vec_to_bcm<T>(std::move(model_diff));   // Si
    auto dist_grad_diff  = vec_to_bcm<T>(std::move(grad_diff));    // Yi
    T val = 1 / dot<T>(dist_model_diff,dist_grad_diff); // Pi := 1/Si.Yi
    one_by_dot.push_back(val);
    model_hist.push_back(std::move(dist_model_diff));
    grad_hist.push_back(std::move(dist_grad_diff));
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
  update_model(model,hkgk,rType); // in-place model update using computed hkgk
  t.show("update model: ");
  update_history(model,grad_vector,iterCount); // 'model' is updated model
  t.show("update history: ");
#ifdef _LBFGS_DEBUG_
  std::cout << "iteration: " << iterCount << std::endl;
  std::cout << model_hist.totActiveElements << " "
            << grad_hist.totActiveElements  << " " 
            << one_by_dot.totActiveElements << std::endl;
  debug_vector_print(old_model);
  debug_vector_print(old_gradient);
#endif
}

}

#endif
