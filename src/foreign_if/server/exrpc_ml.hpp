// ---------------------------------------------------------------------
// NOTE: This file contains completely template-based routines.
// Based on the input argumnet type, e.g., float/double (DT1/DT2)
// sparse/dense (S_MAT1/D_MAT1) the template call will be deduced.
// thus during the support of float type or dense type data, no major
// changes need to be performed in this file.
// ---------------------------------------------------------------------

#ifndef _EXRPC_ML_HPP_
#define _EXRPC_ML_HPP_

#include "frovedis.hpp"
#include "frovedis/ml/glm/logistic_regression_with_sgd.hpp"
#include "frovedis/ml/glm/logistic_regression_with_lbfgs.hpp"
#include "frovedis/ml/glm/svm_with_sgd.hpp"
#include "frovedis/ml/glm/svm_with_lbfgs.hpp"
#include "frovedis/ml/glm/linear_regression_with_sgd.hpp"
#include "frovedis/ml/glm/linear_regression_with_lbfgs.hpp"
#include "frovedis/ml/glm/lasso_with_sgd.hpp"
#include "frovedis/ml/glm/lasso_with_lbfgs.hpp"
#include "frovedis/ml/glm/ridge_regression_with_sgd.hpp"
#include "frovedis/ml/glm/ridge_regression_with_lbfgs.hpp"
#include "frovedis/ml/recommendation/als.hpp"
#include "frovedis/ml/clustering/kmeans.hpp"
#include "../exrpc/exrpc_expose.hpp"
#include "frovedis_mem_pair.hpp"
#include "model_tracker.hpp"

using namespace frovedis;

// --- Handling of Training input/output ---
template <class T>
inline void clear_mat_data(crs_matrix_local<T>& data) {
  std::vector<T> tmp1;      tmp1.swap(data.val);
  std::vector<size_t> tmp2; tmp2.swap(data.idx);
  std::vector<size_t> tmp3; tmp3.swap(data.off);
}

template <class T>
inline void clear_lbl_data(std::vector<T>& data) {
  std::vector<T> tmp; tmp.swap(data);
}

template <class MODEL>
inline void handle_trained_model(int mid, MODEL_KIND mkind, MODEL& model) {
#ifdef _EXRPC_DEBUG_
  std::cout << "training request [" << mid << "] is processed at Frovedis side.\n";
#endif
  auto mptr = new MODEL(std::move(model));
  if(!mptr) REPORT_ERROR(INTERNAL_ERROR,"memory allocation failed!\n");
  auto mptr_ = reinterpret_cast<exrpc_ptr_t>(mptr);
  register_model(mid, mkind, mptr_); // registering the trained model in model_table
  unregister_from_train(mid); // and unregistering it from "under training" state
}

// --- Frovedis ML Trainer Calls ---
template <class T>
void transform_output_label(std::vector<T>& val) {
  size_t sz = val.size();
  T* valp = &val[0];
  for(size_t i=0; i<sz; ++i) valp[i] = (valp[i] == (T)0) ? -1 : valp[i];
}

template <class T, class MATRIX>
void frovedis_lr_sgd(frovedis_mem_pair& mp, int& numIter, T& stepSize, 
                   T& mbf, int& regType, T& regParam, 
                   bool& isIntercept, double& tol, int& verbose, int& mid, 
                   bool& isMovableInput=false) {
  register_for_train(mid);  // mark model 'mid' as "under training"
  // extracting input data
  MATRIX& mat = *reinterpret_cast<MATRIX*>(mp.first());
  dvector<T> lbl = *reinterpret_cast<dvector<T>*>(mp.second()); // copying 
  lbl.mapv_partitions(transform_output_label<T>); // transforming copied dvector
  logistic_regression_model<T> m;      // trained output model holder

  auto old_level = frovedis::get_loglevel();
  if (verbose == 1) frovedis::set_loglevel(frovedis::DEBUG);
  else if (verbose == 2) frovedis::set_loglevel(frovedis::TRACE);

  frovedis::RegType rtype = ZERO;
  if (regType == 1) rtype = L1;
  else if (regType == 2) rtype = L2;

  if (isMovableInput) {
    m = logistic_regression_with_sgd::train(std::move(mat),lbl,numIter,stepSize,
                                            mbf,regParam,rtype,isIntercept,tol);
    lbl.mapv_partitions(clear_lbl_data<T>); 
  }
  else m = logistic_regression_with_sgd::train(mat,lbl,numIter,stepSize,mbf,
                                               regParam,rtype,isIntercept,tol);
  frovedis::set_loglevel(old_level);
  handle_trained_model<logistic_regression_model<T>>(mid, LRM, m);
}

template <class T, class MATRIX>
void frovedis_lr_lbfgs(frovedis_mem_pair& mp, int& numIter, T& stepSize, 
                     int& histSize, int& regType, T& regParam, 
                     bool& isIntercept, double& tol, int& verbose, int& mid,
                     bool& isMovableInput=false) {
  register_for_train(mid);  // mark model 'mid' as "under training"
  // extracting input data
  MATRIX& mat = *reinterpret_cast<MATRIX*>(mp.first());
  dvector<T> lbl = *reinterpret_cast<dvector<T>*>(mp.second()); // copying 
  lbl.mapv_partitions(transform_output_label<T>); // transforming copied dvector
  logistic_regression_model<T> m;                // trained output model holder

  auto old_level = frovedis::get_loglevel();
  if (verbose == 1) frovedis::set_loglevel(frovedis::DEBUG);
  else if (verbose == 2) frovedis::set_loglevel(frovedis::TRACE);

  frovedis::RegType rtype = ZERO;
  if (regType == 1) rtype = L1;
  else if (regType == 2) rtype = L2;

  if (isMovableInput) {
    m = logistic_regression_with_lbfgs::train(std::move(mat),lbl,numIter,stepSize,
                                           histSize,regParam,rtype,
                                           isIntercept,tol);
    mat.data.mapv(clear_mat_data<T>); // is a no-operation, if already freed.
    lbl.mapv_partitions(clear_lbl_data<T>);
  }
  else m = logistic_regression_with_lbfgs::train(mat,lbl,numIter,stepSize,
                                              histSize,regParam,rtype,
                                              isIntercept,tol);
  frovedis::set_loglevel(old_level);
  handle_trained_model<logistic_regression_model<T>>(mid, LRM, m);
}

template <class T, class MATRIX>
void frovedis_svm_sgd(frovedis_mem_pair& mp, int& numIter, T& stepSize, 
                    T& mbf, int& regType, T& regParam, 
                    bool& isIntercept, double& tol, int& verbose, int& mid,
                    bool& isMovableInput=false) {
  register_for_train(mid);  // mark model 'mid' as "under training"
  // extracting input data
  MATRIX& mat = *reinterpret_cast<MATRIX*>(mp.first());
  dvector<T> lbl = *reinterpret_cast<dvector<T>*>(mp.second()); // copying 
  lbl.mapv_partitions(transform_output_label<T>); // transforming copied dvector
  svm_model<T> m;  // trained output model holder

  auto old_level = frovedis::get_loglevel();
  if (verbose == 1) frovedis::set_loglevel(frovedis::DEBUG);
  else if (verbose == 2) frovedis::set_loglevel(frovedis::TRACE);

  frovedis::RegType rtype = ZERO;
  if (regType == 1) rtype = L1;
  else if (regType == 2) rtype = L2;

  if (isMovableInput) {
    m = svm_with_sgd::train(std::move(mat),lbl,numIter,stepSize,
                            mbf,regParam,rtype,isIntercept,tol);
    lbl.mapv_partitions(clear_lbl_data<T>); 
  }
  else m = svm_with_sgd::train(mat,lbl,numIter,stepSize,
                               mbf,regParam,rtype,isIntercept,tol);
  frovedis::set_loglevel(old_level);
  handle_trained_model<svm_model<T>>(mid, SVM, m);
}

template <class T, class MATRIX>
void frovedis_svm_lbfgs(frovedis_mem_pair& mp, int& numIter, T& stepSize, 
                      int& histSize, int& regType, T& regParam, 
                      bool& isIntercept, double& tol, int& verbose, int& mid,
                      bool& isMovableInput=false) {
  register_for_train(mid);  // mark model 'mid' as "under training"
  // extracting input data
  MATRIX& mat = *reinterpret_cast<MATRIX*>(mp.first());
  dvector<T> lbl = *reinterpret_cast<dvector<T>*>(mp.second()); // copying 
  lbl.mapv_partitions(transform_output_label<T>); // transforming copied dvector
  svm_model<T> m;  // trained output model holder

  auto old_level = frovedis::get_loglevel();
  if (verbose == 1) frovedis::set_loglevel(frovedis::DEBUG);
  else if (verbose == 2) frovedis::set_loglevel(frovedis::TRACE);

  frovedis::RegType rtype = ZERO;
  if (regType == 1) rtype = L1;
  else if (regType == 2) rtype = L2;

  if (isMovableInput) {
    m = svm_with_lbfgs::train(std::move(mat),lbl,numIter,stepSize,
                              histSize,regParam,rtype,isIntercept,tol);
    mat.data.mapv(clear_mat_data<T>); // is a no-operation, if already freed.
    lbl.mapv_partitions(clear_lbl_data<T>);
  }
  else m = svm_with_lbfgs::train(mat,lbl,numIter,stepSize,
                                 histSize,regParam,rtype,isIntercept,tol);
  frovedis::set_loglevel(old_level);
  handle_trained_model<svm_model<T>>(mid, SVM, m);
}

template <class T, class MATRIX>
void frovedis_lnr_sgd(frovedis_mem_pair& mp, int& numIter, T& stepSize, 
                    T& mbf,
                    bool& isIntercept, int& verbose, int& mid, 
                    bool& isMovableInput=false) {
  register_for_train(mid);  // mark model 'mid' as "under training"
  // extracting input data
  MATRIX& mat = *reinterpret_cast<MATRIX*>(mp.first());
  dvector<T> lbl = *reinterpret_cast<dvector<T>*>(mp.second()); // copying 
  lbl.mapv_partitions(transform_output_label<T>); // transforming copied dvector
  linear_regression_model<T> m;   // trained output model holder

  auto old_level = frovedis::get_loglevel();
  if (verbose == 1) frovedis::set_loglevel(frovedis::DEBUG);
  else if (verbose == 2) frovedis::set_loglevel(frovedis::TRACE);

  if (isMovableInput) {
    m = linear_regression_with_sgd::train(std::move(mat),lbl,numIter,stepSize,
                                          mbf,isIntercept);
    lbl.mapv_partitions(clear_lbl_data<T>); 
  }
  else m = linear_regression_with_sgd::train(mat,lbl,numIter,stepSize,
                                             mbf,isIntercept);
  frovedis::set_loglevel(old_level);
  handle_trained_model<linear_regression_model<T>>(mid, LNRM, m);
}

template <class T, class MATRIX>
void frovedis_lnr_lbfgs(frovedis_mem_pair& mp, int& numIter, T& stepSize, 
                      int& histSize,
                      bool& isIntercept, int& verbose, int& mid, 
                      bool& isMovableInput=false) {
  register_for_train(mid);  // mark model 'mid' as "under training"
  // extracting input data
  MATRIX& mat = *reinterpret_cast<MATRIX*>(mp.first());
  dvector<T> lbl = *reinterpret_cast<dvector<T>*>(mp.second()); // copying 
  lbl.mapv_partitions(transform_output_label<T>); // transforming copied dvector
  linear_regression_model<T> m;   // trained output model holder

  auto old_level = frovedis::get_loglevel();
  if (verbose == 1) frovedis::set_loglevel(frovedis::DEBUG);
  else if (verbose == 2) frovedis::set_loglevel(frovedis::TRACE);

  if (isMovableInput) {
    m = linear_regression_with_lbfgs::train(std::move(mat),lbl,numIter,stepSize,
                                            histSize,isIntercept);
    mat.data.mapv(clear_mat_data<T>); // is a no-operation, if already freed.
    lbl.mapv_partitions(clear_lbl_data<T>);
  }
  else m = linear_regression_with_lbfgs::train(mat,lbl,numIter,stepSize,
                                               histSize,isIntercept);
  frovedis::set_loglevel(old_level);
  handle_trained_model<linear_regression_model<T>>(mid, LNRM, m);
}

template <class T, class MATRIX>
void frovedis_lasso_sgd(frovedis_mem_pair& mp, int& numIter, T& stepSize, 
                      T& mbf, T& regParam,
                      bool& isIntercept, double& tol, int& verbose, int& mid, 
                      bool& isMovableInput=false) {
  register_for_train(mid);  // mark model 'mid' as "under training"
  // extracting input data
  MATRIX& mat = *reinterpret_cast<MATRIX*>(mp.first());
  dvector<T> lbl = *reinterpret_cast<dvector<T>*>(mp.second()); // copying 
  lbl.mapv_partitions(transform_output_label<T>); // transforming copied dvector
  linear_regression_model<T> m;   // trained output model holder

  auto old_level = frovedis::get_loglevel();
  if (verbose == 1) frovedis::set_loglevel(frovedis::DEBUG);
  else if (verbose == 2) frovedis::set_loglevel(frovedis::TRACE);

  if (isMovableInput) {
    m = lasso_with_sgd::train(std::move(mat),lbl,numIter,stepSize,
                              mbf,regParam,isIntercept,tol);
    lbl.mapv_partitions(clear_lbl_data<T>); 
  }
  else m = lasso_with_sgd::train(mat,lbl,numIter,stepSize,
                                 mbf,regParam,isIntercept,tol);
  frovedis::set_loglevel(old_level);
  handle_trained_model<linear_regression_model<T>>(mid, LNRM, m);
}

template <class T, class MATRIX>
void frovedis_lasso_lbfgs(frovedis_mem_pair& mp, int& numIter, T& stepSize, 
                        int& histSize, T& regParam,
                        bool& isIntercept, double& tol, int& verbose, int& mid, 
                        bool& isMovableInput=false) {
  register_for_train(mid);  // mark model 'mid' as "under training"
  // extracting input data
  MATRIX& mat = *reinterpret_cast<MATRIX*>(mp.first());
  dvector<T> lbl = *reinterpret_cast<dvector<T>*>(mp.second()); // copying 
  lbl.mapv_partitions(transform_output_label<T>); // transforming copied dvector
  linear_regression_model<T> m;   // trained output model holder

  auto old_level = frovedis::get_loglevel();
  if (verbose == 1) frovedis::set_loglevel(frovedis::DEBUG);
  else if (verbose == 2) frovedis::set_loglevel(frovedis::TRACE);

  if (isMovableInput) {
    m = lasso_with_lbfgs::train(std::move(mat),lbl,numIter,stepSize,
                                histSize,regParam,isIntercept,tol);
    mat.data.mapv(clear_mat_data<T>); // is a no-operation, if already freed.
    lbl.mapv_partitions(clear_lbl_data<T>);
  }
  else m = lasso_with_lbfgs::train(mat,lbl,numIter,stepSize,
                                   histSize,regParam,isIntercept,tol);
  frovedis::set_loglevel(old_level);
  handle_trained_model<linear_regression_model<T>>(mid, LNRM, m);
}

template <class T, class MATRIX>
void frovedis_ridge_sgd(frovedis_mem_pair& mp, int& numIter, T& stepSize, 
                      T& mbf, T& regParam, 
                      bool& isIntercept, double& tol, int& verbose, int& mid,
                      bool& isMovableInput=false) {
  register_for_train(mid);  // mark model 'mid' as "under training"
  // extracting input data
  MATRIX& mat = *reinterpret_cast<MATRIX*>(mp.first());
  dvector<T> lbl = *reinterpret_cast<dvector<T>*>(mp.second()); // copying 
  lbl.mapv_partitions(transform_output_label<T>); // transforming copied dvector
  linear_regression_model<T> m;   // trained output model holder

  auto old_level = frovedis::get_loglevel();
  if (verbose == 1) frovedis::set_loglevel(frovedis::DEBUG);
  else if (verbose == 2) frovedis::set_loglevel(frovedis::TRACE);

  if (isMovableInput) {
    m = ridge_regression_with_sgd::train(std::move(mat),lbl,numIter,stepSize,
                                         mbf,regParam,isIntercept,tol);
    lbl.mapv_partitions(clear_lbl_data<T>); 
  }
  else m = ridge_regression_with_sgd::train(mat,lbl,numIter,stepSize,
                                            mbf,regParam,isIntercept,tol);
  frovedis::set_loglevel(old_level);
  handle_trained_model<linear_regression_model<T>>(mid, LNRM, m);
}

template <class T, class MATRIX>
void frovedis_ridge_lbfgs(frovedis_mem_pair& mp, int& numIter, T& stepSize, 
                        int& histSize, T& regParam,
                        bool& isIntercept, double& tol, int& verbose, int& mid,
                        bool& isMovableInput=false) {
  register_for_train(mid);  // mark model 'mid' as "under training"
  // extracting input data
  MATRIX& mat = *reinterpret_cast<MATRIX*>(mp.first());
  dvector<T> lbl = *reinterpret_cast<dvector<T>*>(mp.second()); // copying 
  lbl.mapv_partitions(transform_output_label<T>); // transforming copied dvector
  linear_regression_model<T> m;   // trained output model holder

  auto old_level = frovedis::get_loglevel();
  if (verbose == 1) frovedis::set_loglevel(frovedis::DEBUG);
  else if (verbose == 2) frovedis::set_loglevel(frovedis::TRACE);

  if (isMovableInput) {
    m = ridge_regression_with_lbfgs::train(std::move(mat),lbl,numIter,stepSize,
                                           histSize,regParam,isIntercept,tol);
    mat.data.mapv(clear_mat_data<T>); // is a no-operation, if already freed.
    lbl.mapv_partitions(clear_lbl_data<T>);
  }
  else m = ridge_regression_with_lbfgs::train(mat,lbl,numIter,stepSize,
                                              histSize,regParam,isIntercept,tol);
  frovedis::set_loglevel(old_level);
  handle_trained_model<linear_regression_model<T>>(mid, LNRM, m);
}

template <class T, class MATRIX>
void frovedis_mf_als(exrpc_ptr_t& data_ptr, int& rank, int& numIter, T& alpha,
                   T& regParam, long& seed, int& verbose, int& mid, 
                   bool& isMovableInput=false) {
  register_for_train(mid);   // mark model 'mid' as "under training"
  MATRIX& mat = *reinterpret_cast<MATRIX*>(data_ptr);   // training input data holder
  auto old_level = frovedis::get_loglevel();
  if (verbose == 1) frovedis::set_loglevel(frovedis::DEBUG);
  else if (verbose == 2) frovedis::set_loglevel(frovedis::TRACE);

  auto m = matrix_factorization_using_als::train(mat,rank,numIter,
                                                 alpha,regParam,seed);
  // if input is movable, destroying Frovedis side data after training is done.
  if (isMovableInput)  mat.data.mapv(clear_mat_data<T>); 
  frovedis::set_loglevel(old_level);
  handle_trained_model<matrix_factorization_model<T>>(mid, MFM, m);
}

template <class T, class MATRIX>
void frovedis_kmeans(exrpc_ptr_t& data_ptr, int& k, int& numIter, long& seed,
                   T& epsilon, int& verbose, int& mid, 
                   bool& isMovableInput=false) {
  register_for_train(mid);   // mark model 'mid' as "under training"
  MATRIX& mat = *reinterpret_cast<MATRIX*>(data_ptr);  // training input data holder
  auto old_level = frovedis::get_loglevel();
  if (verbose == 1) frovedis::set_loglevel(frovedis::DEBUG);
  else if (verbose == 2) frovedis::set_loglevel(frovedis::TRACE);

  auto m = frovedis::kmeans(mat,k,numIter,epsilon,seed);
  // if input is movable, destroying Frovedis side data after training is done.
  if (isMovableInput)  mat.data.mapv(clear_mat_data<T>); 
  frovedis::set_loglevel(old_level);
  handle_trained_model<rowmajor_matrix_local<T>>(mid, KMEANS, m);
}

#endif
