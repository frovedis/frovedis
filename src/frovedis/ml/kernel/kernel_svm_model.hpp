#ifndef _KERNEL_SVM_MODEL_HPP_
#define _KERNEL_SVM_MODEL_HPP_


#include "../../matrix/rowmajor_matrix.hpp"
#include "kernel_svm_solver.hpp"
#include "kernel_archive.hpp"
#include "kernel_config.hpp"


namespace frovedis { 
namespace kernel {

// C-SVC (classification) with non-linear kernel functio.
// Template K is type for values of input data, label, 
// and internal kernel matrix which usually takes large amount of memory.
template <typename K>
struct csvc_model {
  
  csvc_model() {}
  // Initializes model with parameters.
  // `cache_size` specifies the cache size (in unit of MB) of kernel matrix.
  csvc_model(const double tol, const double C, const int cache_size, 
             const int max_iter, const std::string& kernel_ty_str, 
             const double gamma = 0.1, const double coef0 = 0.0, const int degree = 3); 

  // Train model with given data and label.
  // It determines support vectors (`sv`), its coefficient (`sv_coef`), and bias term (`rho`).
  // An value of the labels have to be +1 or -1.
  void train(rowmajor_matrix_local<K>& data, std::vector<K>& label);

  // Classify data based on kernel values with support vectors.
  // Input data is splitted into batches with the size specified by `batche_size` (-1 for fixing auto).
  std::vector<double> predict(rowmajor_matrix_local<K>& data, int batch_size = -1);
  std::vector<double> predict_values(rowmajor_matrix_local<K>& data, int batch_size = -1);
  size_t get_working_set_size(size_t cache_size, size_t data_size, size_t feature_size);
  void save(const std::string& path);
  void load(const std::string& path);
  void savebinary(const std::string& path);
  void loadbinary(const std::string& path);
  
  double tol;
  double C;
  int cache_size;
  int max_iter;
  kernel_function_type kernel_ty;
  double gamma;
  double coef0;
  int degree;

  double rho;
  rowmajor_matrix_local<K> sv;
  std::vector<double> sv_coef;

  SERIALIZE(tol, C, cache_size, max_iter, kernel_ty, gamma, coef0, degree,
            rho, sv, sv_coef)
};

template <typename K>
csvc_model<K>::csvc_model(const double tol, const double C, const int cache_size, 
                          const int max_iter, const std::string& kernel_ty_str, 
                          const double gamma, const double coef0, const int degree): 
    tol(tol), C(C), cache_size(cache_size), max_iter(max_iter), 
    gamma(gamma), coef0(coef0), degree(degree) {

  if (kernel_ty_str == "rbf") {
    kernel_ty = kernel_function_type::RBF;
  } else if (kernel_ty_str == "poly") {
    kernel_ty = kernel_function_type::POLY;
  } else if (kernel_ty_str == "sigmoid") {
    kernel_ty = kernel_function_type::SIGMOID;
  } else {
    throw std::logic_error("unimplemented kernel type");
  }
  if (cache_size <= 0) {
    throw std::logic_error("kernel cache size should be positive");
  }

  sv.set_local_num(0,0);
}


template <typename K>
void csvc_model<K>::train(rowmajor_matrix_local<K>& data, std::vector<K>& label) {
  size_t data_size = data.local_num_row;
  size_t feature_size = data.local_num_col;
  if (data_size != label.size()) {
    throw std::runtime_error("Size of data and label are not matched");
  }
  LOG(DEBUG) << "train sample size = " << data_size << std::endl;
  LOG(DEBUG) << "feature size = " << feature_size << std::endl;

  bool invalid_label_value = false;  
  for (size_t i = 0; i < data_size; i++) {
    invalid_label_value |= (label[i] != 1.0 && label[i] != -1.0);
  }
  if (invalid_label_value) {
    throw std::runtime_error("Invalid labal values are feeded");
  }

  size_t ws_size = get_working_set_size(cache_size, data_size, feature_size);
  LOG(DEBUG) << "working set size = " << ws_size << std::endl;

  std::vector<int> y(data_size);
  std::vector<double> f_val(data_size);
  std::vector<double> alpha(data_size);
  for (size_t i = 0; i < label.size(); i++) {
    alpha[i] = 0.0;
    y[i] = label[i];
    f_val[i] = -1.0 * label[i];
  }
  rho = 0.0;

  csmo_solver<K> solver;
  solver.solve(data, y, alpha, rho, f_val, tol, C, C, ws_size, max_iter, kernel_ty, gamma, coef0, degree);
  LOG(DEBUG) << "rho = " << rho << std::endl;

  std::vector<size_t> sv_index(data_size);
  size_t cur = 0;
  for (size_t i = 0; i < data_size; i++) {
    int is_sv = (alpha[i] != 0);
    if (is_sv) {
      sv_index[cur] = i;
      cur++;
    }    
  }
  size_t sv_size = cur;
  LOG(DEBUG) << "number of support vectors = " << sv_size << std::endl;

  sv = rowmajor_matrix_local<K>(sv_size, feature_size);
  sv_coef = std::vector<double>(sv_size);
  for (size_t i = 0; i < sv_size; i++) {
    sv_coef[i] = alpha[sv_index[i]] * y[sv_index[i]];
  }
  for (size_t i = 0; i < sv_size; i++) {
    for (size_t j = 0; j < feature_size; j++) {
      sv.val[j + i * feature_size] = data.val[j + sv_index[i] * feature_size];
    }
  }
}


template <typename K>
std::vector<double> 
csvc_model<K>::predict_values(rowmajor_matrix_local<K>& data, int batch_size) {
  size_t feature_size = data.local_num_col;
  size_t data_size = data.local_num_row;
  size_t sv_size = sv.local_num_row;
  if (feature_size != sv.local_num_col) {
    throw std::runtime_error("Dimensions of support vector and data are not matched");
  }
  if (sv.local_num_row == 0) {
    throw std::runtime_error("There does not exist support vectors.");
  }
  LOG(DEBUG) << "predict sample size = " << data_size << std::endl;
  LOG(DEBUG) << "feature size = " << feature_size << std::endl;

  if (batch_size == -1) {
    // Determine batch size of support vector samples. 
    // This should depend on amount of available memory or user specified constraints.
    batch_size = 1000;  // temporarily
  }

  rowmajor_matrix_local<K> kernel_rows(batch_size, sv_size);
  kernel_matrix<K> kmatrix(sv, kernel_ty, gamma, coef0, degree);

  std::vector<double> dec_values(data_size);

  assert(batch_size > 0);
  size_t loop = ceil_div(data_size, size_t(batch_size));
	size_t count = 0;
	for (size_t i = 0; i < loop; i++) {
    int rem = data_size - i * batch_size;
    assert(rem >= 0);
		int step = std::min(batch_size, rem);
		if (step <= 0) continue;
		
    kmatrix.get_rows(data, count, step, 0, kernel_rows);
    
    for (size_t j = 0; j < step; j++) {
      for (size_t k = 0; k < sv_size; k++) {
        dec_values[j + count] += sv_coef[k] * kernel_rows.val[k + j * sv_size];
      }
    }
		count += step;
	}
  for (size_t j = 0; j < data_size; j++) {
    dec_values[j] -= rho;  
  }
  return dec_values;
}


template <typename K>
std::vector<double> 
csvc_model<K>::predict(rowmajor_matrix_local<K>& data, int batch_size) {
  std::vector<double> dec_values = predict_values(data, batch_size);
  for (size_t i = 0; i < dec_values.size(); i++) {
    double lb;
    if (dec_values[i] > 0) lb = 1.0;
    else lb = -1.0;
    dec_values[i] = lb;
  }
  return dec_values;
}


template <typename K>
size_t csvc_model<K>::get_working_set_size(size_t cache_size, size_t data_size, size_t feature_size) {
  size_t ws_size = cache_size * (1 << 20) / data_size / sizeof(K);
  if (ws_size > data_size) ws_size = data_size;
  if (ws_size % 2) ws_size -= 1;
  assert(ws_size >= 2);
  return ws_size;
}


template <typename K>
void csvc_model<K>::save(const std::string& path) {
  save_model<csvc_model>(*this, path, false);
}

template <typename K>
void csvc_model<K>::load(const std::string& path) {
  *this = load_model<csvc_model>(path, false);
}

template <typename K>
void csvc_model<K>::savebinary(const std::string& path) {
  save_model<csvc_model>(*this, path, true);
}

template <typename K>
void csvc_model<K>::loadbinary(const std::string& path) {
  *this = load_model<csvc_model>(path, true);
}


}  // namespace kernel
}  // namespace frovedis

#endif  // _KERNEL_SVM_MODEL_HPP_

