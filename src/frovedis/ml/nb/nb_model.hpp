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
  std::vector<size_t> cls_counts;
  std::string model_type;
  // optional for bernoulli case
  rowmajor_matrix_local<T> theta_minus_negtheta;
  std::vector<T> negtheta_sum;

  naive_bayes_model() {}

  naive_bayes_model(const rowmajor_matrix_local<T>& th,
                    const std::vector<T>& p, const std::vector<T>& l,
                    const std::vector<size_t>& cc, const std::string& mtype) {
    pi = p;
    label = l;
    cls_counts = cc;
    theta = th;
    model_type = mtype;
    if (mtype == "bernoulli") compute_param();
  }

  // for performance
  naive_bayes_model(rowmajor_matrix_local<T>&& th,
                    std::vector<T>&& p, std::vector<T>&& l,
                    std::vector<size_t>&& cc, const std::string& mtype) {
    pi.swap(p);
    label.swap(l);
    cls_counts.swap(cc);
    theta = th; 
    model_type = mtype;
    if (mtype == "bernoulli") compute_param();
  }

  T predict(std::vector<T>& testData) {
    if (model_type == "bernoulli") return bernoulli_predict(testData);
    else return multinomial_predict(testData);
  }

  template <class MATRIX>
  std::vector<T> predict(MATRIX& testData) {
    if (model_type == "bernoulli") return bernoulli_predict(testData);
    else return multinomial_predict(testData);
  }

  void debug_print() {
    std::cout << "theta: ";  theta.debug_print();
    std::cout << "pi: ";     
    for(auto i: pi) std::cout << i << " "; std::cout << std::endl;
    std::cout << "label: "; 
    for(auto i: label) std::cout << i << " "; std::cout << std::endl;
    std::cout << "class count: ";
    for(auto i: cls_counts) std::cout << i << " "; std::cout << std::endl;
    std::cout << "model_type: " << model_type << std::endl;
    if (model_type == "bernoulli") {
      std::cout << "theta_minus_negtheta: ";  theta_minus_negtheta.debug_print();
      std::cout << "negtheta_sum: ";  
      for(auto i: negtheta_sum) std::cout << i << " "; std::cout << std::endl;
    }
    std::cout << std::endl;
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
    std::ofstream pi_str, label_str, count_str, type_str;
    pi_str.exceptions(std::ofstream::failbit | std::ofstream::badbit);
    label_str.exceptions(std::ofstream::failbit | std::ofstream::badbit);
    type_str.exceptions(std::ofstream::failbit | std::ofstream::badbit);
    pi_str.open(pi_file.c_str()); for(auto& e: pi) pi_str << e << std::endl;
    label_str.open(label_file.c_str()); for(auto& e: label) label_str << e << std::endl;
    count_str.open(count_file.c_str()); for(auto& e: cls_counts) count_str << e << std::endl;
    type_str.open(type_file.c_str()); type_str << model_type << std::endl;
  }
  void savebinary (const std::string& dir) {
    __create_dir_struct(dir);
    std::string theta_dir = dir + "/theta";
    theta.savebinary(theta_dir); //theta: rowmajor_matrix_local<T>
    std::string pi_file = dir + "/pi";
    std::string label_file = dir + "/label";
    std::string count_file = dir + "/cls_count";
    std::string type_file = dir + "/type";
    make_dvector_scatter(pi).savebinary(pi_file);
    make_dvector_scatter(label).savebinary(label_file);
    make_dvector_scatter(cls_counts).savebinary(count_file);
    std::ofstream type_str;
    type_str.exceptions(std::ofstream::failbit | std::ofstream::badbit);
    type_str.open(type_file.c_str()); type_str << model_type << std::endl;
  }
  void load (const std::string& dir) {
    std::string theta_file = dir + "/theta";
    theta = make_rowmajor_matrix_local_load<T>(theta_file);
    std::string pi_file = dir + "/pi";
    std::string label_file = dir + "/label";
    std::string count_file = dir + "/cls_count";
    std::string type_file = dir + "/type";
    std::ifstream pi_str(pi_file.c_str());
    pi.clear(); for(T x; pi_str >> x;) pi.push_back(x);
    std::ifstream label_str(label_file.c_str());
    label.clear(); for(T x; label_str >> x;) label.push_back(x);
    std::ifstream count_str(count_file.c_str());
    cls_counts.clear(); for(size_t x; count_str >> x;) cls_counts.push_back(x);
    std::ifstream type_str(type_file.c_str()); type_str >> model_type;
    if (model_type == "bernoulli") compute_param();
  }
  void loadbinary (const std::string& dir) {
    std::string theta_dir = dir + "/theta";
    theta = make_rowmajor_matrix_local_loadbinary<T>(theta_dir);
    std::string pi_file = dir + "/pi";
    std::string label_file = dir + "/label";
    std::string count_file = dir + "/cls_count";
    std::string type_file = dir + "/type";
    pi = make_dvector_loadbinary<T>(pi_file).gather();
    label = make_dvector_loadbinary<T>(label_file).gather();
    cls_counts = make_dvector_loadbinary<size_t>(count_file).gather();
    std::ifstream type_str(type_file.c_str()); type_str >> model_type;
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
    negtheta_sum.resize(ncol,0); // allocating memory for negtheta_sum
    auto negtheta_sump = negtheta_sum.data();
    // computing theta_minus_negtheta and negtheta_sum
    for (int i = 0; i < nrow; i++) {
      for(int j = 0; j < ncol; j++) {
        auto th = theta_minus_negth[i * ncol + j];
        auto negtheta = log10(1 - exp(th));
        //std::cout << negtheta << " ";
        negtheta_sump[j] += negtheta;
        theta_minus_negth[i * ncol + j] = th - negtheta;        
      }
      //std::cout << std::endl;
    }
  }

  int get_max_index(std::vector<T>& vec) {
    int max_id = 0;
    auto vecp = vec.data();
    for(int i=1; i<vec.size(); i++) {
      if(vecp[i] > vecp[max_id])  max_id = i;
    }
    return max_id;
  }

  T bernoulli_predict(std::vector<T>& testData) {
    auto r = theta_minus_negtheta.transpose() * testData;
    axpy<T>(pi,r,1.0);
    axpy<T>(negtheta_sum,r,1.0);
    auto max_index = get_max_index(r);
    return label[max_index];
  }

  template <class MATRIX>
  std::vector<T> bernoulli_predict(MATRIX& testData) {
    auto r = testData * theta_minus_negtheta;
    auto nrow = r.local_num_row;
    auto ncol = r.local_num_col;
    // computation of axpy for each rows manually (for performace)
    auto rvalp = r.val.data();
    auto pip = pi.data();
    auto negtheta_sump = negtheta_sum.data();
    for(int i=0; i<nrow; i++) {
      for(int j=0; j<ncol; j++) {
        rvalp[i*ncol+j] += pip[i] + negtheta_sump[i];
      }
    }
    // checking max probablity for each case and predicting corresponding labels
    std::vector<T> ret(nrow);
    auto retp = ret.data();
    auto labelp = label.data();
    for(int i=0; i<nrow; i++) {
      int max_id = 0;
      for(int j=1; j<ncol; j++) {
        if (rvalp[i*ncol+j] > rvalp[i*ncol+max_id]) max_id = j;
      }
      retp[i] = labelp[max_id];
    }
    return ret;
  }

  T multinomial_predict(std::vector<T>& testData) {
    auto r = theta.transpose() * testData;
    axpy<T>(pi,r,1.0);
    auto max_index = get_max_index(r);
    return label[max_index];
  }

  template <class MATRIX>
  std::vector<T> multinomial_predict(MATRIX& testData) {
    auto r = testData * theta;
    auto nrow = r.local_num_row;
    auto ncol = r.local_num_col;
    auto rvalp = r.val.data();
    auto pip = pi.data();
    // computation of axpy for each rows manually (for performace)
    for(int i=0; i<nrow; i++) {
      for(int j=0; j<ncol; j++) {
        rvalp[i*ncol+j] += pip[i];
      }
    }
    // checking max probablity for each case and predicting corresponding labels
    std::vector<T> ret(nrow);
    auto retp = ret.data();
    auto labelp = label.data();
    for(int i=0; i<nrow; i++) {
      int max_id = 0;
      for(int j=1; j<ncol; j++) {
        if (rvalp[i*ncol+j] > rvalp[i*ncol+max_id]) max_id = j;
      }
      retp[i] = labelp[max_id];
    }
    return ret;
  }
};

template <class T>
struct nb_model_bcast_helper {
  nb_model_bcast_helper() {}
  nb_model_bcast_helper(std::string& mt): model_type(mt) {}
  naive_bayes_model<T> operator()(rowmajor_matrix_local<T>& theta,
                         std::vector<T>& pi,
                         std::vector<T>& label,
                         std::vector<size_t>& cls_counts) {
    return naive_bayes_model<T>(std::move(theta),
                                std::move(pi),
                                std::move(label),
                                std::move(cls_counts),
                                model_type);
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
  return bth.map(nb_model_bcast_helper<T>(model_type),bpi,blb,bcc);
}

}
#endif 

