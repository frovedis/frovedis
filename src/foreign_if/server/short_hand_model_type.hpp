#ifndef _SHORT_HAND_MODEL_TYPE_
#define _SHORT_HAND_MODEL_TYPE_ 

#include <frovedis/ml/neighbors/knn_unsupervised.hpp>
#include <frovedis/ml/neighbors/knn_supervised.hpp>
#include <frovedis/ml/tree/ensemble_model.hpp>
#include "frovedis/ml/kernel/kernel_svm.hpp"
#include "short_hand_dtype.hpp"

namespace frovedis {
  // --- All Frovedis ML MODEL Types ---
  typedef logistic_regression_model<DT1> LRM1;
  typedef logistic_regression_model<DT2> LRM2;
  typedef multinomial_logistic_regression_model<DT1> MLR1;
  typedef multinomial_logistic_regression_model<DT2> MLR2;
  typedef linear_regression_model<DT1> LNRM1;
  typedef linear_regression_model<DT2> LNRM2;
  typedef svm_model<DT1> SVM1;
  typedef svm_model<DT2> SVM2; 
  typedef kernel_csvc_model<DT1> KSVC1;
  typedef kernel_csvc_model<DT2> KSVC2; 
  typedef linear_regression_model<DT1> SVR1;
  typedef linear_regression_model<DT2> SVR2; 
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
  typedef nearest_neighbors<DT1> KNN1;
  typedef nearest_neighbors<DT2> KNN2;
  typedef kneighbors_classifier<DT1> KNC1;
  typedef kneighbors_classifier<DT2> KNC2;
  typedef kneighbors_regressor<DT1> KNR1;
  typedef kneighbors_regressor<DT2> KNR2;
  typedef lda_model<DT4> LDA4;
  typedef lda_model<DT3> LDA3;
  typedef lda_model_wrapper<DT3> LDASP3; //for spark
  typedef random_forest_model<DT1> RFM1;
  typedef random_forest_model<DT2> RFM2;
  typedef gradient_boosted_trees_model<DT1> GBT1;
  typedef gradient_boosted_trees_model<DT2> GBT2;
}
#endif
