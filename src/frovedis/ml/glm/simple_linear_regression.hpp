#ifndef _SIMPLE_LIN_REG_
#define _SIMPLE_LIN_REG_

#include <frovedis/matrix/blas_wrapper.hpp>
#include <frovedis/matrix/lapack_wrapper.hpp>
#include <frovedis/matrix/scalapack_wrapper.hpp>
#include <frovedis/ml/glm/linear_model.hpp>

namespace frovedis {

// -------- for lapack solvers --------
template <class T>
void preprocess_data(colmajor_matrix<T>& mat,
                     dvector<T>& label,
                     colmajor_matrix_local<T>& Amat,
                     colmajor_matrix_local<T>& Bmat,
                     std::vector<T>& Amean,
                     T& Bmean) {
  // re-alignment label-points
  auto sizes = mat.get_local_num_rows();
  label.align_as(sizes);

  auto nsamples = mat.num_row;
  auto nfeatures = mat.num_col;
  if (nsamples != label.size())
    REPORT_ERROR(USER_ERROR, "Number of samples in input matrix and label vector doesn't match!\n");

  // centerizing feature
  auto rmat = mat.to_rowmajor(); // to_rowmajor(): gather() is supported only on rowmajor_matrix<T> 
  Amean = compute_mean(rmat, 0); // column-wise mean
  centerize(rmat, Amean);
  Amat = colmajor_matrix_local<T>(rmat.gather());
  rmat.clear();

  // centerizing label
  auto label_vec = label.gather();
  auto label_vecp = label_vec.data();
  Bmean = 0.0;
  for(size_t i = 0; i < nsamples; ++i) Bmean += label_vecp[i];
  Bmean /= nsamples;
  if (nfeatures > nsamples) {
    Bmat = colmajor_matrix_local<T>(nfeatures, 1); // extending for solution matrix
    auto bmatvalp = Bmat.val.data();
    for(size_t i = 0; i < nsamples; ++i) bmatvalp[i] = label_vecp[i] - Bmean;
  }
  else {
    for(size_t i = 0; i < nsamples; ++i) label_vecp[i] -= Bmean; // centerized in-place
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
                     T& Bmean) {
  // re-alignment label-points
  auto sizes = mat.get_local_num_rows();
  label.align_as(sizes);

  auto nsamples = mat.num_row;
  auto nfeatures = mat.num_col;
  if (nsamples != label.size())
    REPORT_ERROR(USER_ERROR, "Number of samples in input matrix and label vector doesn't match!\n");

  // centerizing feature
  colmajor_matrix<T> copy_mat(mat);
  Amean = compute_mean(copy_mat, 0); // column-wise mean
  centerize(copy_mat, Amean);
  Amat = blockcyclic_matrix<T>(copy_mat);
  copy_mat.clear();

  // centerizing label
  auto label_vec = label.gather();
  auto label_vecp = label_vec.data();
  Bmean = 0.0;
  for(size_t i = 0; i < nsamples; ++i) Bmean += label_vecp[i];
  Bmean /= nsamples;
  if (nfeatures > nsamples) {
    std::vector<T> tmp(nfeatures); // extending for solution matrix
    auto tmpp = tmp.data();
    for(size_t i = 0; i < nsamples; ++i) tmpp[i] = label_vecp[i] - Bmean;
    Bmat = blockcyclic_matrix<T>(tmp, 2);
  }
  else {         
    for(size_t i = 0; i < nsamples; ++i) label_vecp[i] -= Bmean; // centerized in-place
    Bmat = blockcyclic_matrix<T>(label_vec, 2); // reused label_vec (avoided memory allocation and copy)
  }
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
  T Bmean;
  std::vector<T> Amean;
  colmajor_matrix_local<T> Amat, Bmat;
  preprocess_data(mat, label, Amat, Bmat, Amean, Bmean);
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
  return linear_regression_with_lapack(mat, label, rank, sval, fit_intercept);
}

template <class T>
linear_regression_model<T>
linear_regression_with_scalapack(colmajor_matrix<T>& mat,
                                 dvector<T>& label,
                                 bool fit_intercept = true) {
  T Bmean;
  std::vector<T> Amean;
  blockcyclic_matrix<T> Amat, Bmat;
  preprocess_data(mat, label, Amat, Bmat, Amean, Bmean);
  gels<T>(Amat, Bmat);
  auto r_Bmat = Bmat.to_rowmajor().gather();
  auto model = extract_solution<T, rowmajor_matrix_local<T>>(
                                r_Bmat, mat.num_col);
  if(fit_intercept) set_intercept(model, Amean, Bmean);
  return model;
}

}
#endif
