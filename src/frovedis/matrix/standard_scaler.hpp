#ifndef STANDARD_SCALER_HPP
#define STANDARD_SCALER_HPP

#include <frovedis/matrix/colmajor_matrix.hpp>
#include <frovedis/matrix/crs_matrix.hpp>

namespace frovedis {

template <class T>
struct standard_scaler {
  standard_scaler(bool mean = true, bool std = true, bool sam_std = false):
                  with_mean(mean), with_std(std), sample_stddev(sam_std) {}

  template <class MATRIX>
  standard_scaler<T>& fit(MATRIX& mat);

  template <class MATRIX>
  MATRIX transform(MATRIX& mat, bool copy = true);

  template <class MATRIX>
  MATRIX fit_transform(MATRIX& mat, bool copy = true);

  template <class MATRIX>
  MATRIX inverse_transform(MATRIX& trans_mat, bool copy = true);

  std::vector<T> mean, stddev, one_by_stddev;
  bool with_mean, with_std, sample_stddev;
  SERIALIZE(mean, stddev, one_by_stddev, with_mean, with_std, sample_stddev);
};

template <class T>
template <class MATRIX>
standard_scaler<T>& 
standard_scaler<T>::fit(MATRIX& mat){
  if(mat.num_row < 2)
    throw std::runtime_error("fit: cannot standardize if number of row is 0 or 1");
  if (with_mean || with_std) mean = compute_mean(mat, 0); // column-wise mean
  if (with_std) {
   auto copy_mat = mat;
   stddev = compute_stddev(copy_mat, mean, sample_stddev);
   auto nftr = stddev.size();
    one_by_stddev.clear();
    one_by_stddev.resize(nftr);
    auto stddevp = stddev.data();
    auto one_by_stddevp = one_by_stddev.data();
    // assumption: compute_stddev() should return non-zeros
    for(size_t i = 0; i < nftr; ++i) one_by_stddevp[i] = 1.0 / stddevp[i]; 
  }
  return *this;
}

template <class T>
template <class MATRIX>
MATRIX standard_scaler<T>::transform(MATRIX& mat, bool copy) {
  auto ret = mat;
  if (with_mean) {
    if (mat.num_col != mean.size())
      REPORT_ERROR(USER_ERROR, "Model is either not fitted or computed mean has different size!\n");
    centerize(ret, mean);
  }
  if (with_std) {
    if (mat.num_col != one_by_stddev.size())
      REPORT_ERROR(USER_ERROR, "Model is either not fitted or computed stddev has different size!\n");
    scale_matrix(ret, one_by_stddev);
  }
  return ret;
}

template <class T>
template <class MATRIX>
MATRIX standard_scaler<T>::fit_transform(MATRIX& mat, bool copy) {
  return fit(mat).transform(mat, copy);
}

template <class T>
template <class MATRIX>
MATRIX standard_scaler<T>::inverse_transform(MATRIX& mat, bool copy) {
  auto ret = mat;
  if (with_std) {
    if (mat.num_col != stddev.size())
      REPORT_ERROR(USER_ERROR, "Model is either not fitted or computed stddev has different size!\n");
    scale_matrix(ret, stddev);
  }
  if (with_mean) {
    if (mat.num_col != mean.size())
      REPORT_ERROR(USER_ERROR, "Model is either not fitted or computed mean has different size!\n");
    decenterize(ret, mean);
  }
  return ret;
}

}
#endif
