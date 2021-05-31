#!/bin/sh

LIBRARY_PATH=../lib/
JARS=../lib/frovedis_client.jar
COMMAND="mpirun -np 1 ../../server/frovedis_server"
#COMMAND="mpirun -np 1 $FROVEDIS_SERVER" 
#LIBRARY_PATH=$X86_INSTALLPATH/lib
#JARS=$X86_INSTALLPATH/lib/spark/frovedis_client.jar

# by default, Spark runs as local mode
# if you use distributed mode, put the input files to HDFS
# hadoop fs -put input

if [ ! -d out ]; then
	mkdir out
fi

# --- matrix/data operations ---
echo -e "\n --- executing data transfer demo ---"
spark-submit --driver-java-options "-Djava.library.path=$LIBRARY_PATH" --jars $JARS --conf spark.driver.memory=8g lib/spark2frovedis_data_transfer.jar "$COMMAND"

echo -e "\n --- executing frovedis matrix demo ---"
spark-submit --driver-java-options "-Djava.library.path=$LIBRARY_PATH" --jars $JARS --conf spark.driver.memory=8g lib/spark2frovedis_mat.jar "$COMMAND"

echo -e "\n --- executing frovedis dataframe demo ---"
spark-submit --driver-java-options "-Djava.library.path=$LIBRARY_PATH" --jars $JARS --conf spark.driver.memory=8g lib/spark2frovedis_df.jar "$COMMAND"

echo -e "\n --- executing frovedis wrapper demo ---"
spark-submit --driver-java-options "-Djava.library.path=$LIBRARY_PATH" --jars $JARS --conf spark.driver.memory=8g lib/spark2frovedis_pblas_scalapack.jar "$COMMAND"

# --- PCA/SVD ---
echo -e "\n --- executing frovedis svd demo ---"
spark-submit --driver-java-options "-Djava.library.path=$LIBRARY_PATH" --jars $JARS --conf spark.driver.memory=8g lib/spark2frovedis_svd.jar "$COMMAND"

echo -e "\n --- executing frovedis pca demo ---"
spark-submit --driver-java-options "-Djava.library.path=$LIBRARY_PATH" --jars $JARS --conf spark.driver.memory=8g lib/spark2frovedis_pca.jar "$COMMAND"

# --- Logistic Regression ---
echo -e "\n --- executing frovedis logistic regression with sgd demo ---"
spark-submit --driver-java-options "-Djava.library.path=$LIBRARY_PATH" --jars $JARS --conf spark.driver.memory=8g lib/spark2frovedis_lrsgd.jar "$COMMAND"

echo -e "\n --- executing frovedis logistic regression model operations demo ---"
spark-submit --driver-java-options "-Djava.library.path=$LIBRARY_PATH" --jars $JARS --conf spark.driver.memory=8g lib/spark2frovedis_lrm.jar  "$COMMAND"

# --- Linear SVM Regression ---
echo -e "\n --- executing frovedis svm regression demo ---"
spark-submit --driver-java-options "-Djava.library.path=$LIBRARY_PATH" --jars $JARS --conf spark.driver.memory=8g lib/spark2frovedis_svr.jar "$COMMAND"

# --- SVM Kernel ---
echo -e "\n --- executing frovedis svm kernel demo ---"
spark-submit --driver-java-options "-Djava.library.path=$LIBRARY_PATH" --jars $JARS --conf spark.driver.memory=8g lib/spark2frovedis_svc.jar "$COMMAND"

# --- Naive Bayes ---
echo -e "\n --- executing frovedis naive bayes demo ---"
spark-submit --driver-java-options "-Djava.library.path=$LIBRARY_PATH" --jars $JARS --conf spark.driver.memory=8g lib/spark2frovedis_nb.jar "$COMMAND"

# --- Tree ---
echo -e "\n --- executing frovedis decision tree demo ---"
spark-submit --driver-java-options "-Djava.library.path=$LIBRARY_PATH" --jars $JARS --conf spark.driver.memory=8g lib/spark2frovedis_dt.jar "$COMMAND"

echo -e "\n --- executing GBT demo ---"
spark-submit --driver-java-options "-Djava.library.path=$LIBRARY_PATH" --jars $JARS --conf spark.driver.memory=8g lib/spark2frovedis_gbt.jar "$COMMAND"

echo -e "\n --- executing Random Forest demo ---"
spark-submit --driver-java-options "-Djava.library.path=$LIBRARY_PATH" --jars $JARS --conf spark.driver.memory=8g lib/spark2frovedis_rf.jar "$COMMAND"

echo -e "\n --- executing frovedis factorization machine demo ---"
spark-submit --driver-java-options "-Djava.library.path=$LIBRARY_PATH" --jars $JARS --conf spark.driver.memory=8g lib/spark2frovedis_fm.jar "$COMMAND"

# --- Recommendation using ALS ---
echo -e "\n --- executing frovedis recommendation demo ---"
spark-submit --driver-java-options "-Djava.library.path=$LIBRARY_PATH" --jars $JARS --conf spark.driver.memory=8g lib/spark2frovedis_als.jar "$COMMAND"

# --- Clustering ---
echo -e "\n --- executing frovedis kmeans demo ---"
spark-submit --driver-java-options "-Djava.library.path=$LIBRARY_PATH" --jars $JARS --conf spark.driver.memory=8g lib/spark2frovedis_kmeans.jar "$COMMAND"

echo -e "\n --- executing frovedis agglomerative clustering demo---"
spark-submit --driver-java-options "-Djava.library.path=$LIBRARY_PATH" --jars $JARS --conf spark.driver.memory=8g lib/spark2frovedis_ac.jar "$COMMAND"

echo -e "\n --- executing frovedis spectral clustering demo ---"
spark-submit --driver-java-options "-Djava.library.path=$LIBRARY_PATH" --jars $JARS --conf spark.driver.memory=8g lib/spark2frovedis_sc.jar "$COMMAND"

echo -e "\n --- executing frovedis spectral embedding demo ---"
spark-submit --driver-java-options "-Djava.library.path=$LIBRARY_PATH" --jars $JARS --conf spark.driver.memory=8g lib/spark2frovedis_se.jar "$COMMAND"

echo -e "\n --- executing frovedis dbscan demo ---"
spark-submit --driver-java-options "-Djava.library.path=$LIBRARY_PATH" --jars $JARS --conf spark.driver.memory=8g lib/frovedis_dbscan.jar "$COMMAND"

echo -e "\n --- executing frovedis gmm demo ---"
spark-submit --driver-java-options "-Djava.library.path=$LIBRARY_PATH" --jars $JARS --conf spark.driver.memory=8g lib/frovedis_gmm.jar "$COMMAND"

# --- Neighbors ---
echo -e "\n --- executing knn demo ---"
spark-submit --driver-java-options "-Djava.library.path=$LIBRARY_PATH" --jars $JARS --conf spark.driver.memory=8g lib/spark2frovedis_knn.jar "$COMMAND"

echo -e "\n --- executing knc demo ---"
spark-submit --driver-java-options "-Djava.library.path=$LIBRARY_PATH" --jars $JARS --conf spark.driver.memory=8g lib/spark2frovedis_knc.jar "$COMMAND"

echo -e "\n --- executing knr demo ---"
spark-submit --driver-java-options "-Djava.library.path=$LIBRARY_PATH" --jars $JARS --conf spark.driver.memory=8g lib/spark2frovedis_knr.jar "$COMMAND"

# --- Rule Mining ---
echo -e "\n --- executing fp growth demo ---"
spark-submit --driver-java-options "-Djava.library.path=$LIBRARY_PATH" --jars $JARS --conf spark.driver.memory=8g lib/spark2frovedis_fp.jar "$COMMAND"

# --- Word2Vector/LDA ---
echo -e "\n --- executing word2vector demo ---"
spark-submit --driver-java-options "-Djava.library.path=$LIBRARY_PATH" --jars $JARS --conf spark.driver.memory=8g lib/spark2frovedis_w2v.jar "$COMMAND"

echo -e "\n --- executing lda demo ---"
spark-submit --driver-java-options "-Djava.library.path=$LIBRARY_PATH" --jars $JARS --conf spark.driver.memory=8g lib/spark2frovedis_lda.jar "$COMMAND"

# --- Graph ---
echo -e "\n --- executing page rank demo ---"
spark-submit --driver-java-options "-Djava.library.path=$LIBRARY_PATH" --jars $JARS --conf spark.driver.memory=8g lib/frovedis_pagerank.jar "$COMMAND"

echo -e "\n --- executing SSSP demo ---"
spark-submit --driver-java-options "-Djava.library.path=$LIBRARY_PATH" --jars $JARS --conf spark.driver.memory=8g lib/frovedis_sssp.jar "$COMMAND"

echo -e "\n --- executing BFS demo ---"
spark-submit --driver-java-options "-Djava.library.path=$LIBRARY_PATH" --jars $JARS --conf spark.driver.memory=8g lib/frovedis_bfs.jar "$COMMAND"

echo -e "\n --- executing CC demo ---"
spark-submit --driver-java-options "-Djava.library.path=$LIBRARY_PATH" --jars $JARS --conf spark.driver.memory=8g lib/frovedis_cc.jar "$COMMAND"

echo -e "\n --- executing TSNE demo ---"
spark-submit --driver-java-options "-Djava.library.path=$LIBRARY_PATH" --jars $JARS --conf spark.driver.memory=8g lib/frovedis_tsne.jar "$COMMAND"
