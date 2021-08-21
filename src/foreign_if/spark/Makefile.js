include ../../../Makefile.in.x86

all:
	 ${JAVA_HOME}/bin/javac -cp "./bin:${SPARK_HOME}/jars/*:${HDPATH}" -sourcepath . -d bin ./main/com/nec/frovedis/Jmatrix/*.java
	 ${JAVA_HOME}/bin/javac -cp "./bin:${SPARK_HOME}/jars/*:${HDPATH}" -sourcepath . -d bin ./main/com/nec/frovedis/Jgraph/*.java
	 ${JAVA_HOME}/bin/javac -cp "./bin:${SPARK_HOME}/jars/*:${HDPATH}" -sourcepath . -d bin ./main/com/nec/frovedis/Jmllib/*.java
	 ${SCALA_HOME}/bin/scalac -cp "./bin:${SPARK_HOME}/jars/*:${HDPATH}" -sourcepath . -d bin ./main/com/nec/frovedis/graphx/bfs_result.scala ./main/com/nec/frovedis/graphx/sssp_result.scala ./main/com/nec/frovedis/graphx/cc_result.scala
	 ${JAVA_HOME}/bin/javac -cp "./bin:${SPARK_HOME}/jars/*:${HDPATH}" -sourcepath . -d bin ./main/com/nec/frovedis/Jexrpc/*.java
	 ${JAVA_HOME}/bin/javah -cp ./bin:${SPARK_HOME}/jars/* com.nec.frovedis.Jexrpc.JNISupport
	 mv com_nec_frovedis_Jexrpc_JNISupport.h ./main/cpp/JNISupport.hpp
	 ${SCALA_HOME}/bin/scalac -cp "./bin:${SPARK_HOME}/jars/*:${HDPATH}" -sourcepath . -d bin ./main/com/nec/frovedis/io/iohandler.scala
	 ${SCALA_HOME}/bin/scalac -cp "./bin:${SPARK_HOME}/jars/*:${HDPATH}" -sourcepath . -d bin ./main/com/nec/frovedis/matrix/repartition.scala
	 ${SCALA_HOME}/bin/scalac -cp "./bin:${SPARK_HOME}/jars/*:${HDPATH}" -sourcepath . -d bin ./main/com/nec/frovedis/matrix/ScalaCRS.scala
	 ${SCALA_HOME}/bin/scalac -cp "./bin:${SPARK_HOME}/jars/*:${HDPATH}" -sourcepath . -d bin ./main/com/nec/frovedis/matrix/MAT_KIND.scala
	 ${SCALA_HOME}/bin/scalac -cp "./bin:${SPARK_HOME}/jars/*:${HDPATH}" -sourcepath . -d bin ./main/com/nec/frovedis/matrix/TypedDvector.scala
	 ${SCALA_HOME}/bin/scalac -cp "./bin:${SPARK_HOME}/jars/*:${HDPATH}" -sourcepath . -d bin ./main/com/nec/frovedis/matrix/FrovedisDenseMatrix.scala
	 ${SCALA_HOME}/bin/scalac -cp "./bin:${SPARK_HOME}/jars/*:${HDPATH}" -sourcepath . -d bin ./main/com/nec/frovedis/exrpc/FrovedisSparseData.scala
	 ${SCALA_HOME}/bin/scalac -cp "./bin:${SPARK_HOME}/jars/*:${HDPATH}" -sourcepath . -d bin ./main/com/nec/frovedis/graphx/Graph.scala
	 ${SCALA_HOME}/bin/scalac -cp "./bin:${SPARK_HOME}/jars/*:${HDPATH}" -sourcepath . -d bin ./main/com/nec/frovedis/graphx/GraphLoader.scala
	 ${SCALA_HOME}/bin/scalac -cp "./bin:${SPARK_HOME}/jars/*:${HDPATH}" -sourcepath . -d bin ./main/com/nec/frovedis/exrpc/FrovedisLabeledPoint.scala
	 ${SCALA_HOME}/bin/scalac -cp "./bin:${SPARK_HOME}/jars/*:${HDPATH}" -sourcepath . -d bin ./main/com/nec/frovedis/matrix/blas_wrapper.scala
	 ${SCALA_HOME}/bin/scalac -cp "./bin:${SPARK_HOME}/jars/*:${HDPATH}" -sourcepath . -d bin ./main/com/nec/frovedis/matrix/pblas_wrapper.scala
	 ${SCALA_HOME}/bin/scalac -cp "./bin:${SPARK_HOME}/jars/*:${HDPATH}" -sourcepath . -d bin ./main/com/nec/frovedis/matrix/evd_result.scala
	 ${SCALA_HOME}/bin/scalac -cp "./bin:${SPARK_HOME}/jars/*:${HDPATH}" -sourcepath . -d bin ./main/com/nec/frovedis/matrix/scalapack_result.scala
	 ${SCALA_HOME}/bin/scalac -cp "./bin:${SPARK_HOME}/jars/*:${HDPATH}" -sourcepath . -d bin ./main/com/nec/frovedis/matrix/pca_result.scala
	 ${SCALA_HOME}/bin/scalac -cp "./bin:${SPARK_HOME}/jars/*:${HDPATH}" -sourcepath . -d bin ./main/com/nec/frovedis/matrix/scalapack_wrapper.scala
	 ${SCALA_HOME}/bin/scalac -cp "./bin:${SPARK_HOME}/jars/*:${HDPATH}" -sourcepath . -d bin ./main/com/nec/frovedis/matrix/lapack_wrapper.scala
	 ${SCALA_HOME}/bin/scalac -cp "./bin:${SPARK_HOME}/jars/*:${HDPATH}" -sourcepath . -d bin ./main/com/nec/frovedis/matrix/RowMatrixUtils.scala
	 ${SCALA_HOME}/bin/scalac -cp "./bin:${SPARK_HOME}/jars/*:${HDPATH}" -sourcepath . -d bin ./main/com/nec/frovedis/sql/Expr.scala
	 ${SCALA_HOME}/bin/scalac -cp "./bin:${SPARK_HOME}/jars/*:${HDPATH}" -sourcepath . -d bin ./main/com/nec/frovedis/sql/DFOperator.scala
	 ${SCALA_HOME}/bin/scalac -cp "./bin:${SPARK_HOME}/jars/*:${HDPATH}" -sourcepath . -d bin ./main/com/nec/frovedis/mllib/GenericModel.scala
	 ${SCALA_HOME}/bin/scalac -cp "./bin:${SPARK_HOME}/jars/*:${HDPATH}" -sourcepath . -d bin ./main/com/nec/frovedis/sql/DFtoSparseInfo.scala
	 ${SCALA_HOME}/bin/scalac -cp "./bin:${SPARK_HOME}/jars/*:${HDPATH}" -sourcepath . -d bin ./main/com/nec/frovedis/sql/FrovedisColumn.scala
	 ${SCALA_HOME}/bin/scalac -cp "./bin:${SPARK_HOME}/jars/*:${HDPATH}" -sourcepath . -d bin ./main/com/nec/frovedis/sql/FrovedisAggr.scala
	 ${SCALA_HOME}/bin/scalac -cp "./bin:${SPARK_HOME}/jars/*:${HDPATH}" -sourcepath . -d bin ./main/com/nec/frovedis/sql/FrovedisDataframe.scala ./main/com/nec/frovedis/sql/FrovedisGroupedDF.scala
	 ${SCALA_HOME}/bin/scalac -cp "./bin:${SPARK_HOME}/jars/*:${HDPATH}" -sourcepath . -d bin ./main/com/nec/frovedis/mllib/regression/GeneralizedLinearAlgorithm.scala
	 ${SCALA_HOME}/bin/scalac -cp "./bin:${SPARK_HOME}/jars/*:${HDPATH}" -sourcepath . -d bin ./main/com/nec/frovedis/mllib/regression/LinearRegression.scala
	 ${SCALA_HOME}/bin/scalac -cp "./bin:${SPARK_HOME}/jars/*:${HDPATH}" -sourcepath . -d bin ./main/com/nec/frovedis/mllib/regression/Lasso.scala
	 ${SCALA_HOME}/bin/scalac -cp "./bin:${SPARK_HOME}/jars/*:${HDPATH}" -sourcepath . -d bin ./main/com/nec/frovedis/mllib/regression/RidgeRegression.scala
	 ${SCALA_HOME}/bin/scalac -cp "./bin:${SPARK_HOME}/jars/*:${HDPATH}" -sourcepath . -d bin ./main/com/nec/frovedis/mllib/regression/SVMRegression.scala
	 ${SCALA_HOME}/bin/scalac -cp "./bin:${SPARK_HOME}/jars/*:${HDPATH}" -sourcepath . -d bin ./main/com/nec/frovedis/mllib/classification/LogisticRegression.scala
	 ${SCALA_HOME}/bin/scalac -cp "./bin:${SPARK_HOME}/jars/*:${HDPATH}" -sourcepath . -d bin ./main/com/nec/frovedis/mllib/regression/LinearSVR.scala
	 ${SCALA_HOME}/bin/scalac -cp "./bin:${SPARK_HOME}/jars/*:${HDPATH}" -sourcepath . -d bin ./main/com/nec/frovedis/mllib/classification/LinearSVC.scala
	 ${SCALA_HOME}/bin/scalac -cp "./bin:${SPARK_HOME}/jars/*:${HDPATH}" -sourcepath . -d bin ./main/com/nec/frovedis/mllib/classification/SVC.scala
	 ${SCALA_HOME}/bin/scalac -cp "./bin:${SPARK_HOME}/jars/*:${HDPATH}" -sourcepath . -d bin ./main/com/nec/frovedis/mllib/classification/NaiveBayes.scala
	 ${SCALA_HOME}/bin/scalac -cp "./bin:${SPARK_HOME}/jars/*:${HDPATH}" -sourcepath . -d bin ./main/com/nec/frovedis/mllib/recommendation/MatrixFactorizationModel.scala
	 ${SCALA_HOME}/bin/scalac -cp "./bin:${SPARK_HOME}/jars/*:${HDPATH}" -sourcepath . -d bin ./main/com/nec/frovedis/mllib/recommendation/ALS.scala
	 ${SCALA_HOME}/bin/scalac -cp "./bin:${SPARK_HOME}/jars/*:${HDPATH}" -sourcepath . -d bin ./main/com/nec/frovedis/mllib/clustering/KMeansModel.scala
	 ${SCALA_HOME}/bin/scalac -cp "./bin:${SPARK_HOME}/jars/*:${HDPATH}" -sourcepath . -d bin ./main/com/nec/frovedis/mllib/clustering/KMeans.scala
	 ${SCALA_HOME}/bin/scalac -cp "./bin:${SPARK_HOME}/jars/*:${HDPATH}" -sourcepath . -d bin ./main/com/nec/frovedis/mllib/clustering/SpectralClustering.scala
	 ${SCALA_HOME}/bin/scalac -cp "./bin:${SPARK_HOME}/jars/*:${HDPATH}" -sourcepath . -d bin ./main/com/nec/frovedis/mllib/clustering/SpectralEmbedding.scala
	 ${SCALA_HOME}/bin/scalac -cp "./bin:${SPARK_HOME}/jars/*:${HDPATH}" -sourcepath . -d bin ./main/com/nec/frovedis/mllib/clustering/AgglomerativeClustering.scala
	 ${SCALA_HOME}/bin/scalac -cp "./bin:${SPARK_HOME}/jars/*:${HDPATH}" -sourcepath . -d bin ./main/com/nec/frovedis/mllib/clustering/DBSCAN.scala
	 ${SCALA_HOME}/bin/scalac -cp "./bin:${SPARK_HOME}/jars/*:${HDPATH}" -sourcepath . -d bin ./main/com/nec/frovedis/mllib/clustering/LDA.scala
	 ${SCALA_HOME}/bin/scalac -cp "./bin:${SPARK_HOME}/jars/*:${HDPATH}" -sourcepath . -d bin ./main/com/nec/frovedis/mllib/clustering/GaussianMixtureModel.scala
	 ${SCALA_HOME}/bin/scalac -cp "./bin:${SPARK_HOME}/jars/*:${HDPATH}" -sourcepath . -d bin ./main/com/nec/frovedis/mllib/clustering/GaussianMixture.scala
	${SCALA_HOME}/bin/scalac -cp "./bin:${SPARK_HOME}/jars/*:${HDPATH}" -sourcepath . -d bin ./main/com/nec/frovedis/mllib/manifold/TSNE.scala
	 ${SCALA_HOME}/bin/scalac -cp "./bin:${SPARK_HOME}/jars/*:${HDPATH}" -sourcepath . -d bin ./main/com/nec/frovedis/mllib/fpm/FPGrowth.scala
	 ${SCALA_HOME}/bin/scalac -cp "./bin:${SPARK_HOME}/jars/*:${HDPATH}" -sourcepath . -d bin ./main/com/nec/frovedis/mllib/fm/FMConfig.scala
	 ${SCALA_HOME}/bin/scalac -cp "./bin:${SPARK_HOME}/jars/*:${HDPATH}" -sourcepath . -d bin ./main/com/nec/frovedis/mllib/fm/FactorizationMachine.scala
	 ${SCALA_HOME}/bin/scalac -cp "./bin:${SPARK_HOME}/jars/*:${HDPATH}" -sourcepath . -d bin ./main/com/nec/frovedis/mllib/tree/Impurities.scala
	 ${SCALA_HOME}/bin/scalac -cp "./bin:${SPARK_HOME}/jars/*:${HDPATH}" -sourcepath . -d bin ./main/com/nec/frovedis/mllib/tree/DecisionTreeModel.scala
	 ${SCALA_HOME}/bin/scalac -cp "./bin:${SPARK_HOME}/jars/*:${HDPATH}" -sourcepath . -d bin ./main/com/nec/frovedis/mllib/tree/DecisionTree.scala
	 ${SCALA_HOME}/bin/scalac -cp "./bin:${SPARK_HOME}/jars/*:${HDPATH}" -sourcepath . -d bin ./main/com/nec/frovedis/mllib/feature/Word2Vec.scala
	 ${SCALA_HOME}/bin/scalac -cp "./bin:${SPARK_HOME}/jars/*:${HDPATH}" -sourcepath . -d bin ./main/com/nec/frovedis/mllib/neighbors/NearestNeighbors.scala
	 ${SCALA_HOME}/bin/scalac -cp "./bin:${SPARK_HOME}/jars/*:${HDPATH}" -sourcepath . -d bin ./main/com/nec/frovedis/mllib/neighbors/KNeighborsClassifier.scala
	 ${SCALA_HOME}/bin/scalac -cp "./bin:${SPARK_HOME}/jars/*:${HDPATH}" -sourcepath . -d bin ./main/com/nec/frovedis/mllib/neighbors/KNeighborsRegressor.scala
	 ${SCALA_HOME}/bin/scalac -cp "./bin:${SPARK_HOME}/jars/*:${HDPATH}" -sourcepath . -d bin ./main/com/nec/frovedis/mllib/tree/RandomForestModel.scala
	 ${SCALA_HOME}/bin/scalac -cp "./bin:${SPARK_HOME}/jars/*:${HDPATH}" -sourcepath . -d bin ./main/com/nec/frovedis/mllib/tree/RandomForest.scala
	 ${SCALA_HOME}/bin/scalac -cp "./bin:${SPARK_HOME}/jars/*:${HDPATH}" -sourcepath . -d bin ./main/com/nec/frovedis/mllib/tree/Losses.scala
	 ${SCALA_HOME}/bin/scalac -cp "./bin:${SPARK_HOME}/jars/*:${HDPATH}" -sourcepath . -d bin ./main/com/nec/frovedis/mllib/tree/GradientBoostedTreesModel.scala
	 ${SCALA_HOME}/bin/scalac -cp "./bin:${SPARK_HOME}/jars/*:${HDPATH}" -sourcepath . -d bin ./main/com/nec/frovedis/mllib/tree/GradientBoostedTrees.scala
	 
	 cd bin; ${JAVA_HOME}/bin/jar -cf ../lib/frovedis_client.jar com 

clean:
	rm -fr bin/com lib/frovedis_client.jar main/cpp/JNISupport.hpp main/com/nec/frovedis/Jmatrix/*~ main/com/nec/frovedis/Jexrpc/*~ main/com/nec/frovedis/Jmllib/*~ main/com/nec/frovedis/matrix/*~ main/com/nec/frovedis/exrpc/*~ main/com/nec/frovedis/mllib/*~ main/com/nec/frovedis/mllib/regression/*~ main/com/nec/frovedis/mllib/classification/*~ main/com/nec/frovedis/mllib/recommendation/*~ main/com/nec/frovedis/mllib/clustering/*~ main/com/nec/frovedis/mllib/neighbors/*~ main/com/nec/frovedis/mllib/fpm/*~ main/com/nec/frovedis/mllib/fm/*~ main/com/nec/frovedis/mllib/tree/*~ main/com/nec/frovedis/mllib/feature/*~ main/com/nec/frovedis/sql/*~ main/com/nec/frovedis/mllib/manifold/*~
