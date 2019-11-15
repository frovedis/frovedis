#include "exrpc_model.hpp"
#include "short_hand_dense_type.hpp"
#include "short_hand_sparse_type.hpp"
#include "short_hand_model_type.hpp"

using namespace frovedis;

void expose_frovedis_model_functions() {
  expose(cleanup_frovedis_server);
  // --- frovedis DecisionTreeModel ---
  expose(show_model<DTM1>);
  expose(show_model<DTM2>);    // for python
  expose(release_model<DTM1>);
  expose(release_model<DTM2>); // for python
  expose(save_model<DTM1>);
  expose(save_model<DTM2>);    // for python
  expose(load_model<DTM1>);    // void returning
  expose(load_model<DTM2>);    // for python (void returning)
  expose((bcast_model_to_workers<DT1,DTM1>));
  expose((single_dtm_predict<DT1,S_LMAT1>));
  expose((parallel_dtm_predict<DT1,S_LMAT1>));
  expose((parallel_dtm_predict_with_broadcast<DT1,R_MAT1,R_LMAT1>));   // for python
  expose((parallel_dtm_predict_with_broadcast<DT2,R_MAT2,R_LMAT2>));   // for python
  expose((parallel_dtm_predict_with_broadcast<DT1,S_MAT14,S_LMAT14>)); // for python
  expose((parallel_dtm_predict_with_broadcast<DT1,S_MAT15,S_LMAT15>)); // for python
  expose((parallel_dtm_predict_with_broadcast<DT2,S_MAT24,S_LMAT24>)); // for python
  expose((parallel_dtm_predict_with_broadcast<DT2,S_MAT25,S_LMAT25>)); // for python
  // --- frovedis NBModel ---
  expose(show_model<NBM1>);
  expose(show_model<NBM2>);    // for python
  expose(release_model<NBM1>);
  expose(release_model<NBM2>); // for python
  expose(save_model<NBM1>);
  expose(save_model<NBM2>);    // for python
  expose(load_nbm<DT1>);       // returns string
  expose(load_nbm<DT2>);       // for python (returns string)
  expose((bcast_model_to_workers<DT1,NBM1>));
  expose((single_nbm_predict<DT1,S_LMAT1>));
  expose((parallel_nbm_predict<DT1,S_LMAT1>));
  expose((parallel_nbm_predict_with_broadcast<DT1,R_MAT1,R_LMAT1>));   // for python
  expose((parallel_nbm_predict_with_broadcast<DT2,R_MAT2,R_LMAT2>));   // for python
  expose((parallel_nbm_predict_with_broadcast<DT1,S_MAT14,S_LMAT14>)); // for python
  expose((parallel_nbm_predict_with_broadcast<DT1,S_MAT15,S_LMAT15>)); // for python
  expose((parallel_nbm_predict_with_broadcast<DT2,S_MAT24,S_LMAT24>)); // for python
  expose((parallel_nbm_predict_with_broadcast<DT2,S_MAT25,S_LMAT25>)); // for python
  expose((get_pi_vector<DT1,NBM1>));
  expose((get_pi_vector<DT2,NBM2>));
  expose((get_theta_vector<DT1,NBM1>));
  expose((get_theta_vector<DT2,NBM2>));
  expose((get_cls_counts_vector<DT1,NBM1>));
  expose((get_cls_counts_vector<DT2,NBM2>));
  // --- frovedis FMModel ---
  //expose(show_model<FMM1>);   // not supported
  //expose(show_model<FMM2>);   // for python (not supported)
  expose(release_model<FMM1>);
  expose(release_model<FMM2>);  // for python
  expose(save_fmm<DT1>);
  expose(save_fmm<DT2>);        // for python
  //expose(load_model<FMM1>);   // not supported
  //expose(load_model<FMM2>);   // for python (not supported)
  expose(bcast_fmm_to_workers<DT1>);
  expose((single_fmm_predict<DT1,S_LMAT1>));
  expose((parallel_fmm_predict<DT1,S_LMAT1>));
  //expose((parallel_fmm_predict_with_broadcast<DT1,R_MAT1,R_LMAT1>)); // for python (not supported)
  //expose((parallel_fmm_predict_with_broadcast<DT2,R_MAT2,R_LMAT2>)); // for python (not supported)
  expose((parallel_fmm_predict_with_broadcast<DT1,S_MAT14,S_LMAT14>)); // for python
  expose((parallel_fmm_predict_with_broadcast<DT1,S_MAT15,S_LMAT15>)); // for python
  expose((parallel_fmm_predict_with_broadcast<DT2,S_MAT24,S_LMAT24>)); // for python
  expose((parallel_fmm_predict_with_broadcast<DT2,S_MAT25,S_LMAT25>)); // for python
  // --- frovedis FP GrowthModel ---
  // --- not template based ---
  expose(show_model<FPM1>);
  expose(release_model<FPM1>);
  expose(save_model<FPM1>);
  expose(load_model<FPM1>); // void returning
  expose(show_model<FPR1>);
  expose(release_model<FPR1>);
  expose(save_model<FPR1>);
  expose(load_model<FPR1>);
//  expose(get_fis<FPM1>);  
  // --- frovedis LogisticRegressionModel ---
  expose(show_model<LRM1>);
  expose(show_model<LRM2>);    // for python
  expose(release_model<LRM1>);
  expose(release_model<LRM2>); // for python
  expose(save_model<LRM1>);
  expose(save_model<LRM2>);    // for python
  expose(load_glm<LRM1>);
  expose(load_glm<LRM2>);      // for python
  expose((set_glm_threshold<DT1,LRM1>));
  expose((bcast_model_to_workers<DT1,LRM1>));
  expose((single_glm_predict<DT1,S_LMAT1,LRM1>));
  expose((parallel_glm_predict<DT1,S_LMAT1,LRM1>));
  expose((pgp2<DT1,R_MAT1,R_LMAT1,LRM1>));    // for python
  expose((pgp2<DT2,R_MAT2,R_LMAT2,LRM2>));    // for python
  expose((pgp2<DT1,S_MAT14,S_LMAT14,LRM1>));  // for python
  expose((pgp2<DT1,S_MAT15,S_LMAT15,LRM1>));  // for python
  expose((pgp2<DT2,S_MAT24,S_LMAT24,LRM2>));  // for python
  expose((pgp2<DT2,S_MAT25,S_LMAT25,LRM2>));  // for python
  expose((get_weight_vector<DT1,LRM1>));
  expose((get_weight_vector<DT2,LRM2>));
  expose((get_intercept_as_vector<DT1,LRM1>));
  expose((get_intercept_as_vector<DT2,LRM2>));
  // --- frovedis MultinomialLogisticRegressionModel ---
  expose(show_model<MLR1>);
  expose(show_model<MLR2>);
  expose(save_model<MLR1>);
  expose(save_model<MLR2>);
  expose(release_model<MLR1>);
  expose(release_model<MLR2>);
  expose(load_glm<MLR1>);
  expose(load_glm<MLR2>);
  expose((set_glm_threshold<DT1,MLR1>));
  expose((bcast_model_to_workers<DT1,MLR1>));
  expose((single_glm_predict<DT1,S_LMAT1,MLR1>));
  expose((parallel_glm_predict<DT1,S_LMAT1,MLR1>));
  expose((pgp2<DT1,R_MAT1,R_LMAT1,MLR1>));    // for python
  expose((pgp2<DT2,R_MAT2,R_LMAT2,MLR2>));    // for python
  expose((pgp2<DT1,S_MAT14,S_LMAT14,MLR1>));
  expose((pgp2<DT1,S_MAT15,S_LMAT15, MLR1>));
  expose((pgp2<DT2,S_MAT24,S_LMAT24,MLR2>));
  expose((pgp2<DT2,S_MAT24,S_LMAT24,SVM2>));
  expose((get_weight_as_vector<DT1,MLR1>));
  expose((get_weight_as_vector<DT2,MLR2>));
  expose((get_intercept_vector<DT1,MLR1>));
  expose((get_intercept_vector<DT2,MLR2>));
  // --- frovedis LinearRegressionModel ---
  expose(show_model<LNRM1>);
  expose(release_model<LNRM1>);
  expose(save_model<LNRM1>);
  expose(load_lnrm<DT1>);
  expose(show_model<LNRM2>);    // for python
  expose(release_model<LNRM2>); // for python
  expose(save_model<LNRM2>);    // for python
  expose(load_lnrm<DT2>);       // for python
  expose((bcast_model_to_workers<DT1,LNRM1>));
  expose((single_lnrm_predict<DT1,S_LMAT1>));
  expose((parallel_lnrm_predict<DT1,S_LMAT1>));
  expose((p_lnrm_p2<DT1,R_MAT1,R_LMAT1>));    // for python
  expose((p_lnrm_p2<DT2,R_MAT2,R_LMAT2>));    // for python
  expose((p_lnrm_p2<DT1,S_MAT14,S_LMAT14>));  // for python
  expose((p_lnrm_p2<DT1,S_MAT15,S_LMAT15>));  // for python
  expose((p_lnrm_p2<DT2,S_MAT24,S_LMAT24>));  // for python
  expose((p_lnrm_p2<DT2,S_MAT25,S_LMAT25>));  // for python
  expose((get_weight_vector<DT1,LNRM1>));
  expose((get_weight_vector<DT2,LNRM2>));
  expose((get_intercept_as_vector<DT1,LNRM1>));
  expose((get_intercept_as_vector<DT2,LNRM2>));
  // --- frovedis SVMModel ---
  expose(show_model<SVM1>);
  expose(release_model<SVM1>);
  expose(save_model<SVM1>);
  expose(load_glm<SVM1>);
  expose(show_model<SVM2>);    // for python
  expose(release_model<SVM2>); // for python
  expose(save_model<SVM2>);    // for python
  expose(load_glm<SVM2>);      // for python
  expose((set_glm_threshold<DT1,SVM1>));
  expose((bcast_model_to_workers<DT1,SVM1>));
  expose((single_glm_predict<DT1,S_LMAT1,SVM1>));
  expose((parallel_glm_predict<DT1,S_LMAT1,SVM1>));
  expose((pgp2<DT1,R_MAT1,R_LMAT1,SVM1>));   // for python
  expose((pgp2<DT2,R_MAT2,R_LMAT2,SVM2>));   // for python
  expose((pgp2<DT1,S_MAT14,S_LMAT14,SVM1>)); // for python
  expose((pgp2<DT1,S_MAT15,S_LMAT15,SVM1>)); // for python
  expose((pgp2<DT2,S_MAT24,S_LMAT24,SVM2>)); // for python
  expose((pgp2<DT2,S_MAT25,S_LMAT25,SVM2>)); // for python
  expose((get_weight_vector<DT1,SVM1>));
  expose((get_weight_vector<DT2,SVM2>));
  expose((get_intercept_as_vector<DT1,SVM1>));
  expose((get_intercept_as_vector<DT2,SVM2>));
  // --- frovedis MatrixFactorizationModel ---
  expose(show_model<MFM1>);
  expose(release_model<MFM1>);
  expose(save_model<MFM1>);
  expose(load_mfm<DT1>);
  expose(show_model<MFM2>);    // for python
  expose(release_model<MFM2>); // for python
  expose(save_model<MFM2>);    // for python
  expose(load_mfm<DT2>);       // for python
  expose((bcast_model_to_workers<DT1,MFM1>));
  expose(single_mfm_predict<DT1>);
  expose(parallel_mfm_predict<DT1>);
  expose(frovedis_als_predict<DT1>);
  expose(recommend_users<DT1>);
  expose(recommend_products<DT1>);
  expose(frovedis_als_predict<DT2>); // for python
  expose(recommend_users<DT2>);      // for python
  expose(recommend_products<DT2>);   // for python
  // --- frovedis kmeans model (rowmajor_matrix_local<T>) ---
  expose(show_model<KMM1>);
  expose(release_model<KMM1>);
  expose(save_model<KMM1>);
  expose(load_kmm<DT1>);
  expose(show_model<KMM2>);    // for python
  expose(release_model<KMM2>); // for python
  expose(save_model<KMM2>);    // for python
  expose(load_kmm<DT2>);       // for python
  expose((bcast_model_to_workers<DT1,KMM1>));
  expose((single_kmm_predict<S_LMAT1,KMM1>));
  expose((parallel_kmm_predict<S_LMAT1,KMM1>));
  expose((pkp2<R_MAT1,R_LMAT1,KMM1>));   
  expose((pkp2<R_MAT2,R_LMAT2,KMM2>));   // for python
  expose((pkp2<S_MAT14,S_LMAT14,KMM1>)); 
  expose((pkp2<S_MAT15,S_LMAT15,KMM1>)); // for python
  expose((pkp2<S_MAT24,S_LMAT24,KMM2>)); // for python
  expose((pkp2<S_MAT25,S_LMAT25,KMM2>)); // for python
  // ---frovedis spectral embedding
  expose(release_model<SEM1>);
  expose(save_model<SEM1>);
  expose(show_model<SEM1>);
  expose(load_model<SEM1>);
  expose(get_sem_affinity_matrix<DT1>);
  expose(get_sem_embedding_matrix<DT1>);
  expose(release_model<SEM2>);           // for python
  expose(save_model<SEM2>);              // for python
  expose(show_model<SEM2>);              // for python
  expose(load_model<SEM2>);              // for python
  expose(get_sem_affinity_matrix<DT2>);  // for python
  expose(get_sem_embedding_matrix<DT2>); // for python
  // ---frovedis spectral clustering
  expose(release_model<SCM1>);
  expose(save_model<SCM1>);
  expose(show_model<SCM1>);
  expose(load_scm<DT1>);
  expose(get_scm_affinity_matrix<DT1>);
  expose(release_model<SCM2>);           // for python
  expose(save_model<SCM2>);              // for python
  expose(show_model<SCM2>);              // for python
  expose(load_scm<DT2>);                 // for python
  expose(get_scm_affinity_matrix<DT2>);  // for python
  // ---frovedis agglomerative clustering
  expose(release_model<ACM1>);
  expose(save_model<ACM1>);
  expose(show_model<ACM1>);
  expose(load_acm<DT1>);
  expose(frovedis_acm_pred<DT1>);     
  expose(release_model<ACM2>);           // for python
  expose(save_model<ACM2>);              // for python
  expose(show_model<ACM2>);              // for python
  expose(load_acm<DT2>);                 // for python
  expose(frovedis_acm_pred<DT2>);        // for python
  // --- frovedis word2vector model
  expose(release_model<W2V2>);
  expose(get_w2v_weight_ptr<DT2>);
  expose(get_w2v_weight_vector<DT2>);
  expose(show_w2v_weight<DT2>);
  expose(save_w2v_model<DT2>);
  // ---frovedis dbscan
  expose(release_model<dbscan>);
  // knn -Nearest Neigbors ( NN )
  expose((frovedis_kneighbors<DT1,DT4,R_MAT1,KNN1>));   
  expose((frovedis_kneighbors<DT2,DT4,R_MAT2,KNN2>));   
  expose((frovedis_kneighbors<DT1,DT5,R_MAT1,KNN1>));   
  expose((frovedis_kneighbors<DT2,DT5,R_MAT2,KNN2>));   
  expose((frovedis_kneighbors_graph<DT4,R_MAT1,KNN1,S_MAT14,S_LMAT14>));
  expose((frovedis_kneighbors_graph<DT4,R_MAT2,KNN2,S_MAT24,S_LMAT24>));
  expose((frovedis_kneighbors_graph<DT5,R_MAT1,KNN1,S_MAT15,S_LMAT15>));
  expose((frovedis_kneighbors_graph<DT5,R_MAT2,KNN2,S_MAT25,S_LMAT25>));
  expose((frovedis_radius_neighbors<DT4,R_MAT1,KNN1,S_MAT14,S_LMAT14>));   
  expose((frovedis_radius_neighbors<DT4,R_MAT2,KNN2,S_MAT24,S_LMAT24>));   
  expose((frovedis_radius_neighbors<DT5,R_MAT1,KNN1,S_MAT15,S_LMAT15>));   
  expose((frovedis_radius_neighbors<DT5,R_MAT2,KNN2,S_MAT25,S_LMAT25>));   
  expose((frovedis_radius_neighbors_graph<DT4,R_MAT1,KNN1,S_MAT14,S_LMAT14>));
  expose((frovedis_radius_neighbors_graph<DT4,R_MAT2,KNN2,S_MAT24,S_LMAT24>));
  expose((frovedis_radius_neighbors_graph<DT5,R_MAT1,KNN1,S_MAT15,S_LMAT15>));
  expose((frovedis_radius_neighbors_graph<DT5,R_MAT2,KNN2,S_MAT25,S_LMAT25>));
  // knc
  expose((frovedis_kneighbors<DT1,DT4,R_MAT1,KNC1>));
  expose((frovedis_kneighbors<DT2,DT4,R_MAT2,KNC2>));
  expose((frovedis_kneighbors<DT1,DT5,R_MAT1,KNC1>));
  expose((frovedis_kneighbors<DT2,DT5,R_MAT2,KNC2>));
  // knc - graph
  expose((frovedis_kneighbors_graph<DT4,R_MAT1,KNC1,S_MAT14,S_LMAT14>));
  expose((frovedis_kneighbors_graph<DT4,R_MAT2,KNC2,S_MAT24,S_LMAT24>));
  expose((frovedis_kneighbors_graph<DT5,R_MAT1,KNC1,S_MAT15,S_LMAT15>));
  expose((frovedis_kneighbors_graph<DT5,R_MAT2,KNC2,S_MAT25,S_LMAT25>));
  // knc - predict
  expose((frovedis_knc_predict<DT1,DT4,R_MAT1,KNC1>));
  expose((frovedis_knc_predict<DT2,DT4,R_MAT2,KNC2>));
  expose((frovedis_knc_predict<DT1,DT5,R_MAT1,KNC1>));
  expose((frovedis_knc_predict<DT2,DT5,R_MAT2,KNC2>));
  //knc - predict_proba
  expose((frovedis_knc_predict_proba<DT4,R_MAT1,KNC1,R_MAT1,R_LMAT1>));
  expose((frovedis_knc_predict_proba<DT4,R_MAT2,KNC2,R_MAT2,R_LMAT2>));
  expose((frovedis_knc_predict_proba<DT5,R_MAT1,KNC1,R_MAT1,R_LMAT1>));
  expose((frovedis_knc_predict_proba<DT5,R_MAT2,KNC2,R_MAT2,R_LMAT2>));
  // knc -score
  expose((frovedis_model_score<DT1,DT4,R_MAT1,KNC1>));
  expose((frovedis_model_score<DT2,DT4,R_MAT2,KNC2>));
  expose((frovedis_model_score<DT1,DT5,R_MAT1,KNC1>));
  expose((frovedis_model_score<DT2,DT5,R_MAT2,KNC2>));
  //knr
  expose((frovedis_kneighbors<DT1,DT4,R_MAT1,KNR1>));
  expose((frovedis_kneighbors<DT2,DT4,R_MAT2,KNR2>));
  expose((frovedis_kneighbors<DT1,DT5,R_MAT1,KNR1>));
  expose((frovedis_kneighbors<DT2,DT5,R_MAT2,KNR2>));
  // knr - graph
  expose((frovedis_kneighbors_graph<DT4,R_MAT1,KNR1,S_MAT14,S_LMAT14>));
  expose((frovedis_kneighbors_graph<DT4,R_MAT2,KNR2,S_MAT24,S_LMAT24>));
  expose((frovedis_kneighbors_graph<DT5,R_MAT1,KNR1,S_MAT15,S_LMAT15>));
  expose((frovedis_kneighbors_graph<DT5,R_MAT2,KNR2,S_MAT25,S_LMAT25>));
  // knr - predict
  expose((frovedis_knr_predict<DT1,DT4,R_MAT1,KNR1>));
  expose((frovedis_knr_predict<DT2,DT4,R_MAT2,KNR2>));
  expose((frovedis_knr_predict<DT1,DT5,R_MAT1,KNR1>));
  expose((frovedis_knr_predict<DT2,DT5,R_MAT2,KNR2>));
  // knr -score
  expose((frovedis_model_score<DT1,DT4,R_MAT1,KNR1>));
  expose((frovedis_model_score<DT2,DT4,R_MAT2,KNR2>));
  expose((frovedis_model_score<DT1,DT5,R_MAT1,KNR1>));
  expose((frovedis_model_score<DT2,DT5,R_MAT2,KNR2>));
  // release knn algo models
  expose(release_model<KNN1>);
  expose(release_model<KNR1>);
  expose(release_model<KNC1>);
  expose(release_model<KNN2>);
  expose(release_model<KNR2>);
  expose(release_model<KNC2>);
  // --- frovedis Latent Dirichlet Allocation
  expose((frovedis_lda_transform<DT4,S_MAT45,LDA4>));
  expose((frovedis_lda_transform<DT3,S_MAT35,LDA3>));
  /* currently frovedis lda does not accept I for input csr matrix */
  /*
  expose((frovedis_lda_transform<DT4,S_MAT44,LDA4>));
  expose((frovedis_lda_transform<DT3,S_MAT34,LDA3>));
  */
  expose(get_lda_component<LDA3>);
  expose(get_lda_component<LDA4>);
  expose(save_model<LDA4>);
  expose(save_model<LDA3>);
  expose(load_model<LDA4>);
  expose(load_model<LDA3>);
  expose(release_model<LDA4>);
  expose(release_model<LDA3>);
}
