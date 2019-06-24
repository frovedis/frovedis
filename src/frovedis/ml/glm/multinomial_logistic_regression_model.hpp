#ifndef _MULT_LOG_MODEL_HPP_
#define _MULT_LOG_MODEL_HPP_

#include <cstdlib>
#include "softmax_gradient_descent.hpp"

namespace frovedis {

template <class T>
struct multinomial_logistic_regression_model {
  multinomial_logistic_regression_model() {}

  multinomial_logistic_regression_model(
    const multinomial_logistic_regression_model<T>& m) {
    weight = m.weight; 
    intercept = m.intercept;
    nfeatures = m.nfeatures;
    isIntercept = m.isIntercept;
    nclasses = m.nclasses;
  }

  multinomial_logistic_regression_model(
    multinomial_logistic_regression_model<T>&& m) {
    weight = m.weight; 
    intercept.swap(m.intercept);
    nfeatures = m.nfeatures;
    isIntercept = m.isIntercept;
    nclasses = m.nclasses;
  }

  multinomial_logistic_regression_model<T>&
  operator= (const multinomial_logistic_regression_model<T>& l) {
    weight = l.weight;
    intercept = l.intercept;
    nfeatures = l.nfeatures;
    isIntercept = l.isIntercept;
    nclasses = l.nclasses;
    return *this;
  }

  multinomial_logistic_regression_model<T>&
  operator= (multinomial_logistic_regression_model<T>&& l) {
    weight = l.weight;
    intercept.swap(l.intercept);
    nfeatures = l.nfeatures;
    isIntercept = l.isIntercept;
    nclasses = l.nclasses;
    return *this;
  }

  multinomial_logistic_regression_model(size_t feature, 
                     size_t classes,
                     bool isIcpt = false): weight(feature,classes), 
                     intercept(classes,0) {
    isIntercept = isIcpt;
    nclasses = classes;
    nfeatures = feature;
  }

  void debug_print() {
    std::cout << "weight: ";     weight.debug_print();
    std::cout << "intercept: "; 
    for(auto i: intercept) std::cout << i << " "; std::cout << std::endl;
    std::cout << "nclasses " << nclasses << std::endl;
    std::cout << "nfeatures " << nfeatures << std::endl;
    std::cout << "isIntercept " << isIntercept << std::endl;
    std::cout << std::endl;
  }

  template <class MATRIX>
  rowmajor_matrix_local<T> compute_probability_matrix (MATRIX& mat) {
    softmax_gradient_descent gd(isIntercept);
    auto wtx = gd.compute_wtx<T>(mat, weight, intercept);
    auto softmax_mat = gd.compute_softmax_probability<T>(wtx);
    //softmax_mat.save("softmax_prob");
    return softmax_mat;
  }

  template <class MATRIX>
  std::vector<T> predict (MATRIX& mat) {
    auto nsamples = mat.local_num_row;
    auto softmax_mat = compute_probability_matrix(mat);
    std::vector<T> ret(nsamples,0);
    auto retp = ret.data();
    auto smatp = softmax_mat.val.data();
    for(size_t j = 1; j < nclasses; ++j) { // j starts from 1, considering 0th class as target label
      for (size_t i = 0; i < nsamples; ++i) { // nsamples >> nclasses
        auto max_id = static_cast<int>(retp[i]);
        auto max_proba = smatp[i * nclasses + max_id];
        auto cur_proba = smatp[i * nclasses + j];
        if(cur_proba > max_proba) retp[i] = j;
      }
    }
    return ret;
  }

  template <class MATRIX>
  std::vector<T> predict_probability (MATRIX& mat) {
    auto nsamples = mat.local_num_row;
    auto softmax_mat = compute_probability_matrix(mat);
    std::vector<T> ret(nsamples,0);
    auto retp = ret.data();
    auto smatp = softmax_mat.val.data();
    for (size_t i = 0; i < nsamples; ++i) retp[i] = smatp[i * nclasses + 0];
    for(size_t j = 1; j < nclasses; ++j) { // j starts from 1, considering 0th class as target label
      for (size_t i = 0; i < nsamples; ++i) { // nsamples >> nclasses
        auto max_proba = retp[i];
        auto cur_proba = smatp[i * nclasses + j];
        if(cur_proba > max_proba) retp[i] = cur_proba; 
      }
    }
    return ret;
  }

  size_t get_num_features() { return nfeatures; }
  size_t get_num_classes()  { return nclasses; }
  T get_threshold() { return 0.5; } // to make it uniform
  void set_threshold(T thr) {/*skip*/}     // to make it call by spark

  void __create_dir_struct (const std::string& dir) {
    struct stat sb;
    if(stat(dir.c_str(), &sb) != 0) { // no file/directory
      mode_t mode = S_IRWXU | S_IRWXG | S_IRWXO; // man 2 stat
      if(mkdir(dir.c_str(), mode) != 0) {
        perror("mkdir failed:");
        throw std::runtime_error("mkdir failed");
      }
    } else if(!S_ISDIR(sb.st_mode)) {
      throw std::runtime_error(dir + " is not a directory");
    }
  }

  void save (const std::string& dir) {
    __create_dir_struct(dir);
    std::string weight_file = dir + "/weight";
    weight.save(weight_file); //weight: rowmajor_matrix_local<T>
    std::string intercept_file = dir + "/intercept";
    std::string metadata_file = dir + "/metadata";
    std::ofstream intercept_str,  metadata_str;
    intercept_str.exceptions(std::ofstream::failbit | std::ofstream::badbit);
    metadata_str.exceptions(std::ofstream::failbit | std::ofstream::badbit);
    intercept_str.open(intercept_file.c_str()); for(auto& e: intercept) intercept_str << e << std::endl;
    metadata_str.open(metadata_file.c_str()); 
    metadata_str << nclasses    << std::endl;
    metadata_str << nfeatures   << std::endl;
    metadata_str << isIntercept << std::endl;
    //std::cout << "save request on multinomial logistic regression model with dirname: " << dir << std::endl;
  }

  void savebinary (const std::string& dir) {
    __create_dir_struct(dir);
    std::string weight_dir = dir + "/weight";
    weight.savebinary(weight_dir); //weight: rowmajor_matrix_local<T>
    std::string intercept_file = dir + "/intercept";
    std::string metadata_file = dir + "/metadata";
    make_dvector_scatter(intercept).savebinary(intercept_file);
    std::ofstream metadata_str;
    metadata_str.exceptions(std::ofstream::failbit | std::ofstream::badbit);
    metadata_str.open(metadata_file.c_str()); 
    metadata_str << nclasses    << std::endl;
    metadata_str << nfeatures   << std::endl;
    metadata_str << isIntercept << std::endl;
    //std::cout << "save binary request on multinomial logistic regression model with dirname: " << dir << std::endl;
  }

  void load (const std::string& dir) {
    std::string weight_file = dir + "/weight";
    weight = make_rowmajor_matrix_local_load<T>(weight_file);
    std::string intercept_file = dir + "/intercept";
    std::string metadata_file = dir + "/metadata";
    std::ifstream intercept_str(intercept_file.c_str());
    intercept.clear(); for(T x; intercept_str >> x;) intercept.push_back(x);
    std::ifstream metadata_str(metadata_file.c_str()); 
    metadata_str >> nclasses;
    metadata_str >> nfeatures;
    metadata_str >> isIntercept;
    //std::cout << "load request for multinomial logistic regression model with dirname: " << dir << std::endl;
  }

  void loadbinary (const std::string& dir) {
    std::string weight_dir = dir + "/weight";
    weight = make_rowmajor_matrix_local_loadbinary<T>(weight_dir);
    std::string intercept_file = dir + "/intercept";
    std::string metadata_file = dir + "/metadata";
    intercept = make_dvector_loadbinary<T>(intercept_file).gather();
    std::ifstream metadata_str(metadata_file.c_str()); 
    metadata_str >> nclasses;
    metadata_str >> nfeatures;
    metadata_str >> isIntercept;
    //std::cout << "loadbinary request for multinomial logistic regression model with dirname: " << dir << std::endl;
  }

  node_local<multinomial_logistic_regression_model<T>> broadcast();  // for performance

  rowmajor_matrix_local<T> weight;
  std::vector<T> intercept;
  size_t nfeatures, nclasses;
  bool isIntercept;
  SERIALIZE(weight,intercept,nfeatures,nclasses,isIntercept)
};

template <class T>
multinomial_logistic_regression_model<T>
operator- (const multinomial_logistic_regression_model<T>& m1,
           const multinomial_logistic_regression_model<T>& m2) {
  checkAssumption(m1.nfeatures == m2.nfeatures && m1.nclasses == m2.nclasses);
  multinomial_logistic_regression_model<T> ret(m1.nfeatures,m1.nclasses);
  const T* m1wp = &m1.weight.val[0];
  const T* m2wp = &m2.weight.val[0];
  const T* m1ip = &m1.intercept[0];
  const T* m2ip = &m2.intercept[0];
  T* rwp = &ret.weight.val[0];
  T* rip = &ret.intercept[0];
  for(size_t i=0; i<ret.weight.val.size(); ++i) rwp[i] = m1wp[i] - m2wp[i];
  for(size_t i=0; i<ret.nclasses; ++i) rip[i] = m1ip[i] - m2ip[i];
  return ret;
}

template <class T>
struct model_bcast_helper {
  model_bcast_helper() {}
  model_bcast_helper(size_t nc, size_t nf, bool in): 
  nclasses(nc),nfeatures(nf),isIntercept(in) {}
  multinomial_logistic_regression_model<T> 
  operator()(rowmajor_matrix_local<T>& weight,
             std::vector<T>& intercept) {
    multinomial_logistic_regression_model<T> ret(nfeatures, nclasses, isIntercept);
    ret.weight = std::move(weight);
    ret.intercept.swap(intercept);
    return ret;
  }
  size_t nclasses,nfeatures;
  bool isIntercept;
  SERIALIZE(nclasses,nfeatures,isIntercept)
};

template <class T>
node_local<multinomial_logistic_regression_model<T>>
multinomial_logistic_regression_model<T>::broadcast() {
  auto bweight = weight.broadcast();
  auto bintercept = frovedis::broadcast(intercept);
  return bweight.map(model_bcast_helper<T>(nclasses,nfeatures,isIntercept),
                     bintercept);
}

}
#endif
