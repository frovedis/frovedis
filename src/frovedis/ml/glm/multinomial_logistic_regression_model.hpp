#ifndef _MULT_LOG_MODEL_HPP_
#define _MULT_LOG_MODEL_HPP_

#include <cstdlib>
#include "softmax_gradient_descent.hpp"
#include "../utility/mattype.hpp"

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
    threshold = m.threshold;
  }

  multinomial_logistic_regression_model(
    multinomial_logistic_regression_model<T>&& m) {
    weight = m.weight; 
    intercept.swap(m.intercept);
    nfeatures = m.nfeatures;
    isIntercept = m.isIntercept;
    nclasses = m.nclasses;
    threshold = m.threshold;
  }

  multinomial_logistic_regression_model<T>&
  operator= (const multinomial_logistic_regression_model<T>& m) {
    weight = m.weight;
    intercept = m.intercept;
    nfeatures = m.nfeatures;
    isIntercept = m.isIntercept;
    nclasses = m.nclasses;
    threshold = m.threshold;
    return *this;
  }

  multinomial_logistic_regression_model<T>&
  operator= (multinomial_logistic_regression_model<T>&& m) {
    weight = m.weight;
    intercept.swap(m.intercept);
    nfeatures = m.nfeatures;
    isIntercept = m.isIntercept;
    nclasses = m.nclasses;
    threshold = m.threshold;
    return *this;
  }

  multinomial_logistic_regression_model(size_t feature, 
                     size_t classes,
                     bool isIcpt = false,
                     double thr = 0.5):
    weight(feature, classes), intercept(classes, 0) {
    isIntercept = isIcpt;
    nclasses = classes;
    nfeatures = feature;
    threshold = thr; //To make it uniform with binary model
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
  rowmajor_matrix_local<T> 
  compute_probability_matrix (MATRIX& mat) {
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
    std::vector<T> ret_lbl(nsamples,0), ret_proba(nsamples, 0);
    auto ret_lblp = ret_lbl.data();
    auto ret_probap = ret_proba.data();
    auto smatp = softmax_mat.val.data();
    for (size_t i = 0; i < nsamples; ++i) ret_probap[i] = smatp[i * nclasses + 0];
    for(size_t j = 1; j < nclasses; ++j) { // j starts from 1, considering 0th class as target label
      for (size_t i = 0; i < nsamples; ++i) { // nsamples >> nclasses
        auto max_proba = ret_probap[i];
        auto cur_proba = smatp[i * nclasses + j];
        if(cur_proba > max_proba) {
          ret_lblp[i] = j;
          ret_probap[i] = cur_proba;
        }
      }
    }
    // for spark: if threshold is cleared, it will always return raw probability values
    if (get_threshold() == NONE) return ret_proba;
    else return ret_lbl;
  }

  template <class MATRIX>
  std::vector<T> predict_probability (MATRIX& mat, bool use_score = false) {
    auto thr = get_threshold();
    set_threshold(NONE);
    auto ret = predict(mat);
    set_threshold(thr);
    return ret;
  }

  size_t get_num_features() { return nfeatures; }
  size_t get_num_classes()  { return nclasses; }
  T get_threshold() { return threshold;} // to make it uniform
  void set_threshold(T thr) {threshold = thr;}     // to make it call by spark

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

  void save(const std::string& dir) {
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
    metadata_str << threshold << std::endl;
    //std::cout << "save request on multinomial logistic regression model with dirname: " << dir << std::endl;
  }

  void savebinary(const std::string& dir) {
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
    metadata_str << threshold << std::endl;
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
    metadata_str >> threshold;
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
    metadata_str >> threshold;
    //std::cout << "loadbinary request for multinomial logistic regression model with dirname: " << dir << std::endl;
  }

  node_local<multinomial_logistic_regression_model<T>> broadcast();  // for performance

  rowmajor_matrix_local<T> weight;
  std::vector<T> intercept;
  size_t nfeatures, nclasses;
  bool isIntercept;
  double threshold;
  SERIALIZE(weight,intercept,nfeatures,nclasses,isIntercept,threshold)
};

template <class T>
multinomial_logistic_regression_model<T>
operator- (const multinomial_logistic_regression_model<T>& m1,
           const multinomial_logistic_regression_model<T>& m2) {
  checkAssumption(m1.nfeatures == m2.nfeatures && 
                  m1.nclasses == m2.nclasses &&
                  m1.isIntercept == m2.isIntercept && 
                  m1.threshold == m2.threshold);
  multinomial_logistic_regression_model<T> ret(m1.nfeatures, m1.nclasses,
                                               m1.isIntercept, m1.threshold);
  ret.weight.val = m1.weight.val - m2.weight.val;
  ret.intercept = m1.intercept - m2.intercept;
  return ret;
}

template <class T>
struct model_bcast_helper {
  model_bcast_helper() {}
  model_bcast_helper(size_t nc, size_t nf, bool in, double thr): 
  nclasses(nc),nfeatures(nf),isIntercept(in),threshold(thr) {}
  multinomial_logistic_regression_model<T> 
  operator()(rowmajor_matrix_local<T>& weight,
             std::vector<T>& intercept) {
    multinomial_logistic_regression_model<T> ret(nfeatures, nclasses, isIntercept);
    ret.weight = std::move(weight);
    ret.intercept.swap(intercept);
    ret.threshold = threshold;
    return ret;
  }
  size_t nclasses,nfeatures;
  bool isIntercept;
  double threshold;
  SERIALIZE(nclasses,nfeatures,isIntercept,threshold)
};

template <class T>
node_local<multinomial_logistic_regression_model<T>>
multinomial_logistic_regression_model<T>::broadcast() {
  auto bweight = weight.broadcast();
  auto bintercept = frovedis::broadcast(intercept);
  return bweight.map(model_bcast_helper<T>(nclasses,nfeatures,
                     isIntercept,threshold),
                     bintercept);
}

}
#endif
