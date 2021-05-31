#ifndef _SIMPLE_LIN_REG_
#define _SIMPLE_LIN_REG_

#include <frovedis/matrix/blas_wrapper.hpp>
#include <frovedis/matrix/lapack_wrapper.hpp>
#include <frovedis/matrix/scalapack_wrapper.hpp>
#include <frovedis/ml/glm/linear_model.hpp>

namespace frovedis {

template<class T>
void rescale_data(colmajor_matrix<T>& mat, 
            std::vector<T>& vect, std::vector<T>& sample_weight) {
  auto sqrt_sw = vector_sqrt(sample_weight);
  auto vp = vect.data();
  auto swp = sqrt_sw.data();
  auto vect_size = vect.size();
  for(size_t i = 0; i < vect_size; ++i) vp[i] *= swp[i];
  mat = scale_cmm_matrix(mat, 0, sqrt_sw);
}

template<class T>
void rescale_data(colmajor_matrix_local<T>& mat, 
           std::vector<T>& vect, std::vector<T>& sample_weight) {
  auto sqrt_sw = vector_sqrt(sample_weight);
  auto vp = vect.data();
  auto swp = sqrt_sw.data();
  auto vect_size = vect.size();
  for(size_t i = 0; i < vect_size; ++i) vp[i] *= swp[i];
  mat = scale_cmm_matrix_impl(mat, 0, sqrt_sw);
}

// -------- for lapack solvers --------
template <class T>
void preprocess_data(colmajor_matrix<T>& mat,
                     dvector<T>& label,
                     colmajor_matrix_local<T>& Amat,
                     colmajor_matrix_local<T>& Bmat,
                     std::vector<T>& Amean,
                     T& Bmean,
                     std::vector<T>& sample_weight) {
  // re-alignment label-points
  auto sizes = mat.get_local_num_rows();
  label.align_as(sizes);

  auto nsamples = mat.num_row;
  auto nfeatures = mat.num_col;
  if (nsamples != label.size())
    REPORT_ERROR(USER_ERROR, "Number of samples in input matrix and label vector doesn't match!\n");

  // centerizing feature
  auto rmat = mat.to_rowmajor(); // to_rowmajor(): gather() is supported only on rowmajor_matrix<T> 
  Amean = compute_mean(rmat, 0, sample_weight); // column-wise mean
  centerize(rmat, Amean);
  Amat = colmajor_matrix_local<T>(rmat.gather());
  rmat.clear();

  // centerizing label
  auto label_vec = label.gather();
  auto label_vecp = label_vec.data();
  Bmean = 0.0;
  auto swp  = sample_weight.data();
  T total_sw = vector_sum(sample_weight);
  for(size_t i = 0; i < nsamples; ++i) Bmean += label_vecp[i] * swp[i];
  Bmean /= total_sw;
  if (nfeatures > nsamples) {
    Bmat = colmajor_matrix_local<T>(nfeatures, 1); // extending for solution matrix
    auto bmatvalp = Bmat.val.data();
    for(size_t i = 0; i < nsamples; ++i) bmatvalp[i] = label_vecp[i] - Bmean;
    rescale_data(Amat, Bmat.val, sample_weight);
  }
  else {
    for(size_t i = 0; i < nsamples; ++i) label_vecp[i] -= Bmean; // centerized in-place
    rescale_data(Amat, label_vec, sample_weight);
    Bmat.val.swap(label_vec); // reused label_vec (avoided memory allocation and copy)
    Bmat.local_num_row = nsamples;
    Bmat.local_num_col = 1;
  }
}

// -------- for scalapack solvers --------
template <class T>
void preprocess_data(colmajor_matrix<T>& mat,
                     dvector<T>& label,
                     blockcyclic_matrix<T>& Amat,
                     blockcyclic_matrix<T>& Bmat,
                     std::vector<T>& Amean,
                     T& Bmean,
                     std::vector<T> &sample_weight) {
  // re-alignment label-points
  auto sizes = mat.get_local_num_rows();
  label.align_as(sizes);

  auto nsamples = mat.num_row;
  auto nfeatures = mat.num_col;
  if (nsamples != label.size())
    REPORT_ERROR(USER_ERROR, "Number of samples in input matrix and label vector doesn't match!\n");

  // centerizing feature
  colmajor_matrix<T> copy_mat(mat);
  Amean = compute_mean(copy_mat, 0, sample_weight); // column-wise mean
  centerize(copy_mat, Amean);

  // centerizing label
  auto label_vec = label.gather();
  auto label_vecp = label_vec.data();
  Bmean = 0.0;
  auto swp  = sample_weight.data();
  T total_sw = vector_sum(sample_weight);
  for(size_t i = 0; i < nsamples; ++i) Bmean += label_vecp[i] * swp[i];
  Bmean /= total_sw;
  if (nfeatures > nsamples) {
    std::vector<T> tmp(nfeatures); // extending for solution matrix
    auto tmpp = tmp.data();
    for(size_t i = 0; i < nsamples; ++i) tmpp[i] = label_vecp[i] - Bmean;
    rescale_data(copy_mat, tmp, sample_weight);
    Bmat = blockcyclic_matrix<T>(tmp, 2);
  }
  else {         
    for(size_t i = 0; i < nsamples; ++i) label_vecp[i] -= Bmean; // centerized in-place
    rescale_data(copy_mat, label_vec, sample_weight);
    Bmat = blockcyclic_matrix<T>(label_vec, 2); // reused label_vec (avoided memory allocation and copy)
  }
  Amat = blockcyclic_matrix<T>(copy_mat);
  copy_mat.clear();
}



template <class T>
void set_intercept(linear_regression_model<T>& model,
                   std::vector<T>& Amean,
                   T Bmean) {
  model.intercept = Bmean - dot<T>(model.weight, Amean);
}

template <class T, class LOC_MAT>
linear_regression_model<T>
extract_solution(LOC_MAT& Bmat, size_t nfeatures) {
  linear_regression_model<T> model(nfeatures);
  auto weightp = model.weight.data();
  auto solxp = Bmat.val.data(); // NRHS is assumed as 1
  for (size_t i = 0; i < nfeatures; ++i) weightp[i] = solxp[i];
  return model;
}

// -------- User APIs -------
template <class T>
linear_regression_model<T>
linear_regression_with_lapack(colmajor_matrix<T>& mat,
                              dvector<T>& label,
                              int& rank,
                              std::vector<T>& sval,
                              bool fit_intercept = true) {

  std::vector<T> sample_weight;
  return linear_regression_with_lapack(mat, label, rank, sval, sample_weight, 
                                       fit_intercept);
}
template <class T>
linear_regression_model<T>
linear_regression_with_lapack(colmajor_matrix<T>& mat,
                              dvector<T>& label,
                              int& rank,
                              std::vector<T>& sval,
                              std::vector<T>& sample_weight,
                              bool fit_intercept = true) {
  T Bmean;
  std::vector<T> Amean;
  colmajor_matrix_local<T> Amat, Bmat;
  if(sample_weight.empty()) sample_weight = vector_full<T>(mat.num_row, 1);
  preprocess_data(mat, label, Amat, Bmat, Amean, Bmean, sample_weight);
  auto rcond = 0; // as per scipy.linalg.lstsq
  gelsd<T>(Amat, Bmat, sval, rank, rcond);
  auto model = extract_solution<T, colmajor_matrix_local<T>>(
                                Bmat, mat.num_col);
  if(fit_intercept) set_intercept(model, Amean, Bmean);
  return model;
}

template <class T>
linear_regression_model<T>
linear_regression_with_lapack(colmajor_matrix<T>& mat,
                              dvector<T>& label,
                              bool fit_intercept = true) {
  int rank;
  std::vector<T> sval;
  std::vector<T> sample_weight;
  return linear_regression_with_lapack(mat, label, rank, sval, sample_weight, 
                                       fit_intercept);
}

template <class T, class I, class O>
linear_regression_model<T>
linear_regression_with_lapack(crs_matrix<T,I,O>& mat,
                              dvector<T>& label,
                              int& rank,
                              std::vector<T>& sval,
                              std::vector<T>& sample_weight,
                              bool fit_intercept = true) {
  std::string msg = "lapack solver is supported only for dense data!\n";
  REPORT_ERROR(USER_ERROR, msg);
  return linear_regression_model<T>(); // never reachable: to supress compiler warning!
}

template <class T>
linear_regression_model<T>
linear_regression_with_scalapack(colmajor_matrix<T>& mat,
                                 dvector<T>& label,
                                 bool fit_intercept = true) {
  std::vector<T> sample_weight;
  return linear_regression_with_scalapack(mat, label, sample_weight, 
                                          fit_intercept);
}

template <class T>
linear_regression_model<T>
linear_regression_with_scalapack(colmajor_matrix<T>& mat,
                                 dvector<T>& label,
                                 std::vector<T>& sample_weight,
                                 bool fit_intercept = true) {
  T Bmean;
  std::vector<T> Amean;
  blockcyclic_matrix<T> Amat, Bmat;
  if(sample_weight.empty()) sample_weight = vector_full<T>(mat.num_row, 1);
  preprocess_data(mat, label, Amat, Bmat, Amean, Bmean, sample_weight);
  gels<T>(Amat, Bmat);
  auto r_Bmat = Bmat.to_rowmajor().gather();
  auto model = extract_solution<T, rowmajor_matrix_local<T>>(
                                r_Bmat, mat.num_col);
  if(fit_intercept) set_intercept(model, Amean, Bmean);
  return model;
}

template <class T, class I, class O>
linear_regression_model<T>
linear_regression_with_scalapack(crs_matrix<T,I,O>& mat,
                                 dvector<T>& label,
                                 std::vector<T>& sample_weight,
                                 bool fit_intercept = true) {
  std::string msg = "scalapack solver is supported only for dense data!\n";
  REPORT_ERROR(USER_ERROR, msg);
  return linear_regression_model<T>(); // never reachable: to supress compiler warning!
}

}
#endif
