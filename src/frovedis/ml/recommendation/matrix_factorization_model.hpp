#ifndef _MFM_H_
#define _MFM_H_

#include <fstream>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <algorithm>
#include "../../core/exceptions.hpp"
#include "../../matrix/rowmajor_matrix.hpp"

namespace frovedis {

template <typename T>
class matrix_factorization_model {

public:
  std::vector<T> X;
  std::vector<T> Y;
  size_t numRows, numCols;
  size_t factor;

  matrix_factorization_model() : numRows(0), numCols(0), factor(0) {}
  matrix_factorization_model(size_t nRows, size_t nCols, size_t fctr,
                           size_t seed = 0);
  matrix_factorization_model(const std::string &modelFileName) {
    load(modelFileName);
  }
  matrix_factorization_model(const matrix_factorization_model& cloneModel);
  matrix_factorization_model(matrix_factorization_model&& cloneModel);
  matrix_factorization_model<T>&
  operator=(const matrix_factorization_model& cloneModel);
  matrix_factorization_model<T>&
  operator=(matrix_factorization_model&& cloneModel);

  T predict(const size_t uID, const size_t pID);
  std::vector<T> predict_all
  (const std::vector<std::pair<size_t,size_t>> IDs);

  std::vector<std::pair<size_t, T>> recommend_products(size_t u, int k);
  std::vector<std::pair<size_t, T>> recommend_users(size_t p, int k);

  rowmajor_matrix_local<T> predict_all(); // for debugging
  
  node_local<matrix_factorization_model<T>> broadcast(); // for performance
  void save(const std::string& dir);
  void savebinary(const std::string& dir);
  void load(const std::string& dir);
  void loadbinary(const std::string& dir);
  size_t get_rank() const { return factor; }
  void initialize_model(size_t seed = 0);
  void debug_print() const; // for debugging

  SERIALIZE(X, Y, numRows, numCols, factor)
};

template <class T>
struct mfm_bcast_helper {
  mfm_bcast_helper() {}
  mfm_bcast_helper(size_t nr, size_t nc, size_t f) :
    nrow(nr), ncol(nc), factor(f) {}
  matrix_factorization_model<T> operator()(std::vector<T>& x,
                                         std::vector<T>& y) {
    matrix_factorization_model<T> r;
    r.X.swap(x);
    r.Y.swap(y);
    r.numRows = nrow;
    r.numCols = ncol;
    r.factor = factor;
    return r;
  }
  size_t nrow, ncol, factor;
  SERIALIZE(nrow, ncol, factor)
};

template <typename T>
node_local<matrix_factorization_model<T>>
matrix_factorization_model<T>::broadcast() {
  auto bx = make_node_local_broadcast(X);
  auto by = make_node_local_broadcast(Y);
  return bx.template map<matrix_factorization_model<T>>
                        (mfm_bcast_helper<T>(numRows,numCols,factor),by);
}

template <typename T>
matrix_factorization_model<T>::
matrix_factorization_model(size_t nRows,
                         size_t nCols,
                         size_t fctr,
                         size_t seed) :
  X(nRows*fctr), Y(nCols*fctr), numRows(nRows), numCols(nCols), factor(fctr) {
  checkAssumption(nRows > 0 && nCols > 0 && fctr > 0);
  if(fctr > std::max(nRows,nCols))
    REPORT_WARNING(WARNING_MESSAGE, "Latent factor value is too large\n");
  initialize_model(seed);
}
 
template <typename T>
matrix_factorization_model<T>::
matrix_factorization_model(const matrix_factorization_model& cloneModel) {
  X = cloneModel.X;
  Y = cloneModel.Y;
  numRows = cloneModel.numRows;
  numCols = cloneModel.numCols;
  factor = cloneModel.factor;
}

template <typename T>
matrix_factorization_model<T>::
matrix_factorization_model(matrix_factorization_model&& cloneModel) {
  X.swap(cloneModel.X);
  Y.swap(cloneModel.Y);
  numRows = cloneModel.numRows;
  numCols = cloneModel.numCols;
  factor = cloneModel.factor;
}

template <typename T>
matrix_factorization_model<T>&
matrix_factorization_model<T>::
operator=(const matrix_factorization_model& cloneModel) {
  X = cloneModel.X;
  Y = cloneModel.Y;
  numRows = cloneModel.numRows;
  numCols = cloneModel.numCols;
  factor = cloneModel.factor;
  return *this;
}

template <typename T>
matrix_factorization_model<T>&
matrix_factorization_model<T>::
operator=(matrix_factorization_model&& cloneModel) {
  X.swap(cloneModel.X);
  Y.swap(cloneModel.Y);
  numRows = cloneModel.numRows;
  numCols = cloneModel.numCols;
  factor = cloneModel.factor;
  return *this;
}

template <typename T>
void matrix_factorization_model<T>::initialize_model(size_t seed) {
  srand(seed);
  for(size_t i = 0; i < numRows; i++)
    for(size_t j = 0; j < factor; j++)
      X[i*factor+j] = (T) rand();

   for(size_t i = 0; i < numCols; i++)
     for(size_t j = 0; j < factor; j++)
       Y[i*factor+j] = (T) rand();
}

template <typename T>
void matrix_factorization_model<T>::debug_print() const {
  std::cout << "========== Matrix::X ==========\n";
  for(size_t i = 0; i < numRows; i++) {
    for(size_t j = 0; j < factor; j++)
      std::cout << " " << X[i*factor+j];
    std::cout << "\n";
  }
  std::cout << "========== Matrix::Y ==========\n";
  for(size_t i=0; i < numCols; i++) {
    for(size_t j=0; j < factor; j++)
      std::cout << " " << Y[i*factor+j];
    std::cout << "\n";
  }
}

//-------------------------------------------------
// Predicting Unrated Rating for a particular point
//-------------------------------------------------
template <typename T>
T matrix_factorization_model<T>::predict(const size_t uID,
                                         const size_t pID) {
  std::string errMsg;
  if (uID >= numRows) {
    REPORT_ERROR(USER_ERROR, "User ID is not in range\n");
  }
  if (pID >= numCols) {
    REPORT_ERROR(USER_ERROR,"Product ID is not in range\n");
  }
  T sum = 0.0;
  auto Xp = X.data();
  auto Yp = Y.data();
  for(size_t i = 0; i < factor; i++) {
    sum = sum + Xp[uID*factor+i] * Yp[pID*factor+i];
  }    
  return sum;
}

template <typename T>
std::vector<T>
matrix_factorization_model<T>::
predict_all(const std::vector<std::pair<size_t,size_t>> IDs) {
  size_t totalPoints = IDs.size();
  if(!totalPoints) REPORT_WARNING(WARNING_MESSAGE,"Nothing to predict");
  std::vector<T> result(totalPoints);
  for(size_t i=0; i<totalPoints; i++) {
    result[i] = predict(IDs[i].first, IDs[i].second);
  }
  return result;
}

//------------------------------------------------------
// Predicting ratings for entire ratings matrix: for debugging
//------------------------------------------------------
template <typename T>
rowmajor_matrix_local<T>
matrix_factorization_model<T>::predict_all() {
  rowmajor_matrix_local<T> mX, mY;
  mX.local_num_row = numRows;
  mX.local_num_col = factor;
  mX.val = X;
  mY.local_num_row = numCols;
  mY.local_num_col = factor;
  mY.val = Y;
  auto mYt = mY.transpose();
  return mX * mYt;
}

template <class T>
struct als_sort_by_second {
  bool operator()(const std::pair<size_t, T>& l,
                  const std::pair<size_t, T>& r) {
    return l.second > r.second;
  } 
};

// TODO: parallelize
template <typename T>
std::vector<std::pair<size_t, T>> 
matrix_factorization_model<T>::recommend_products(size_t user, int k) {
  if(k > numCols) k = numCols;
  std::vector<std::pair<size_t, T>> ret;
  for(size_t i = 0; i < numCols; i++) {
    ret.push_back(std::make_pair(i, predict(user, i)));
  }
  std::partial_sort(ret.begin(), ret.begin() + k, ret.end(),
                    als_sort_by_second<T>());
  ret.erase(ret.begin() + k, ret.end());
  return ret;
}

// TODO: parallelize
template <typename T>
std::vector<std::pair<size_t, T>> 
matrix_factorization_model<T>::recommend_users(size_t product, int k) {
  if(k > numRows) k = numRows;
  std::vector<std::pair<size_t, T>> ret;
  for(size_t i = 0; i < numRows; i++) {
    ret.push_back(std::make_pair(i, predict(i, product)));
  }
  std::partial_sort(ret.begin(), ret.begin() + k, ret.end(),
                    als_sort_by_second<T>());
  ret.erase(ret.begin() + k, ret.end());
  return ret;
}

template <typename T>
void matrix_factorization_model<T>::save(const std::string& dir) {
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
  std::string xfile = dir + "/X";
  std::string yfile = dir + "/Y";
  rowmajor_matrix_local<T> mat;
  mat.local_num_row = numRows;
  mat.local_num_col = factor;
  mat.val = X;
  mat.save(xfile);
  mat.local_num_row = numCols;
  mat.local_num_col = factor;
  mat.val = Y;
  mat.save(yfile);
}

template <typename T>
void matrix_factorization_model<T>::savebinary(const std::string& dir) {
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
  std::string xfile = dir + "/X";
  std::string yfile = dir + "/Y";
  rowmajor_matrix_local<T> mat;
  mat.local_num_row = numRows;
  mat.local_num_col = factor;
  mat.val = X;
  mat.savebinary(xfile);
  mat.local_num_row = numCols;
  mat.local_num_col = factor;
  mat.val = Y;
  mat.savebinary(yfile);
}

template <typename T>
void matrix_factorization_model<T>::load(const std::string &dir) {
  auto tmp = make_rowmajor_matrix_local_load<T>(dir + "/X");
  numRows = tmp.local_num_row;
  factor = tmp.local_num_col;
  X.swap(tmp.val);
  tmp = make_rowmajor_matrix_local_load<T>(dir + "/Y");
  numCols = tmp.local_num_row;
  Y.swap(tmp.val);
}

template <typename T>
void matrix_factorization_model<T>::loadbinary(const std::string &dir) {
  auto tmp = make_rowmajor_matrix_local_loadbinary<T>(dir + "/X");
  numRows = tmp.local_num_row;
  factor = tmp.local_num_col;
  X.swap(tmp.val);
  tmp = make_rowmajor_matrix_local_loadbinary<T>(dir + "/Y");
  numCols = tmp.local_num_row;
  Y.swap(tmp.val);
}

}
#endif
