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
#include "frovedis/ml/glm/multinomial_logistic_regression.hpp"
#include "frovedis/ml/glm/logistic_regression.hpp"
#include "frovedis/ml/glm/ridge_regression.hpp"
#include "frovedis/ml/glm/linear_svc.hpp"
#include "frovedis/ml/glm/linear_svr.hpp"
#include "frovedis/ml/glm/linear_regression.hpp"
#include "frovedis/ml/glm/lasso.hpp"
#include "frovedis/ml/recommendation/als.hpp"
#include "frovedis/ml/clustering/agglomerative.hpp"
#include "frovedis/ml/clustering/spectral_clustering.hpp"
#include "frovedis/ml/clustering/spectral_embedding.hpp"
#include "frovedis/ml/clustering/kmeans.hpp"
#include "frovedis/ml/clustering/dbscan.hpp"
#include "frovedis/ml/clustering/gmm.hpp"
#include "frovedis/ml/tree/tree.hpp"
#include "frovedis/ml/fm/main.hpp"
#include "frovedis/ml/w2v/word2vec.hpp"
#include "frovedis/ml/nb/naive_bayes.hpp"
#include "frovedis/ml/fpm/fp_growth.hpp"
#include "frovedis/dataframe.hpp"
#include "frovedis/dataframe/dftable_to_dvector.hpp"
#include "frovedis/ml/neighbors/knn_unsupervised.hpp"
#include "frovedis/ml/neighbors/knn_supervised.hpp"
#include "frovedis/ml/lda/lda_cgs.hpp"
#include "frovedis/ml/kernel/kernel_svm.hpp"

#include "../exrpc/exrpc_expose.hpp"
#include "frovedis_mem_pair.hpp"
#include "model_tracker.hpp"
#include "lda_result.hpp"
#include "ml_result.hpp"

using namespace frovedis;

// --- defined in expose_ml.cc ---
RegType get_regularizer(int regType);

std::vector<float> frovedis_w2v_train(std::string& encode,
                                      std::string& count,
                                      w2v::train_config& config);
// -------------------------------

// --- Handling of Training input/output ---
template <class T>
inline void clear_lbl_data(std::vector<T>& data) {
  std::vector<T> tmp; tmp.swap(data);
}

inline std::string get_regularizer_type(int rType) {
  if (rType == 0) return "ZERO";
  else if (rType == 1) return "L1";
  else if (rType == 2) return "L2";
  else REPORT_ERROR(USER_ERROR,
       "Currently frovedis supports ZERO/L1/L2 regularizer!");
  return "";
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
}

// --- Frovedis ML Trainer Calls ---
template <class T, class MATRIX>
size_t frovedis_lr(frovedis_mem_pair& mp, glm_config& config, 
                   int& verbose, int& mid,
                   std::vector<T>& sample_weight, 
                   bool& isMovableInput=false) {

  // extracting input data
  MATRIX& mat = *reinterpret_cast<MATRIX*>(mp.first());
  dvector<T>& lbl = *reinterpret_cast<dvector<T>*>(mp.second()); 

  set_verbose_level(verbose);
  auto is_fitted = is_registered_model(mid);
  logistic_regression<T> *m = is_fitted ? 
           get_model_ptr<logistic_regression<T>>(mid) :
           new logistic_regression<T>();
  m->set_params(config);

  if (isMovableInput) {
    m->fit(std::move(mat), lbl, sample_weight);
    lbl.mapv_partitions(clear_lbl_data<T>); 
  }
  else m->fit(mat, lbl, sample_weight);
  auto n_iter =  m->n_iter_;
  handle_trained_model<logistic_regression<T>>(mid, LR, *m);
  reset_verbose_level();
  return n_iter;
}

template <class T, class MATRIX>
size_t frovedis_svm(frovedis_mem_pair& mp, glm_config& config, 
                    int& verbose, int& mid,
                    std::vector<T>& sample_weight, 
                    bool& isMovableInput=false) {
  // extracting input data
  MATRIX& mat = *reinterpret_cast<MATRIX*>(mp.first());
  dvector<T>& lbl = *reinterpret_cast<dvector<T>*>(mp.second()); 
  auto is_fitted = is_registered_model(mid);
  linear_svm_classifier<T> *m = is_fitted ?
           get_model_ptr<linear_svm_classifier<T>>(mid) :
           new linear_svm_classifier<T>();
  m->set_params(config);
  set_verbose_level(verbose);
 
  size_t n_iter = 0;
  if (isMovableInput) {
    m->fit(std::move(mat), lbl, sample_weight); 
    lbl.mapv_partitions(clear_lbl_data<T>); 
  }
  else m->fit(mat, lbl, sample_weight);
  n_iter = m->n_iter_;
  reset_verbose_level();
  handle_trained_model<linear_svm_classifier<T>>(mid, SVM, *m);
  return n_iter;
}

template <class T, class MATRIX>
size_t frovedis_svm_regressor(frovedis_mem_pair& mp, glm_config& config, 
                              int& verbose,
                              int& mid,
                              std::vector<T>& sample_weight, 
                              bool& isMovableInput=false) {
  // extracting input data
  MATRIX& mat = *reinterpret_cast<MATRIX*>(mp.first());
  dvector<T>& lbl = *reinterpret_cast<dvector<T>*>(mp.second());
  auto is_fitted = is_registered_model(mid);
  linear_svm_regressor<T> *m = is_fitted ?
           get_model_ptr<linear_svm_regressor<T>>(mid) :
           new linear_svm_regressor<T>();
  m->set_params(config);
  set_verbose_level(verbose);

  size_t n_iter = 0;
  if (isMovableInput) {
    m->fit(std::move(mat), lbl, sample_weight);
    lbl.mapv_partitions(clear_lbl_data<T>);
  }
  else m->fit(mat, lbl, sample_weight); 
  n_iter = m->n_iter_;
  reset_verbose_level();
  handle_trained_model<linear_svm_regressor<T>>(mid, SVR, *m);
  return n_iter;
}

template <class T, class MATRIX>
void frovedis_svc(frovedis_mem_pair& mp, double& tol, double& C, 
                  int& cache_size,
                  int& max_iter, std::string& kernel, 
                  double& gamma, double& coef0,
                  int& degree, int& verbose, int& mid, 
                  bool& isMovableInput=false) {
  // extracting input data
  MATRIX& matrix = *reinterpret_cast<MATRIX*>(mp.first());
  dvector<T>& label = *reinterpret_cast<dvector<T>*>(mp.second());
  auto lbl = label.gather();
  auto mat = matrix.gather();

  set_verbose_level(verbose);
 
  frovedis::kernel_csvc_model<T> model(tol, C, cache_size, max_iter, 
                                       kernel, gamma, coef0, degree);
  model.train(mat, lbl);
  if (isMovableInput) {
    matrix.clear();
    label.mapv_partitions(clear_lbl_data<T>);
  }
  reset_verbose_level();
  handle_trained_model<kernel_csvc_model<T>>(mid, KSVC, model);
}

template <class T, class MATRIX>
std::vector<T> 
frovedis_lnr_lapack(frovedis_mem_pair& mp,
                    bool& isIntercept, int& verbose, int& mid,
                    std::vector<T>& sample_weight, 
                    bool& isMovableInput=false) {
  // extracting input data
  auto matptr = reinterpret_cast<MATRIX*>(mp.first());
  auto lblptr = reinterpret_cast<dvector<T>*>(mp.second());

  set_verbose_level(verbose);

  linear_regression<T> m;
  m.set_intercept(isIntercept);
  m.set_solver("lapack");
  int rank;
  std::vector<T> sval;
  m.fit(*matptr, *lblptr, rank, sval, sample_weight); 
  sval.push_back(static_cast<T>(rank));
  if (isMovableInput) {
    delete matptr; delete lblptr;
  }
  reset_verbose_level();
  handle_trained_model<linear_regression<T>>(mid, LNRM, m);
  return sval;
}

template <class T, class MATRIX>
void frovedis_lnr_scalapack(frovedis_mem_pair& mp,
                            bool& isIntercept, int& verbose, int& mid,
                            std::vector<T>& sample_weight, 
                            bool& isMovableInput=false) {
  // extracting input data
  auto matptr = reinterpret_cast<MATRIX*>(mp.first());
  auto lblptr = reinterpret_cast<dvector<T>*>(mp.second());

  set_verbose_level(verbose);

  linear_regression<T> m;
  m.set_intercept(isIntercept);
  m.set_solver("scalapack");
  m.fit(*matptr, *lblptr, sample_weight); 
  if (isMovableInput) {
    delete matptr; delete lblptr;
  }
  reset_verbose_level();
  handle_trained_model<linear_regression<T>>(mid, LNRM, m);
}

template <class T, class MATRIX>
size_t frovedis_lnr(frovedis_mem_pair& mp, glm_config& config, 
                    int& verbose, int& mid, 
                    std::vector<T>& sample_weight, 
                    bool& isMovableInput=false) {
  // extracting input data
  MATRIX& mat = *reinterpret_cast<MATRIX*>(mp.first());
  dvector<T>& lbl = *reinterpret_cast<dvector<T>*>(mp.second()); 
  auto is_fitted = is_registered_model(mid);
  linear_regression<T> *m = is_fitted ?
           get_model_ptr<linear_regression<T>>(mid) :
           new linear_regression<T>();
  m->set_params(config);

   set_verbose_level(verbose);
 
  size_t n_iter = 0;
  if (isMovableInput) {
    m->fit(std::move(mat),lbl,sample_weight);
    lbl.mapv_partitions(clear_lbl_data<T>); 
  }
  else m->fit(mat,lbl,sample_weight);
  n_iter = m->n_iter_;
  reset_verbose_level();
  handle_trained_model<linear_regression<T>>(mid, LNRM, *m);
  return n_iter;
}

template <class T, class MATRIX>
size_t frovedis_lasso(frovedis_mem_pair& mp, glm_config& config, 
                        int& verbose, int& mid, 
                        std::vector<T>& sample_weight, 
                        bool& isMovableInput=false) {
  // extracting input data
  MATRIX& mat = *reinterpret_cast<MATRIX*>(mp.first());
  dvector<T>& lbl = *reinterpret_cast<dvector<T>*>(mp.second());  

  auto is_fitted = is_registered_model(mid);
  lasso_regression<T> *m = is_fitted ?
           get_model_ptr<lasso_regression<T>>(mid) :
           new lasso_regression<T>();
  m->set_params(config);

  set_verbose_level(verbose);

  size_t n_iter = 0;
  if (isMovableInput) {
    m->fit(std::move(mat),lbl,sample_weight);
    lbl.mapv_partitions(clear_lbl_data<T>); 
  }
  else m->fit(mat,lbl,sample_weight);
  n_iter = m->n_iter_;
  reset_verbose_level();
  handle_trained_model<lasso_regression<T>>(mid, LSR, *m);
  return n_iter;
}

template <class T, class MATRIX>
size_t frovedis_ridge(frovedis_mem_pair& mp, glm_config& config,
                      int& verbose, int& mid,
                      std::vector<T>& sample_weight,
                      bool& isMovableInput=false) {
  // extracting input data
  MATRIX& mat = *reinterpret_cast<MATRIX*>(mp.first());
  dvector<T>& lbl = *reinterpret_cast<dvector<T>*>(mp.second());
  auto is_fitted = is_registered_model(mid);
  ridge_regression<T> *m = is_fitted ?
           get_model_ptr<ridge_regression<T>>(mid) :
           new ridge_regression<T>();
  m->set_params(config);

  set_verbose_level(verbose);

  size_t n_iter = 0;
  if (isMovableInput) {
    m->fit(std::move(mat), lbl, sample_weight);
    lbl.mapv_partitions(clear_lbl_data<T>);
  }
  else  m->fit(mat, lbl, sample_weight);
  n_iter = m->n_iter_;
  reset_verbose_level();
  handle_trained_model<ridge_regression<T>>(mid, RR, *m);
  return n_iter;

}

template <class T, class MATRIX>
void frovedis_mf_als(exrpc_ptr_t& data_ptr, int& rank, int& numIter, 
                     double& alpha, double& regParam, double& sim_factor, 
                     long& seed, int& verbose, int& mid, 
                     bool& isMovableInput=false) {
  MATRIX& mat = *reinterpret_cast<MATRIX*>(data_ptr);   // training input data holder
  auto old_level = frovedis::get_loglevel();
  if (verbose == 1) frovedis::set_loglevel(frovedis::DEBUG);
  else if (verbose == 2) frovedis::set_loglevel(frovedis::TRACE);
  auto m = matrix_factorization_using_als::train(mat,rank,numIter,
                                                 alpha,regParam,seed,sim_factor);
  // if input is movable, destroying Frovedis side data after training is done.
  if (isMovableInput)  mat.clear(); 
  frovedis::set_loglevel(old_level);
  handle_trained_model<matrix_factorization_model<T>>(mid, MFM, m);
}

template <class T, class MATRIX>
kmeans_result 
frovedis_kmeans_fit(exrpc_ptr_t& data_ptr, int& k, 
                    int& numIter, int& numInit,
                    double& epsilon, long& seed,
                    int& verbose, int& mid,
                    bool& use_shrink=false,
                    bool& isMovableInput=false) {
  MATRIX& mat = *reinterpret_cast<MATRIX*>(data_ptr);  // training input data holder
  set_verbose_level(verbose);
  auto est = KMeans<T>(k).set_max_iter(numIter)
                         .set_n_init(numInit)
                         .set_eps(epsilon)
                         .set_seed(seed)
                         .set_use_shrink(use_shrink);
  est.fit(mat);
  if (isMovableInput) mat.clear(); 
  reset_verbose_level();
  auto ret = kmeans_result(est.labels_(), est.n_iter_(), 
                           est.inertia_(),
                           est.n_clusters_(), 0, 0);
  handle_trained_model<KMeans<T>>(mid, KMEANS, est);
  return ret;
}

template <class T, class MATRIX>
kmeans_result
frovedis_kmeans_fit_transform(exrpc_ptr_t& data_ptr, int& k,
                              int& numIter, int& numInit,
                              double& epsilon, long& seed,
                              int& verbose, int& mid,
                              bool& use_shrink=false,
                              bool& isMovableInput=false) {
  MATRIX& mat = *reinterpret_cast<MATRIX*>(data_ptr);  // training input data holder
  set_verbose_level(verbose);
  auto est = KMeans<T>(k).set_max_iter(numIter)
                         .set_n_init(numInit)
                         .set_eps(epsilon)
                         .set_seed(seed)
                         .set_use_shrink(use_shrink);
  auto trans_matp = new rowmajor_matrix<T>(est.fit_transform(mat));
  auto trans_matptr = reinterpret_cast<exrpc_ptr_t>(trans_matp);
  if (isMovableInput) mat.clear();
  reset_verbose_level();
  auto ret = kmeans_result(est.labels_(), est.n_iter_(),
                           est.inertia_(),
                           est.n_clusters_(), 
                           trans_matptr, mat.num_row);
  handle_trained_model<KMeans<T>>(mid, KMEANS, est);
  return ret;
}

// knn - nearest neighbors (NN)
template <class T, class MATRIX>
void frovedis_knn(exrpc_ptr_t& data_ptr, int& k, 
                  float& radius,
                  std::string& algorithm, 
                  std::string& metric, 
                  float& chunk_size,
                  int& verbose, 
                  int& mid) {
  MATRIX& mat = *reinterpret_cast<MATRIX*>(data_ptr);  // training input data holder
  set_verbose_level(verbose);

  nearest_neighbors<T, MATRIX> obj(k, radius, algorithm, 
                                   metric, chunk_size);
  obj.fit(mat);
  reset_verbose_level();
  handle_trained_model<nearest_neighbors<T, MATRIX>>(mid, KNN , obj);
}

// knc - knn classifier
template <class T, class MATRIX>
void frovedis_knc(frovedis_mem_pair& mp, int& k, 
                  std::string& algorithm, 
                  std::string& metric, 
                  float& chunk_size,
                  int& verbose, 
                  int& mid) {
  MATRIX& mat = *reinterpret_cast<MATRIX*>(mp.first());
  dvector<T>& lbl = *reinterpret_cast<dvector<T>*>(mp.second());

  set_verbose_level(verbose);

  kneighbors_classifier<T, MATRIX> obj(k, algorithm, 
                                       metric, chunk_size);
  obj.fit(mat, lbl);
  reset_verbose_level();
  handle_trained_model<kneighbors_classifier<T, MATRIX>>(mid, KNC , obj);
}

// knr - knn regressor
template <class T, class MATRIX>
void frovedis_knr(frovedis_mem_pair& mp, int& k,
                  std::string& algorithm, 
                  std::string& metric, 
                  float& chunk_size,
                  int& verbose, 
                  int& mid) {
  MATRIX& mat = *reinterpret_cast<MATRIX*>(mp.first());
  dvector<T>& lbl = *reinterpret_cast<dvector<T>*>(mp.second()); 

  set_verbose_level(verbose);

  kneighbors_regressor<T, MATRIX> obj(k, algorithm,
                                      metric, chunk_size);
  obj.fit(mat, lbl);
  reset_verbose_level();
  handle_trained_model<kneighbors_regressor<T, MATRIX>>(mid, KNR , obj);
}

// Agglomerative Clustering 
// fit-predict
template <class T, class MATRIX>
std::vector<int> 
frovedis_aca(exrpc_ptr_t& data_ptr, int& mid, 
             std::string& linkage, 
             int& ncluster, T& threshold,
             int& verbose, bool& isMovableInput=false) {
  MATRIX& mat = *reinterpret_cast<MATRIX*>(data_ptr);  // training input data holder
  set_verbose_level(verbose);  
  auto agg_est = agglomerative_clustering<T>(ncluster).set_linkage(linkage)
                                                      .set_threshold(threshold);
  auto label = agg_est.fit_predict(mat);                                              
  // if input is movable, destroying Frovedis side data after training is done.
  if (isMovableInput)  mat.clear();
  reset_verbose_level();
  handle_trained_model<agglomerative_clustering<T>>(mid, ACM, agg_est);
  return label;
}

// DBSCAN fit-predict
template <class T, class MATRIX>
std::vector<int>
frovedis_dbscan(exrpc_ptr_t& data_ptr,
                std::vector<T>& sample_weight,
                double& eps, int& min_pts,
                int& verbose, int& mid) {
  MATRIX& mat = *reinterpret_cast<MATRIX*>(data_ptr);  // training input data holder
  set_verbose_level(verbose);
  auto dbm = dbscan<T>(eps, min_pts);
  if(sample_weight.size())
    dbm.fit(mat, sample_weight);
  else
    dbm.fit(mat);
  auto label = dbm.labels();
  reset_verbose_level();
  handle_trained_model<dbscan<T>>(mid,DBSCAN,dbm);
  return label;
}

template <class T, class MATRIX>
std::vector<int> 
frovedis_sca(exrpc_ptr_t& data_ptr, int& ncluster, 
             int& iteration, int& component, double& eps,
             double& gamma, bool& nlaplacian, 
             int& mid, int& verbose, 
             bool& pre, int& mode, bool& drop_first, 
             bool& isMovableInput=false) {
  MATRIX& mat = *reinterpret_cast<MATRIX*>(data_ptr);  // training input data holder
  auto old_level = frovedis::get_loglevel();
  if (verbose == 1) frovedis::set_loglevel(frovedis::DEBUG);
  else if (verbose == 2) frovedis::set_loglevel(frovedis::TRACE);
  auto model = frovedis::spectral_clustering_impl(mat,ncluster,component,
                                             iteration,eps,nlaplacian,
                                             pre,drop_first,gamma,mode,isMovableInput);
  frovedis::set_loglevel(old_level);
  auto m = model.labels;
  handle_trained_model<spectral_clustering_model<T>>(mid,SCM,model);
  return m;
}

template <class T, class MATRIX>
void frovedis_sea(exrpc_ptr_t& data_ptr, int& component,
                  double& gamma, bool& nlaplacian, 
                  int& mid, int& verbose, 
                  bool& pre, int& mode, bool& drop_first,
                  bool& isMovableInput=false) {
  MATRIX& mat = *reinterpret_cast<MATRIX*>(data_ptr);  // training input data holder
  auto old_level = frovedis::get_loglevel();
  if (verbose == 1) frovedis::set_loglevel(frovedis::DEBUG);
  else if (verbose == 2) frovedis::set_loglevel(frovedis::TRACE);
  auto m = frovedis::spectral_embedding(mat,component,nlaplacian,pre,drop_first,gamma,mode);
  // if input is movable, destroying Frovedis side data after training is done.
  if (isMovableInput)  mat.clear();
  frovedis::set_loglevel(old_level);
  handle_trained_model<spectral_embedding_model<T>>(mid,SEM,m);
}

template <class T, class MATRIX>
int frovedis_gmm(exrpc_ptr_t& data_ptr, int& mid, 
                 int& k, std::string& cov_type,
                 T& tol, int& max_iter, int& n_init,
                 std::string& init_params, long& seed,
                 int& verbose, bool& isMovableInput=false) {
  MATRIX& mat = *reinterpret_cast<MATRIX*>(data_ptr);  // training input data holder
  set_verbose_level(verbose);
  if (verbose == 1) frovedis::set_loglevel(frovedis::DEBUG);
  else if (verbose == 2) frovedis::set_loglevel(frovedis::TRACE);
  auto gmm_model = gaussian_mixture<T>().set_n_components(k)
                                        .set_covariance_type(cov_type)
                                        .set_tol(tol)
                                        .set_max_iter(max_iter)
                                        .set_init_params(init_params)
                                        .set_random_state(seed);
    
  gmm_model.fit(mat);
  int niter = gmm_model.n_iter_();
  reset_verbose_level();
  handle_trained_model<gaussian_mixture<T>>(mid, GMM, gmm_model);    
  return niter;
}

template <class T, class MATRIX>
void frovedis_dt(frovedis_mem_pair& mp, tree::strategy<T>& str,
                 int& verbose, int& mid,
                 bool& isMovableInput=false) {
  // extracting input data
  MATRIX& mat = *reinterpret_cast<MATRIX*>(mp.first());
  dvector<T>& lbl = *reinterpret_cast<dvector<T>*>(mp.second());

  auto old_level = frovedis::get_loglevel();
  if (verbose == 1) frovedis::set_loglevel(frovedis::DEBUG);
  else if (verbose == 2) frovedis::set_loglevel(frovedis::TRACE);

  auto builder = make_decision_tree_builder<T>(std::move(str));
  auto model = builder.run(mat, lbl);

  frovedis::set_loglevel(old_level);
  handle_trained_model<decision_tree_model<T>>(mid, DTM, model);

  if (isMovableInput) {
    mat.clear(); 
    lbl.mapv_partitions(clear_lbl_data<T>);
  }
}

template <class T, class MATRIX>
void frovedis_fm(frovedis_mem_pair& mp, std::string& optimizer_name, 
                 fm::fm_config<T>& conf,
                 int& verbose, int& mid,
                 bool& isMovableInput=false) {
  // extracting input data
  MATRIX& mat = *reinterpret_cast<MATRIX*>(mp.first());
  dvector<T>& lbl = *reinterpret_cast<dvector<T>*>(mp.second()); 

  auto old_level = frovedis::get_loglevel();
  if (verbose == 1) frovedis::set_loglevel(frovedis::DEBUG);
  else if (verbose == 2) frovedis::set_loglevel(frovedis::TRACE);

  fm::FmOptimizer optimizer;
  if (optimizer_name == "SGD") optimizer = fm::FmOptimizer::SGD;
  else if (optimizer_name == "SGDA") optimizer = fm::FmOptimizer::SGDA;
  else if (optimizer_name == "ALS")  optimizer = fm::FmOptimizer::ALS;
  else if (optimizer_name == "MCMC") optimizer = fm::FmOptimizer::MCMC;
  else throw std::runtime_error("Specified optimizer is not supported!\n");
  
  auto model = fm::train(mat,lbl,optimizer,conf);

  frovedis::set_loglevel(old_level);
  handle_trained_model<fm::fm_model<T>>(mid, FMM, model);

  if (isMovableInput) {
    mat.clear();
    lbl.mapv_partitions(clear_lbl_data<T>);
  }
}

template <class T, class MATRIX>
void frovedis_nb(frovedis_mem_pair& mp, std::string& model_type,
                 double& lambda, bool& fit_prior,
                 std::vector<T>& class_prior,
                 std::vector<T>& sample_weight,
                 double& binarize,
                 int& verbose, int& mid,
                 bool& isMovableInput=false) {

  // extracting input data
  MATRIX& mat = *reinterpret_cast<MATRIX*>(mp.first());
  dvector<T>& lbl = *reinterpret_cast<dvector<T>*>(mp.second());

  set_verbose_level(verbose);
  naive_bayes_model<T> model;
  if (model_type == "multinomial") 
    model = multinomial_nb(mat,lbl,lambda,fit_prior,
                           class_prior,sample_weight);
  else if (model_type == "bernoulli") 
    model = bernoulli_nb(mat,lbl,lambda,binarize,fit_prior,
                         class_prior,sample_weight);
  else throw std::runtime_error("Unsupported naive bayes algorithm!\n");
  handle_trained_model<naive_bayes_model<T>>(mid, NBM, model);
  reset_verbose_level();
  if (isMovableInput) {
    mat.clear();
    lbl.mapv_partitions(clear_lbl_data<T>);
  }
}

template <class DATA>
int frovedis_fp_growth(exrpc_ptr_t& dptr, double& min_support,
                        int& tree_depth, int& compression_point,
                        int& mem_opt_level,
                        int& verbose, int& mid,
                        bool& isMovableInput=false) {
  auto dfptr = reinterpret_cast<DATA*>(dptr);
  DATA& db = *dfptr;
  
  set_verbose_level(verbose);
  auto model = grow_fp_tree(db, min_support, tree_depth, 
                            compression_point, mem_opt_level); 
  auto fis_count = model.get_count();
  reset_verbose_level();
  if (isMovableInput) delete dfptr;
  handle_trained_model<fp_growth_model>(mid, FPM, model);
  return fis_count;
}

template <class MODEL>
int frovedis_fpr(double& min_confidence,
                 int& mid , int& midr) {
  register_for_train(midr);  // mark model 'mid' as "under training"
  auto mptr = get_model_ptr<fp_growth_model>(mid);
  auto rule =  mptr->generate_rules(min_confidence);
  auto count = rule.get_count();
  handle_trained_model<association_rule>(midr, FPR, rule);
  return count;
}

template <class MODEL>
std::vector<dummy_dftable>
frovedis_fp_fis(int& mid) {
  auto mptr = get_model_ptr<MODEL>(mid);
  auto depth = mptr->get_depth();
  auto fis =  mptr->get_frequent_itemset();
  std::vector<dummy_dftable> ret(depth);
  for(size_t i = 0; i < depth; ++i) {
    auto fisp = new dftable(fis[i]);
    ret[i] = to_dummy_dftable(fisp);
  }
  return ret;
}

template <class MODEL>
std::vector<dummy_dftable>
frovedis_fp_rules(int& mid) {
  auto mptr = get_model_ptr<MODEL>(mid);
  auto depth = mptr->get_depth();
  auto rules =  mptr->rule;
  std::vector<dummy_dftable> ret(depth);
  for(size_t i = 0; i < depth; ++i) {
    auto rulep = new dftable(rules[i]);
    ret[i] = to_dummy_dftable(rulep);
  }
  return ret;
}

template<class T>
std::pair<std::vector<T> , long>
convert_0( std::tuple<T,long> t){
    std::vector<T> element;
    long count = std::get<1>(t);
    element.push_back(std::get<0>(t));
    return std::make_pair(element,count);
 }

template<class T>
std::pair<std::vector<T> , long>
convert_1( std::tuple<T,T,long> t){
    std::vector<T> element;
    long count = std::get<2>(t);
    element.push_back(std::get<0>(t));
    element.push_back(std::get<1>(t));
    return std::make_pair(element,count);
  }

template<class T>
std::pair<std::vector<T> , long>
convert_2( std::tuple<T,T,T,long> t){
    std::vector<T> element;
    long count = std::get<3>(t);
    element.push_back(std::get<0>(t));
    element.push_back(std::get<1>(t));
    element.push_back(std::get<2>(t));
    return std::make_pair(element,count);
 }

template<class T>
std::pair<std::vector<T> , long>
convert_3( std::tuple<T,T,T,T,long> t){
    std::vector<T> element;
    long count = std::get<4>(t);
    element.push_back(std::get<0>(t));
    element.push_back(std::get<1>(t));
    element.push_back(std::get<2>(t));
    element.push_back(std::get<3>(t));
 return std::make_pair(element,count);
 }

template<class T>
std::pair<std::vector<T> , long>
convert_4(std::tuple<T,T,T,T,T,long> t){
    std::vector<T> element;
    long count = std::get<5>(t);
    element.push_back(std::get<0>(t));
    element.push_back(std::get<1>(t));
    element.push_back(std::get<2>(t));
    element.push_back(std::get<3>(t));
    element.push_back(std::get<4>(t));
 return std::make_pair(element,count);
 }

template<class T>
std::pair<std::vector<T> , long>
convert_5(std::tuple<T,T,T,T,T,T,long> t){
    std::vector<T> element;
    long count = std::get<6>(t);
    element.push_back(std::get<0>(t));
    element.push_back(std::get<1>(t));
    element.push_back(std::get<2>(t));
    element.push_back(std::get<3>(t));
    element.push_back(std::get<4>(t));
    element.push_back(std::get<5>(t));
 return std::make_pair(element,count);
 }
/*
template<class T>
std::vector<std::pair<std::vector<T>, long>>
merge(std::vector<std::pair<std::vector<T>,long>>& ret ,
     std::vector<std::pair<std::vector<T>,long>>&  res){

   ret.insert(std::end(ret), std::begin(res), std::end(res));
   return ret;
 }

template<class T >
std::vector<std::pair<std::vector<T>, long>>
get_frovedis_fpmr(std::vector<dftable>& freq){

  dvector<std::pair<std::vector<T>, long>>  ret;
  for (size_t i=0; i<freq.size(); ++i) {
    if(i == 0) {
      auto dv0 = dftable_to_dvector<T, size_t>(freq[i]); // dvector<tuple>
      ret  = dv0.map(convert_0<T>); // dvector<pair>
      }
    else if (i == 1) {
      auto dv1 = dftable_to_dvector<T, T, size_t>(freq[i]); // dvector<tuple>
      auto dv21 = dv1.map(convert_1<T>); // dvector<pair>
      ret =  dv21.map_partitions(merge<T>,ret.viewas_node_local());
     }
    else if (i == 2) {
      auto dv2 = dftable_to_dvector<T, T, T, size_t>(freq[i]); // dvector<tuple>
      auto dv22 = dv2.map(convert_2<T>); // dvector<pair>
      ret =  dv22.map_partitions(merge<T>,ret.viewas_node_local());
        }
    else if (i == 3) {
      auto dv3 = dftable_to_dvector<T,T,T,T, size_t>(freq[i]); // dvector<tuple>
      auto dv33 = dv3.map(convert_3<T>); // dvector<pair>
      ret =  dv33.map_partitions(merge<T>,ret.viewas_node_local());
        }
    else {
        std::cout<<"Frovedis model to spark model support upto 4 frequent item set";
        break;
        }
      }
     auto dr = ret.gather();
   return dr;

}

*/
//updated code 

template<class T>
void merge(std::vector<std::pair<std::vector<T>,long>>& dv1 ,
           std::vector<std::pair<std::vector<T>,long>>& dv2){
  dv2.insert(std::end(dv2), std::begin(dv1), std::end(dv1));
}

template<class T>
std::vector<std::pair<std::vector<T>, long>>
get_frovedis_fpmr(std::vector<dftable>& freq){
  dvector<std::pair<std::vector<T>, long>>  ret;
  for( size_t i=0; i <freq.size();i++){
    if(i == 0) {
      auto dv0 = dftable_to_dvector<T, size_t>(freq[0]); // dvector<tuple>
      ret  = dv0.map(convert_0<T>); // dvector<pair>
    }
    else if (i == 1) {
      auto dv1 = dftable_to_dvector<T, T, size_t>(freq[1]); // dvector<tuple>
      auto dv21 = dv1.map(convert_1<T>); // dvector<pair>
      dv21.mapv_partitions(merge<T>,ret.viewas_node_local());
    }
    else if (i ==2) {
      auto dv2 = dftable_to_dvector<T, T, T, size_t>(freq[2]); // dvector<tuple>
      auto dv22 = dv2.map(convert_2<T>); // dvector<pair>
      dv22.mapv_partitions(merge<T>,ret.viewas_node_local());
    }
    else if (i== 3) {
      auto dv3 = dftable_to_dvector<T,T,T,T, size_t>(freq[3]); // dvector<tuple>
      auto dv33 = dv3.map(convert_3<T>); // dvector<pair>
      dv33.mapv_partitions(merge<T>,ret.viewas_node_local());
    }
    else if (i == 4) {
      auto dv4 = dftable_to_dvector<T,T,T,T,T, size_t>(freq[4]); // dvector<tuple>
      auto dv44 = dv4.map(convert_4<T>); // dvector<pair>
      dv44.mapv_partitions(merge<T>,ret.viewas_node_local());
    }
    else if (i == 5) {
      auto dv5 = dftable_to_dvector<T,T,T,T,T,T, size_t>(freq[5]); // dvector<tuple>
      auto dv55 = dv5.map(convert_5<T>); // dvector<pair>
      dv55.mapv_partitions(merge<T>,ret.viewas_node_local());
    }
    else REPORT_ERROR(USER_ERROR,
      "Frovedis model to spark model support upto 6 frequent item set!\n");
  } 
  auto dr = ret.gather();
  /*
  for(auto e: dr) {
    for(auto i: e.first) std::cout << i << " "; std::cout << std::endl;
    std::cout << "count: " << e.second << std::endl;
  }*/
  return dr;
}

template <class T>
std::vector<std::pair<std::vector<T>, long>>
 get_frovedis_fpm(int& mid){
  auto mptr = get_model_ptr<fp_growth_model>(mid);
  auto ret = get_frovedis_fpmr<T>(mptr->item);
  return ret;
}

// for spark client
template <class T>
void frovedis_w2v(exrpc_ptr_t& hash_dptr,
                  std::vector<int>& vocab_count,
                  w2v::train_config& config,
                  int& mid) {
  dvector<int>& hash = *reinterpret_cast<dvector<int>*>(hash_dptr);
  auto hash_local = hash.viewas_node_local();
  auto vector = hash_local.map(w2v::train_each, 
                        broadcast(vocab_count), 
                        broadcast(config)).get(0);
  auto vocab_size = vocab_count.size();
  auto hidden_size = vector.size() / vocab_size;
  rowmajor_matrix_local<T> model;
  model.val.swap(vector);
  model.set_local_num(vocab_size,hidden_size);
  handle_trained_model<rowmajor_matrix_local<T>>(mid, W2V, model);
}

template <class TC, class MATRIX>
dummy_lda_model 
frovedis_lda_train(exrpc_ptr_t& dptr, double& alpha,
                   double& beta, int& num_topics,
                   int& num_iter, std::string& algorithm,
                   int& num_explore_iter, int& num_eval_cycle,
                   int& verbose, int& mid) {

  auto old_level = frovedis::get_loglevel();
  if (verbose == 1) frovedis::set_loglevel(frovedis::DEBUG);
  else if (verbose == 2) frovedis::set_loglevel(frovedis::TRACE);

  MATRIX& mat = *reinterpret_cast<MATRIX*>(dptr);
  auto model = lda_train<TC>(mat,alpha,beta,num_topics,num_iter,algorithm,
                             num_explore_iter,num_eval_cycle);
  handle_trained_model<lda_model<TC>>(mid, LDA, model);
  frovedis::set_loglevel(old_level);
  return dummy_lda_model(mat.num_row, num_topics, 
                         model.word_topic_count.local_num_row);
}

template <class TC, class MATRIX>
dummy_lda_model 
frovedis_lda_train_for_spark(exrpc_ptr_t& dptr, 
                             std::vector<long>& orig_doc_id, 
                             double& alpha, double& beta, int& num_topics,
                             int& num_iter, std::string& algorithm,
                             int& num_explore_iter, int& num_eval_cycle,
                             int& verbose, int& mid) {

  auto old_level = frovedis::get_loglevel();
  if (verbose == 1) frovedis::set_loglevel(frovedis::DEBUG);
  else if (verbose == 2) frovedis::set_loglevel(frovedis::TRACE);

  MATRIX& mat = *reinterpret_cast<MATRIX*>(dptr);
  auto mod_mat = mat.template change_datatype<TC>();
  rowmajor_matrix<TC> doc_topic_count;
  auto model = lda_train<TC>(mod_mat,alpha,beta,num_topics,num_iter,algorithm,
                             num_explore_iter,num_eval_cycle,doc_topic_count);
  lda_model_wrapper<TC> wrapper(std::move(model),std::move(doc_topic_count),
                                std::move(orig_doc_id));
  handle_trained_model<lda_model_wrapper<TC>>(mid, LDASP, wrapper);
  frovedis::set_loglevel(old_level);
  return dummy_lda_model(mat.num_row, num_topics, 
                         model.word_topic_count.local_num_row);
}

template <class T, class MATRIX>
void frovedis_rf(frovedis_mem_pair& mp, tree::strategy<T>& strat,
                 int& verbose, int& mid,
                 bool& isMovableInput=false) {
  // extracting input data
  MATRIX& mat = *reinterpret_cast<MATRIX*>(mp.first());
  dvector<T>& lbl = *reinterpret_cast<dvector<T>*>(mp.second());

  auto old_level = frovedis::get_loglevel();
  if (verbose == 1) frovedis::set_loglevel(frovedis::DEBUG);
  else if (verbose == 2) frovedis::set_loglevel(frovedis::TRACE);

  //auto builder = make_random_forest_builder<T>(strat.move());
  random_forest_builder<T> builder(strat.move());
  auto model = builder.run(mat, lbl);

  frovedis::set_loglevel(old_level);
  handle_trained_model<random_forest_model<T>>(mid, RFM, model);

  if (isMovableInput) {
    mat.clear(); 
    lbl.mapv_partitions(clear_lbl_data<T>);
  }
}

//gbt
template <class T, class MATRIX>
void frovedis_gbt(frovedis_mem_pair& mp, tree::strategy<T>& strategy,
                 int& verbose, int& mid,
                 bool& isMovableInput=false) {
  // extracting input data
  MATRIX& mat = *reinterpret_cast<MATRIX*>(mp.first());
  dvector<T>& lbl = *reinterpret_cast<dvector<T>*>(mp.second());

  auto old_level = frovedis::get_loglevel();
  if (verbose == 1) frovedis::set_loglevel(frovedis::DEBUG);
  else if (verbose == 2) frovedis::set_loglevel(frovedis::TRACE);

  auto builder = make_gradient_boosted_trees_builder(std::move(strategy));
  auto model = builder.run(mat, lbl);

  frovedis::set_loglevel(old_level);
  handle_trained_model<gradient_boosted_trees_model<T>>(mid, GBT, model);

  if (isMovableInput) {
    mat.clear();
    lbl.mapv_partitions(clear_lbl_data<T>);
  }
}

#endif
