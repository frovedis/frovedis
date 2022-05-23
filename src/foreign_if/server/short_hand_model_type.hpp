#ifndef _SHORT_HAND_MODEL_TYPE_
#define _SHORT_HAND_MODEL_TYPE_ 

#include <frovedis/ml/neighbors/knn_unsupervised.hpp>
#include <frovedis/ml/neighbors/knn_supervised.hpp>
#include <frovedis/ml/clustering/gmm.hpp>
#include <frovedis/ml/tree/ensemble_model.hpp>
#include "frovedis/ml/kernel/kernel_svm.hpp"
#include "frovedis/ml/glm/logistic_regression.hpp"
#include "frovedis/ml/glm/ridge_regression.hpp"
#include "frovedis/ml/glm/lasso.hpp"
#include "frovedis/ml/glm/linear_svc.hpp"
#include "frovedis/ml/glm/linear_svr.hpp"
#include "frovedis/ml/glm/linear_regression.hpp"
#include "frovedis/ml/tsa/arima.hpp"
#include "short_hand_dtype.hpp"

using namespace frovedis;

// --- All Frovedis ML MODEL Types ---
typedef logistic_regression<DT1> LR1;
typedef logistic_regression<DT2> LR2;
typedef ridge_regression<DT1> RR1;
typedef ridge_regression<DT2> RR2;
typedef lasso_regression<DT1> LSR1;
typedef lasso_regression<DT2> LSR2;
typedef linear_regression<DT1> LNRM1;
typedef linear_regression<DT2> LNRM2;
typedef linear_svm_classifier<DT1> SVM1;
typedef linear_svm_classifier<DT2> SVM2; 
typedef kernel_csvc_model<DT1> KSVC1;
typedef kernel_csvc_model<DT2> KSVC2; 
typedef linear_svm_regressor<DT1> SVR1;
typedef linear_svm_regressor<DT2> SVR2; 
typedef matrix_factorization_model<DT1> MFM1;
typedef matrix_factorization_model<DT2> MFM2;
typedef KMeans<DT1> KMM1;
typedef KMeans<DT2> KMM2;
typedef decision_tree_model<DT1> DTM1;
typedef decision_tree_model<DT2> DTM2;
typedef fm::fm_model<DT1> FMM1;
typedef fm::fm_model<DT2> FMM2;
typedef naive_bayes_model<DT1> NBM1;
typedef naive_bayes_model<DT2> NBM2;
typedef fp_growth_model FPM1;
typedef association_rule FPR1;
typedef spectral_embedding_model<DT1> SEM1;
typedef spectral_embedding_model<DT2> SEM2;
typedef spectral_clustering_model<DT1> SCM1;
typedef spectral_clustering_model<DT2> SCM2;
typedef agglomerative_clustering<DT1> ACM1;
typedef agglomerative_clustering<DT1> ACM2;
typedef rowmajor_matrix_local<DT1> W2V1;
typedef rowmajor_matrix_local<DT2> W2V2;
typedef dbscan<DT1> DBSCAN1;
typedef dbscan<DT2> DBSCAN2;
typedef gaussian_mixture<DT1> GMM1;
typedef gaussian_mixture<DT2> GMM2;    
typedef nearest_neighbors<DT1, R_MAT1> KNNR1;
typedef nearest_neighbors<DT2, R_MAT2> KNNR2;
typedef nearest_neighbors<DT1, S_MAT14> KNNS14;
typedef nearest_neighbors<DT1, S_MAT15> KNNS15;
typedef nearest_neighbors<DT2, S_MAT24> KNNS24;
typedef nearest_neighbors<DT2, S_MAT25> KNNS25;
typedef kneighbors_classifier<DT1, R_MAT1> KNCR1;
typedef kneighbors_classifier<DT2, R_MAT2> KNCR2;
typedef kneighbors_classifier<DT1, S_MAT14> KNCS14;
typedef kneighbors_classifier<DT1, S_MAT15> KNCS15;
typedef kneighbors_classifier<DT2, S_MAT24> KNCS24;
typedef kneighbors_classifier<DT2, S_MAT25> KNCS25;
typedef kneighbors_regressor<DT1, R_MAT1> KNRR1;
typedef kneighbors_regressor<DT2, R_MAT2> KNRR2;
typedef kneighbors_regressor<DT1, S_MAT14> KNRS14;
typedef kneighbors_regressor<DT1, S_MAT15> KNRS15;
typedef kneighbors_regressor<DT2, S_MAT24> KNRS24;
typedef kneighbors_regressor<DT2, S_MAT25> KNRS25;
typedef lda_model<DT4> LDA4;
typedef lda_model<DT3> LDA3;
typedef lda_model_wrapper<DT3> LDASP3; //for spark
typedef random_forest_model<DT1> RFM1;
typedef random_forest_model<DT2> RFM2;
typedef gradient_boosted_trees_model<DT1> GBT1;
typedef gradient_boosted_trees_model<DT2> GBT2;
typedef standard_scaler<DT1> STANDARDSCALER1;
typedef standard_scaler<DT2> STANDARDSCALER2;
typedef Arima<DT1> ARM1;
typedef Arima<DT2> ARM2;
#endif
