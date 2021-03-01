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
                    const std::vector<T>& p, const std::vector<T>& l,
                    const std::vector<T>& cc,
                    const std::vector<T>& fc, const std::string& mtype,
                    double b = 0.0) {
    pi = p;
    label = l;
    cls_counts = cc;
    theta = th;
    feature_count = fc;
    model_type = mtype;
    if (mtype == "bernoulli") compute_param();
    binarize = b;
  }

  // for performance
  naive_bayes_model(rowmajor_matrix_local<T>&& th,
                    std::vector<T>&& p, std::vector<T>&& l,
                    std::vector<T>&& cc,
                    std::vector<T>&& fc, const std::string& mtype,
                    double b = 0.0) {
    pi.swap(p);
    label.swap(l);
    cls_counts.swap(cc);
    theta = th; 
    feature_count.swap(fc);
    model_type = mtype;
    if (mtype == "bernoulli") compute_param();
    binarize = b;
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
    std::cout << "theta: "; theta.debug_print(limit);
    std::cout << "pi: "; debug_print_vector(pi, limit);
    std::cout << "label: "; debug_print_vector(label, limit) ;
    std::cout << "class count: "; debug_print_vector(cls_counts, limit);
    std::cout << "model_type: " << model_type << std::endl;
    std::cout << "feature_count: "; debug_print_vector(feature_count, limit);
    if (model_type == "bernoulli") {
      std::cout << "theta_minus_negtheta: "; theta_minus_negtheta.debug_print(limit);
      std::cout << "negtheta_sum: "; debug_print_vector(negtheta_sum, limit);
      std::cout << "binarize: " << binarize;
    }
  }
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
    std::string theta_file = dir + "/theta";
    theta.save(theta_file); //theta: rowmajor_matrix_local<T>
    std::string pi_file = dir + "/pi";
    std::string label_file = dir + "/label";
    std::string count_file = dir + "/cls_count";
    std::string type_file = dir + "/type";
    std::string fc_file = dir + "/feature_count";
    std::string binarize_file = dir + "/binarize";
    std::ofstream pi_str, label_str, count_str, type_str, fc_str, binarize_str;
    pi_str.exceptions(std::ofstream::failbit | std::ofstream::badbit);
    label_str.exceptions(std::ofstream::failbit | std::ofstream::badbit);
    type_str.exceptions(std::ofstream::failbit | std::ofstream::badbit);
    pi_str.open(pi_file.c_str()); for(auto& e: pi) pi_str << e << std::endl;
    label_str.open(label_file.c_str()); for(auto& e: label) label_str << e << std::endl;
    count_str.open(count_file.c_str()); for(auto& e: cls_counts) count_str << e << std::endl;
    type_str.open(type_file.c_str()); type_str << model_type << std::endl;
    fc_str.open(fc_file.c_str()); for(auto& e: feature_count) fc_str << e << std::endl;
    if (model_type == "bernoulli")
      binarize_str.open(binarize_file.c_str()); binarize_str << binarize << std::endl;
  }
  void savebinary (const std::string& dir) {
    __create_dir_struct(dir);
    std::string theta_dir = dir + "/theta";
    theta.savebinary(theta_dir); //theta: rowmajor_matrix_local<T>
    std::string pi_file = dir + "/pi";
    std::string label_file = dir + "/label";
    std::string count_file = dir + "/cls_count";
    std::string type_file = dir + "/type";
    std::string fc_file = dir + "/feature_count";
    std::string binarize_file = dir + "/binarize";
    make_dvector_scatter(pi).savebinary(pi_file);
    make_dvector_scatter(label).savebinary(label_file);
    make_dvector_scatter(cls_counts).savebinary(count_file);
    make_dvector_scatter(feature_count).savebinary(fc_file);
    std::ofstream type_str, binarize_str;
    type_str.exceptions(std::ofstream::failbit | std::ofstream::badbit);
    type_str.open(type_file.c_str()); type_str << model_type << std::endl;
    if (model_type == "bernoulli")
      binarize_str.open(binarize_file.c_str()); binarize_str << binarize << std::endl;
  }
  void load (const std::string& dir) {
    std::string theta_file = dir + "/theta";
    theta = make_rowmajor_matrix_local_load<T>(theta_file);
    std::string pi_file = dir + "/pi";
    std::string label_file = dir + "/label";
    std::string count_file = dir + "/cls_count";
    std::string type_file = dir + "/type";
    std::ifstream pi_str(pi_file.c_str());
    std::string fc_file = dir + "/feature_count";
    std::string binarize_file = dir + "/binarize";
    pi.clear(); for(T x; pi_str >> x;) pi.push_back(x);
    std::ifstream label_str(label_file.c_str());
    label.clear(); for(T x; label_str >> x;) label.push_back(x);
    std::ifstream count_str(count_file.c_str());
    cls_counts.clear(); for(size_t x; count_str >> x;) cls_counts.push_back(x);
    std::ifstream fc_str(fc_file.c_str());
    feature_count.clear(); for(T x; fc_str >> x;) feature_count.push_back(x);
    std::ifstream type_str(type_file.c_str()); type_str >> model_type;
    if (model_type == "bernoulli") {
      std::ifstream binarize_str(binarize_file.c_str()); binarize_str >> binarize;
      compute_param();
    }
  }
  void loadbinary (const std::string& dir) {
    std::string theta_dir = dir + "/theta";
    theta = make_rowmajor_matrix_local_loadbinary<T>(theta_dir);
    std::string pi_file = dir + "/pi";
    std::string label_file = dir + "/label";
    std::string count_file = dir + "/cls_count";
    std::string type_file = dir + "/type";
    std::string fc_file = dir + "/feature_count";
    std::string binarize_file = dir + "/binarize";
    pi = make_dvector_loadbinary<T>(pi_file).gather();
    label = make_dvector_loadbinary<T>(label_file).gather();
    cls_counts = make_dvector_loadbinary<T>(count_file).gather();
    feature_count = make_dvector_loadbinary<T>(fc_file).gather();
    std::ifstream type_str(type_file.c_str()); type_str >> model_type;
    if (model_type == "bernoulli") {
      std::ifstream binarize_str(binarize_file.c_str()); binarize_str >> binarize;
      compute_param();
    }
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
  nb_model_bcast_helper(std::string& mt): model_type(mt) {}
  naive_bayes_model<T> operator()(rowmajor_matrix_local<T>& theta,
                         std::vector<T>& pi,
                         std::vector<T>& label,
                         std::vector<T>& cls_counts,
                         std::vector<T>& feature_count,
                         double binarize) {
    return naive_bayes_model<T>(std::move(theta),
                                std::move(pi),
                                std::move(label),
                                std::move(cls_counts),
                                std::move(feature_count),
                                model_type,
                                binarize);
  }
  std::string model_type;
  SERIALIZE(model_type)
};

template <class T>
node_local<naive_bayes_model<T>>
naive_bayes_model<T>::broadcast() {
  auto bth = theta.broadcast();
  auto bpi = frovedis::broadcast(pi);
  auto blb = frovedis::broadcast(label);
  auto bcc = frovedis::broadcast(cls_counts);
  auto bfc = frovedis::broadcast(feature_count);
  auto bbinarize = frovedis::broadcast(binarize);
  return bth.map(nb_model_bcast_helper<T>(model_type),bpi,blb,bcc,bfc,bbinarize);
}

}
#endif 

