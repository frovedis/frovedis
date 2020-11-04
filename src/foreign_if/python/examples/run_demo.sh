export PYTHONPATH=../main/python:$PYTHONPATH
export LD_LIBRARY_PATH=../lib:$LD_LIBRARY_PATH

COMMAND="mpirun -np 1 ../../server/frovedis_server"
#COMMAND="mpirun -np 1 $FROVEDIS_SERVER" 

if [ ! -d out ]; then
       mkdir out
fi
rm -rf out/*

# --- Matrix Creation Demo ---
python crs_matrix_demo.py "$COMMAND"
python dense_matrix_demo.py "$COMMAND"
python df_demo.py "$COMMAND"
python linalg_demo.py "$COMMAND"

# --- ML Data Creation Demo ---
python frovedis_labeled_point_demo.py "$COMMAND"

# --- Wrapper Demo ---
python pblas_demo.py "$COMMAND"
python scalapack_demo.py "$COMMAND"
python truncated_svd_demo.py "$COMMAND"
python pca_demo.py "$COMMAND"

# --- ML Demo ---
python lr_demo.py "$COMMAND"
python lnr_demo.py "$COMMAND"
python mlr_demo.py "$COMMAND"
python sgd_classifier_demo.py "$COMMAND"
python sgd_regressor_demo.py "$COMMAND"

python svm_demo.py "$COMMAND"
python svr_demo.py "$COMMAND"
python svc_demo.py "$COMMAND"
python dt_demo.py "$COMMAND"
python rf_demo.py "$COMMAND"
python gbt_demo.py "$COMMAND"
python nbm_demo.py "$COMMAND"
python fm_demo.py "$COMMAND"

python kmeans_demo.py "$COMMAND"
python dbscan_demo.py "$COMMAND"
python supervised_knn_demo.py "$COMMAND"
python unsupervised_knn_demo.py "$COMMAND"
python agglomerative_clustering_demo.py "$COMMAND"
python spectral_clustering_demo.py "$COMMAND"
python spectral_embedding_demo.py "$COMMAND"

python als_demo.py "$COMMAND"
python fp_growth_demo.py "$COMMAND"
python word2vec_demo.py "$COMMAND"
python lda_demo.py "$COMMAND"

# --- GridSearchCV with Logistic Regression Demo ---
python LRGridSearchCV_demo.py "$COMMAND"

# --- Graph Demo ---
python graph_pagerank.py "$COMMAND"
python graph_sssp.py "$COMMAND"
python graph_bfs.py "$COMMAND"
python graph_cc.py "$COMMAND"
