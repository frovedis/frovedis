#ifndef STANDARD_SCALER_HPP
#define STANDARD_SCALER_HPP

#include <frovedis/matrix/colmajor_matrix.hpp>
#include <frovedis/matrix/crs_matrix.hpp>

namespace frovedis {

template <class T>
struct standard_scaler {
  standard_scaler(bool mean = true, bool std = true, bool sam_std = false):
                  with_mean(mean), with_std(std), sample_stddev(sam_std) { 
                  sample_count = 0;
                  }

  template <class MATRIX>
  standard_scaler<T>& fit(MATRIX& mat);

  template <class MATRIX>
  standard_scaler<T>& partial_fit(MATRIX& mat);

  template <class MATRIX>
  MATRIX transform(MATRIX& mat, bool copy = true);

  template <class MATRIX>
  MATRIX fit_transform(MATRIX& mat, bool copy = true);

  template <class MATRIX>
  MATRIX inverse_transform(MATRIX& trans_mat, bool copy = true);

  template <class MATRIX>
  void increment_mean_and_var(MATRIX& mat, std::vector<T>& last_mean,
                              std::vector<T>& last_var, size_t& sample_count);

  std::vector<T> mean, stddev, one_by_stddev, var;
  bool with_mean, with_std, sample_stddev;
  size_t sample_count;
  SERIALIZE(mean, stddev, one_by_stddev, var, with_mean, with_std,
            sample_stddev, sample_count);
};


template <class T>
template <class MATRIX>
void
standard_scaler<T>::increment_mean_and_var(MATRIX& mat, std::vector<T>& last_mean,
                                           std::vector<T>& last_var, size_t& sample_count) {
  std::vector<T> last_sum, new_sum;
  size_t updated_sample_count;
  if(with_mean || with_std){
    last_sum = last_mean * sample_count;
    new_sum = sum_of_rows(mat);
    updated_sample_count = sample_count + mat.num_row;
    //TODO: Update in place
    last_mean = (last_sum + new_sum) / updated_sample_count;
  }

  if(with_std){
    auto copy_mat = mat;  
    //TODO: Update in place
    auto last_unnormalized_variance = last_var * sample_count;
    auto new_var = compute_var(copy_mat, last_mean, sample_stddev);
    auto new_unnormalized_variance = new_var * mat.num_row;

    T last_over_new_count = sample_count ? (sample_count / mat.num_row) : 0;
    std::vector<T> last_sum_div;  
    if(last_over_new_count == 0) last_sum_div = vector_zeros<T>(mat.num_col);
    else last_sum_div = last_sum / last_over_new_count;
     
    auto updated_unnormalized_variance =
    (last_unnormalized_variance + new_unnormalized_variance +
                  last_over_new_count / updated_sample_count *
                  vector_pow((last_sum_div - new_sum), 2));

    last_var = vector_astype<T>(updated_unnormalized_variance / updated_sample_count);
  } 
  sample_count = updated_sample_count;
}

template <class T>
template <class MATRIX>
standard_scaler<T>&
standard_scaler<T>::fit(MATRIX& mat) {
  sample_count = 0;
  partial_fit(mat);
  return *this;
}

    
template <class T>
template <class MATRIX>
standard_scaler<T>&
standard_scaler<T>::partial_fit(MATRIX& mat) {
  if(mat.num_row < 2)
    throw std::runtime_error("fit: cannot standardize if number of row is 0 or 1");

  if(sample_count == 0)  {//First Pass
    mean = vector_zeros<T>(mat.num_col);
    var = vector_zeros<T>(mat.num_col);
  }
  //update mean and var for this batch  
  increment_mean_and_var(mat, mean, var, sample_count);

  //compute standard deviation 
  stddev = vector_sqrt(var); 
  auto nftr = stddev.size();
  one_by_stddev.clear();
  one_by_stddev.resize(nftr);
  auto stddevp = stddev.data();
  auto one_by_stddevp = one_by_stddev.data();

  for(size_t i = 0; i < nftr; ++i) one_by_stddevp[i] = 1.0 / stddevp[i]; 
 
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
