#include "exrpc_ml.hpp"
#include "short_hand_dense_type.hpp"
#include "short_hand_sparse_type.hpp"
#include "short_hand_model_type.hpp"

using namespace frovedis;

void frovedis_w2v_train(std::string& encode,
                        std::string& weight,
                        std::string& count,
                        w2v::train_config& config) {
  w2v::train_each(encode, count, weight, config);
}

void expose_frovedis_ml_functions() {
  // --- frovedis ML trainers ---
  // (1) logistic regression
  expose((frovedis_lr_sgd<DT1,D_MAT1>));    // both spark and python case
  expose((frovedis_lr_sgd<DT2,D_MAT2>));    // python case
  expose((frovedis_lr_sgd<DT1,S_MAT1>));    // spark case
  expose((frovedis_lr_sgd<DT1,S_MAT14>));   // python case
  expose((frovedis_lr_sgd<DT1,S_MAT15>));   // python case
  expose((frovedis_lr_sgd<DT2,S_MAT24>));   // python case
  expose((frovedis_lr_sgd<DT2,S_MAT25>));   // python case
  expose((frovedis_lr_lbfgs<DT1,D_MAT1>));  // both spark and python case
  expose((frovedis_lr_lbfgs<DT2,D_MAT2>));  // python case
  expose((frovedis_lr_lbfgs<DT1,S_MAT1>));  // spark case
  expose((frovedis_lr_lbfgs<DT1,S_MAT14>)); // python case
  expose((frovedis_lr_lbfgs<DT1,S_MAT15>)); // python case
  expose((frovedis_lr_lbfgs<DT2,S_MAT24>)); // python case
  expose((frovedis_lr_lbfgs<DT2,S_MAT25>)); // python case
  // (2) linear svm
  expose((frovedis_svm_sgd<DT1,D_MAT1>));    // both spark and python case
  expose((frovedis_svm_sgd<DT2,D_MAT2>));    // python case
  expose((frovedis_svm_sgd<DT1,S_MAT1>));    // spark case
  expose((frovedis_svm_sgd<DT1,S_MAT14>));   // python case
  expose((frovedis_svm_sgd<DT1,S_MAT15>));   // python case
  expose((frovedis_svm_sgd<DT2,S_MAT24>));   // python case
  expose((frovedis_svm_sgd<DT2,S_MAT25>));   // python case
  expose((frovedis_svm_lbfgs<DT1,D_MAT1>));  // both spark and python case
  expose((frovedis_svm_lbfgs<DT2,D_MAT2>));  // python case
  expose((frovedis_svm_lbfgs<DT1,S_MAT1>));  // spark case
  expose((frovedis_svm_lbfgs<DT1,S_MAT14>)); // python case
  expose((frovedis_svm_lbfgs<DT1,S_MAT15>)); // python case
  expose((frovedis_svm_lbfgs<DT2,S_MAT24>)); // python case
  expose((frovedis_svm_lbfgs<DT2,S_MAT25>)); // python case
  // (3) decision tree
  //expose((frovedis_dt<DT1,S_MAT1>)); // not supported currently
  expose((frovedis_dt<DT1,D_MAT1>));   // both spark and python case
  expose((frovedis_dt<DT2,D_MAT2>));   // python case
  // (4) naive bayes
  expose((frovedis_nb<DT1,D_MAT1,D_LMAT1>));   // both spark and python case
  expose((frovedis_nb<DT2,D_MAT2,D_LMAT2>));   // python case
  expose((frovedis_nb<DT1,S_MAT1,S_LMAT1>));   // spark case
  expose((frovedis_nb<DT1,S_MAT14,S_LMAT14>)); // python case
  expose((frovedis_nb<DT1,S_MAT15,S_LMAT15>)); // python case
  expose((frovedis_nb<DT2,S_MAT24,S_LMAT24>)); // python case
  expose((frovedis_nb<DT2,S_MAT25,S_LMAT25>)); // python case
  // (5) linear regression
  expose((frovedis_lnr_lbfgs<DT1,D_MAT1>));    // both spark and python case
  expose((frovedis_lnr_lbfgs<DT2,D_MAT2>));    // python case
  expose((frovedis_lnr_lbfgs<DT1,S_MAT1>));    // spark case
  expose((frovedis_lnr_lbfgs<DT1,S_MAT14>));   // python case
  expose((frovedis_lnr_lbfgs<DT1,S_MAT15>));   // python case
  expose((frovedis_lnr_lbfgs<DT2,S_MAT24>));   // python case
  expose((frovedis_lnr_lbfgs<DT2,S_MAT25>));   // python case
  expose((frovedis_lnr_sgd<DT1,D_MAT1>));      // both spark and python case
  expose((frovedis_lnr_sgd<DT2,D_MAT2>));      // python case
  expose((frovedis_lnr_sgd<DT1,S_MAT1>));      // spark case
  expose((frovedis_lnr_sgd<DT1,S_MAT14>));     // python case
  expose((frovedis_lnr_sgd<DT1,S_MAT15>));     // python case
  expose((frovedis_lnr_sgd<DT2,S_MAT24>));     // python case
  expose((frovedis_lnr_sgd<DT2,S_MAT25>));     // python case
  // (6) lasso regression
  expose((frovedis_lasso_sgd<DT1,D_MAT1>));    // both spark and python case
  expose((frovedis_lasso_sgd<DT2,D_MAT2>));    // python case
  expose((frovedis_lasso_sgd<DT1,S_MAT1>));    // spark case
  expose((frovedis_lasso_sgd<DT1,S_MAT14>));   // python case
  expose((frovedis_lasso_sgd<DT1,S_MAT15>));   // python case
  expose((frovedis_lasso_sgd<DT2,S_MAT24>));   // python case
  expose((frovedis_lasso_sgd<DT2,S_MAT25>));   // python case
  expose((frovedis_lasso_lbfgs<DT1,D_MAT1>));  // both spark and python case
  expose((frovedis_lasso_lbfgs<DT2,D_MAT2>));  // python case
  expose((frovedis_lasso_lbfgs<DT1,S_MAT1>));  // spark case
  expose((frovedis_lasso_lbfgs<DT1,S_MAT14>)); // python case
  expose((frovedis_lasso_lbfgs<DT1,S_MAT15>)); // python case
  expose((frovedis_lasso_lbfgs<DT2,S_MAT24>)); // python case
  expose((frovedis_lasso_lbfgs<DT2,S_MAT25>)); // python case
  // (7) ridge regression
  expose((frovedis_ridge_sgd<DT1,D_MAT1>));    // both spark and python case
  expose((frovedis_ridge_sgd<DT2,D_MAT2>));    // python case
  expose((frovedis_ridge_sgd<DT1,S_MAT1>));    // spark case
  expose((frovedis_ridge_sgd<DT1,S_MAT14>));   // python case
  expose((frovedis_ridge_sgd<DT1,S_MAT15>));   // python case
  expose((frovedis_ridge_sgd<DT2,S_MAT24>));   // python case
  expose((frovedis_ridge_sgd<DT2,S_MAT25>));   // python case
  expose((frovedis_ridge_lbfgs<DT1,D_MAT1>));  // both spark and python case
  expose((frovedis_ridge_lbfgs<DT2,D_MAT2>));  // python case
  expose((frovedis_ridge_lbfgs<DT1,S_MAT1>));  // spark case
  expose((frovedis_ridge_lbfgs<DT1,S_MAT14>)); // python case
  expose((frovedis_ridge_lbfgs<DT1,S_MAT15>)); // python case
  expose((frovedis_ridge_lbfgs<DT2,S_MAT24>)); // python case
  expose((frovedis_ridge_lbfgs<DT2,S_MAT25>)); // python case
  // (8) kmeans clustering
  // as for dense input, kmeans supports only rowmajor_matrix<T> for dense data
  expose((frovedis_kmeans<DT1,R_MAT1>));  // both spark and python case
  expose((frovedis_kmeans<DT2,R_MAT2>));  // python case
  expose((frovedis_kmeans<DT1,S_MAT1>));  // spark case
  expose((frovedis_kmeans<DT1,S_MAT14>)); // python case
  expose((frovedis_kmeans<DT1,S_MAT15>)); // python case
  expose((frovedis_kmeans<DT2,S_MAT24>)); // python case
  expose((frovedis_kmeans<DT2,S_MAT25>)); // python case
  // (9) matrix factorization using als
  expose((frovedis_mf_als<DT1,S_MAT1>));  // spark case
  expose((frovedis_mf_als<DT1,S_MAT14>)); // python case
  expose((frovedis_mf_als<DT1,S_MAT15>)); // python case
  expose((frovedis_mf_als<DT2,S_MAT24>)); // python case
  expose((frovedis_mf_als<DT2,S_MAT25>)); // python case
  // (10) factorization machine
  expose((frovedis_fm<DT1,S_MAT1>));      // spark case
  expose((frovedis_fm<DT1,S_MAT14>));     // python case
  expose((frovedis_fm<DT1,S_MAT15>));     // python case
  expose((frovedis_fm<DT2,S_MAT24>));     // python case
  expose((frovedis_fm<DT2,S_MAT25>));     // python case
  // (11) frequent pattern mining using fp-growth
  expose(frovedis_fp_growth<dftable>);
  expose(frovedis_fpr<fp_growth_model>);
  //expose(get_frovedis_fpm<std::string>);
  expose(get_frovedis_fpm<int>);
  // (12) hierarchical clustering
  expose((frovedis_aca2<DT1,R_MAT1>)); // spark case (only fit)
  expose((frovedis_aca<DT1,R_MAT1>));  // python case (fit-predict)
  expose((frovedis_aca<DT2,R_MAT2>));  // python case (fit-predict)
  // (13) spectral clustering
  expose((frovedis_sca<DT1,R_MAT1>));
  expose((frovedis_sca<DT2,R_MAT2>)); // python case
  // (14) spectral embedding
  expose((frovedis_sea<DT1,R_MAT1>));
  expose((frovedis_sea<DT2,R_MAT2>)); // python case
  // (15) word2vector
  expose(frovedis_w2v<DT2>);          // spark case
  expose(frovedis_w2v_train);         // python case
  // (16) dbscan
  expose((frovedis_dbscan<DT1,R_MAT1>)); // python case (fit-predict)
  expose((frovedis_dbscan<DT2,R_MAT2>)); // python case (fit-predict)
  // (17) knn - Nearest Neighbors (NN)
  expose((frovedis_knn<DT1,R_MAT1>));
  expose((frovedis_knn<DT2,R_MAT2>)); 
  // (18) knc
  expose((frovedis_knc<DT1,R_MAT1>));
  expose((frovedis_knc<DT2,R_MAT2>)); 
  // (19) knr
  expose((frovedis_knr<DT1,R_MAT1>));
  expose((frovedis_knr<DT2,R_MAT2>)); 
  // (20) Latent Dirichlet Allocation
  expose((frovedis_lda_train<DT4,S_MAT45>));
  expose((frovedis_lda_train<DT3,S_MAT35>));
  /* currently frovedis lda does not accept I for input csr matrix */
  /*
  expose((frovedis_lda_train<DT4,S_MAT44>));
  expose((frovedis_lda_train<DT3,S_MAT34>));
  */
}
