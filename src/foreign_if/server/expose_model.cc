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
  expose((pkp2<R_MAT1,R_LMAT1,KMM1>));   // for python
  expose((pkp2<R_MAT2,R_LMAT2,KMM2>));   // for python
  expose((pkp2<S_MAT14,S_LMAT14,KMM1>)); // for python
  expose((pkp2<S_MAT15,S_LMAT15,KMM1>)); // for python
  expose((pkp2<S_MAT24,S_LMAT24,KMM2>)); // for python
  expose((pkp2<S_MAT25,S_LMAT25,KMM2>)); // for python
}
