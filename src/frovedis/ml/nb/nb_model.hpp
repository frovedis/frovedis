#ifndef _NB_MODEL_HPP_
#define _NB_MODEL_HPP_

#include "frovedis/matrix/rowmajor_matrix.hpp"
#include "frovedis/matrix/blas_wrapper.hpp"

namespace frovedis {

template <class T>
struct naive_bayes_model {
  rowmajor_matrix_local<T> theta;
  std::vector<T> pi;
  std::vector<T> label;
  std::vector<T> cls_counts;
  std::string model_type;
  std::vector<T> feature_count;
  // optional for bernoulli case
  rowmajor_matrix_local<T> theta_minus_negtheta;
  double binarize;
  std::vector<T> negtheta_sum;

  naive_bayes_model() {}

  naive_bayes_model(const rowmajor_matrix_local<T>& th,
                    const std::vector<T>& p, 
                    const std::vector<T>& l,
                    const std::vector<T>& cc,
                    const std::vector<T>& fc, 
                    const std::string& mtype,
                    double b = 0.0) {
    pi = p;
    label = l;
    cls_counts = cc;
    theta = th;
    feature_count = fc;
    model_type = mtype;
    binarize = b;
    if (mtype == "bernoulli") compute_param();
  }

  // for performance
  naive_bayes_model(rowmajor_matrix_local<T>&& th,
                    std::vector<T>&& p, 
                    std::vector<T>&& l,
                    std::vector<T>&& cc,
                    std::vector<T>&& fc, 
                    const std::string& mtype,
                    double b = 0.0) {
    pi.swap(p);
    label.swap(l);
    cls_counts.swap(cc);
    theta = th; 
    feature_count.swap(fc);
    model_type = mtype;
    binarize = b;
    if (mtype == "bernoulli") compute_param();
  }

  template <class MATRIX>
  rowmajor_matrix_local<T> 
  joint_log_likelihood(MATRIX& testData) {
    if (model_type == "bernoulli") 
      return bernoulli_joint_log_likelihood(testData);
    else return multinomial_joint_log_likelihood(testData);
  }

  std::vector<T>
  joint_log_likelihood(std::vector<T>& testData) {
    if (model_type == "bernoulli") 
      return bernoulli_joint_log_likelihood(testData);
    else return multinomial_joint_log_likelihood(testData);
  }

  T predict(std::vector<T>& testData) {
    auto llh = joint_log_likelihood(testData);
    return label[vector_argmax(llh)];
  }

  template <class MATRIX>
  std::vector<T> predict(MATRIX& testData) {
    auto prob_mat = joint_log_likelihood(testData);
    auto max = matrix_argmax(prob_mat, 1);
    return vector_take<T>(label, max);
  }

  template <class MATRIX>
  std::vector<T> predict_probability(MATRIX& testData) {
    auto prob_mat = joint_log_likelihood(testData);
    return matrix_amax(prob_mat, 1);
  }

  template <class MATRIX>
  rowmajor_matrix_local<T>
  predict_log_proba(MATRIX& testData) {
    auto prob_matrix = joint_log_likelihood(testData);
    auto num_row = prob_matrix.local_num_row;
    auto num_col = prob_matrix.local_num_col;
    auto pmp = prob_matrix.val.data();
    auto lse = logsumexp(prob_matrix, 1);
    auto lsep = lse.data();
    #pragma _NEC nointerchange
    for(size_t j = 0; j < num_col; ++j) {
      for(size_t i = 0; i < num_row; ++i) pmp[i * num_col + j] -= lsep[i];
    }
    return prob_matrix;
  }

  //predict_proba in sklearn
  template <class MATRIX>
  rowmajor_matrix_local<T>
  compute_probability_matrix(MATRIX& testData) {
    auto log_prob = predict_log_proba(testData);
    vector_exp_inplace(log_prob.val);
    return log_prob;
  }
  
  void debug_print(size_t limit = 5) {
    std::cout << "\nmodel_type: " << model_type << std::endl;
    std::cout << "binarize: " << binarize << std::endl;
    std::cout << "feature_count: "; debug_print_vector(feature_count, limit);
    std::cout << "theta: "; theta.debug_print(limit);
    std::cout << "pi: "; debug_print_vector(pi, limit);
    std::cout << "label: "; debug_print_vector(label, limit) ;
    std::cout << "class count: "; debug_print_vector(cls_counts, limit);
    if (model_type == "bernoulli") {
      std::cout << "theta_minus_negtheta: "; theta_minus_negtheta.debug_print(limit);
      std::cout << "negtheta_sum: "; debug_print_vector(negtheta_sum, limit);
    }
  }
  void clear() {
    theta.clear(); feature_count.clear(); pi.clear();
    label.clear(); cls_counts.clear();
    if (model_type == "bernoulli") {
      theta_minus_negtheta.clear(); negtheta_sum.clear();
    }
    model_type = ""; binarize = 0.0;
  }
  void save (const std::string& dir) {
    require(!directory_exists(dir), 
    "save: a directory with same name already exists!\n");
    make_directory(dir);
    theta.save(dir + "/theta"); //theta: rowmajor_matrix_local<T>
    make_dvector_scatter(feature_count).saveline(dir + "/feature_count");
    make_dvector_scatter(pi).saveline(dir + "/pi");
    make_dvector_scatter(label).saveline(dir + "/label");
    make_dvector_scatter(cls_counts).saveline(dir + "/cls_count");
    std::string type_file = dir + "/type";
    std::ofstream type_str;
    type_str.exceptions(std::ofstream::failbit | std::ofstream::badbit);
    type_str.open(type_file.c_str()); 
    type_str << model_type << "\n" << binarize << std::endl;
  }
  void savebinary (const std::string& dir) {
    require(!directory_exists(dir), 
    "save: a directory with same name already exists!\n");
    make_directory(dir);
    theta.savebinary(dir + "/theta"); //theta: rowmajor_matrix_local<T>
    make_dvector_scatter(feature_count).savebinary(dir + "/feature_count");
    make_dvector_scatter(pi).savebinary(dir + "/pi");
    make_dvector_scatter(label).savebinary(dir + "/label");
    make_dvector_scatter(cls_counts).savebinary(dir + "/cls_count");
    std::string type_file = dir + "/type";
    std::ofstream type_str;
    type_str.exceptions(std::ofstream::failbit | std::ofstream::badbit);
    type_str.open(type_file.c_str()); 
    type_str << model_type << "\n" << binarize << std::endl;
  }
  void load (const std::string& dir) {
    theta = make_rowmajor_matrix_local_load<T>(dir + "/theta");
    feature_count = make_dvector_loadline<T>(dir + "/feature_count").gather();
    pi = make_dvector_loadline<T>(dir + "/pi").gather();
    label = make_dvector_loadline<T>(dir + "/label").gather();
    cls_counts = make_dvector_loadline<T>(dir + "/cls_count").gather();
    std::string type_file = dir + "/type";
    std::ifstream type_str(type_file.c_str()); type_str >> model_type >> binarize;
    if (model_type == "bernoulli") compute_param(); 
  }
  void loadbinary (const std::string& dir) {
    theta = make_rowmajor_matrix_local_loadbinary<T>(dir + "/theta");
    feature_count = make_dvector_loadbinary<T>(dir + "/feature_count").gather();
    pi = make_dvector_loadbinary<T>(dir + "/pi").gather();
    label = make_dvector_loadbinary<T>(dir + "/label").gather();
    cls_counts = make_dvector_loadbinary<T>(dir + "/cls_count").gather();
    std::string type_file = dir + "/type";
    std::ifstream type_str(type_file.c_str()); type_str >> model_type >> binarize;
    if (model_type == "bernoulli") compute_param(); 
  }

  node_local<naive_bayes_model<T>> broadcast();  // for performance

  private:
  void compute_param() {
    auto nrow = theta.local_num_row;
    auto ncol = theta.local_num_col;
    theta_minus_negtheta = theta;
    T *theta_minus_negth = &theta_minus_negtheta.val[0];
    negtheta_sum.clear();
    negtheta_sum.resize(nrow,0); // allocating memory for negtheta_sum
    auto negtheta_sump = negtheta_sum.data();
    // computing theta_minus_negtheta and negtheta_sum
    for (int i = 0; i < nrow; i++) {
      for(int j = 0; j < ncol; j++) {
        auto th = theta_minus_negth[i * ncol + j];
        auto negtheta = log(1 - exp(th));
        negtheta_sump[i] += negtheta;
        theta_minus_negth[i * ncol + j] = th - negtheta;        
      }
    }
  }

  std::vector<T>
  bernoulli_joint_log_likelihood(std::vector<T>& testData) {
    auto b_testData = vector_binarize(testData, static_cast<T>(binarize));
    auto r = theta_minus_negtheta * b_testData;
    T al = 1.0;
    auto temp = pi + negtheta_sum;
    axpy<T>(temp, r, al);
    return r;
  }

  template <class MATRIX>
  rowmajor_matrix_local<T> 
  bernoulli_joint_log_likelihood(MATRIX& testData) {
    auto b_testData = matrix_binarize(testData, static_cast<T>(binarize));
    auto r = b_testData * theta_minus_negtheta.transpose();
    auto nrow = r.local_num_row;
    auto ncol = r.local_num_col;
    // computation of axpy for each rows manually (for performance)
    auto rvalp = r.val.data();
    auto pip = pi.data();
    auto negtheta_sump = negtheta_sum.data();
    #pragma _NEC nointerchange
    for(size_t j = 0; j < ncol; ++j) {
      for(size_t i = 0; i < nrow; ++i) {
        rvalp[i * ncol + j] += pip[j] + negtheta_sump[j];
      }
    }
    return r;
  }

  std::vector<T>
   multinomial_joint_log_likelihood(std::vector<T>& testData) {
    auto r = theta * testData;
    T al = 1.0;
    axpy<T>(pi, r, al);
    return r;
  }

  template <class MATRIX>
  rowmajor_matrix_local<T> 
  multinomial_joint_log_likelihood(MATRIX& testData) {
    auto r = testData * theta.transpose();
    auto nrow = r.local_num_row;
    auto ncol = r.local_num_col;
    auto rvalp = r.val.data();
    auto pip = pi.data();
    // computation of axpy for each rows manually (for performance)
    #pragma _NEC nointerchange
    for(size_t j = 0; j < ncol; ++j) {
      for(size_t i = 0; i < nrow; ++i) rvalp[i * ncol + j] += pip[j];
    }
    return r;
  }
};

template <class T>
struct nb_model_bcast_helper {
  nb_model_bcast_helper() {}
  nb_model_bcast_helper(std::string& mt, double bin): 
    model_type(mt), binarize(bin) {}
  naive_bayes_model<T> operator()(rowmajor_matrix_local<T>& theta,
                         std::vector<T>& pi,
                         std::vector<T>& label,
                         std::vector<T>& cls_counts,
                         std::vector<T>& feature_count) {
    return naive_bayes_model<T>(std::move(theta),
                                std::move(pi),
                                std::move(label),
                                std::move(cls_counts),
                                std::move(feature_count),
                                model_type, binarize);
  }
  std::string model_type;
  double binarize;
  SERIALIZE(model_type, binarize)
};

template <class T>
node_local<naive_bayes_model<T>>
naive_bayes_model<T>::broadcast() {
  auto bth = theta.broadcast();
  auto bpi = frovedis::broadcast(pi);
  auto blb = frovedis::broadcast(label);
  auto bcc = frovedis::broadcast(cls_counts);
  auto bfc = frovedis::broadcast(feature_count);
  return bth.map(nb_model_bcast_helper<T>(model_type, binarize),
                 bpi, blb, bcc, bfc);
}

}
#endif 

