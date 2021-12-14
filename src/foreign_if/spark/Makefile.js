include ../../../Makefile.in.x86

# CLASSES are just for dependency; not all the classes are listed

JCLASSES_NOSERVER := bin/com/nec/frovedis/Jmatrix/DummyEvdResult.class bin/com/nec/frovedis/Jmatrix/DummyFreqItemset.class bin/com/nec/frovedis/Jmatrix/DummyGesvdResult.class bin/com/nec/frovedis/Jmatrix/DummyGetrfResult.class bin/com/nec/frovedis/Jmatrix/DummyMatrix.class bin/com/nec/frovedis/Jmatrix/DummyPCAResult.class \
bin/com/nec/frovedis/Jmllib/DummyDftable.class bin/com/nec/frovedis/Jmllib/DummyGLM.class bin/com/nec/frovedis/Jmllib/DummyKNNResult.class bin/com/nec/frovedis/Jmllib/DummyLDAModel.class bin/com/nec/frovedis/Jmllib/DummyLDAResult.class bin/com/nec/frovedis/Jmllib/DummyTSNEResult.class bin/com/nec/frovedis/Jmllib/IntDoublePair.class \
bin/com/nec/frovedis/Jgraph/DummyEdge.class bin/com/nec/frovedis/Jgraph/DummyGraph.class

JCLASSES := bin/com/nec/frovedis/Jexrpc/FrovedisServer.class ${JCLASSES_NOSERVER}

SMATCLASSES_BASE := bin/com/nec/frovedis/matrix/GenericUtils.class bin/com/nec/frovedis/matrix/Utils.class bin/com/nec/frovedis/matrix/ScalaCRS.class bin/com/nec/frovedis/matrix/MAT_KIND.class bin/com/nec/frovedis/matrix/DTYPE.class bin/com/nec/frovedis/matrix/FrovedisDenseMatrix.class 
SMATCLASSES := ${SMATCLASSES_BASE} bin/com/nec/frovedis/matrix/BLAS.class bin/com/nec/frovedis/matrix/PBLAS.class bin/com/nec/frovedis/matrix/EvdResult.class bin/com/nec/frovedis/matrix/GetrfResult.class bin/com/nec/frovedis/matrix/FrovedisPCAModel.class bin/com/nec/frovedis/matrix/ScaLAPACK.class bin/com/nec/frovedis/matrix/LAPACK.class bin/com/nec/frovedis/matrix/RowMatrixUtils.class 

SCLASSES := ${SMATCLASSES} \
bin/com/nec/frovedis/exrpc/FrovedisSparseData.class bin/com/nec/frovedis/exrpc/FrovedisLabeledPoint.class \
bin/com/nec/frovedis/io/FrovedisIO.class \
bin/com/nec/frovedis/graphx/bfs_result.class bin/com/nec/frovedis/graphx/cc_result.class bin/com/nec/frovedis/graphx/sssp_result.class bin/com/nec/frovedis/graphx/Graph.class bin/com/nec/frovedis/graphx/GraphLoader.class \
bin/com/nec/frovedis/sql/DFtoSparseInfo.class bin/com/nec/frovedis/sql/FrovedisColumn.class bin/com/nec/frovedis/sql/FrovedisGroupedDF.class bin/com/nec/frovedis/Jsql/jPlatform.class bin/com/nec/frovedis/Jsql/jDFTransfer.class \
bin/com/nec/frovedis/mllib/GenericModel.class bin/com/nec/frovedis/mllib/regression/GeneralizedLinearModel.class bin/com/nec/frovedis/mllib/regression/LinearRegressionModel.class bin/com/nec/frovedis/mllib/regression/LassoWithSGD.class bin/com/nec/frovedis/mllib/regression/RidgeRegressionWithSGD.class bin/com/nec/frovedis/mllib/regression/SVMRegressionWithSGD.class bin/com/nec/frovedis/mllib/regression/LinearSVR.class bin/com/nec/frovedis/mllib/classification/LogisticRegression.class bin/com/nec/frovedis/mllib/classification/SVMModel.class bin/com/nec/frovedis/mllib/classification/SVC.class bin/com/nec/frovedis/mllib/classification/NaiveBayes.class bin/com/nec/frovedis/mllib/recommendation/MatrixFactorizationModel.class bin/com/nec/frovedis/mllib/recommendation/ALS.class bin/com/nec/frovedis/mllib/clustering/KMeansModel.class bin/com/nec/frovedis/mllib/clustering/KMeans.class bin/com/nec/frovedis/mllib/clustering/SpectralClustering.class bin/com/nec/frovedis/mllib/clustering/SpectralEmbedding.class bin/com/nec/frovedis/mllib/clustering/AgglomerativeClustering.class bin/com/nec/frovedis/mllib/clustering/DBSCAN.class bin/com/nec/frovedis/mllib/clustering/LDA.class bin/com/nec/frovedis/mllib/clustering/GaussianMixtureModel.class bin/com/nec/frovedis/mllib/clustering/GaussianMixture.class bin/com/nec/frovedis/mllib/manifold/TSNE.class bin/com/nec/frovedis/mllib/fpm/FPUtil.class bin/com/nec/frovedis/mllib/fpm/FPGrowth.class bin/com/nec/frovedis/mllib/fm/FactorizationMachine.class bin/com/nec/frovedis/mllib/tree/Impurities.class bin/com/nec/frovedis/mllib/tree/DecisionTreeModel.class bin/com/nec/frovedis/mllib/tree/DecisionTree.class bin/com/nec/frovedis/mllib/tree/RandomForestModel.class bin/com/nec/frovedis/mllib/tree/RandomForest.class bin/com/nec/frovedis/mllib/tree/Losses.class bin/com/nec/frovedis/mllib/tree/GradientBoostedTreesModel.class bin/com/nec/frovedis/mllib/tree/GradientBoostedTrees.class bin/com/nec/frovedis/mllib/feature/Word2Vec.class bin/com/nec/frovedis/mllib/feature/StandardScaler.class bin/com/nec/frovedis/mllib/neighbors/NearestNeighbors.class bin/com/nec/frovedis/mllib/neighbors/KNeighborsClassifier.class bin/com/nec/frovedis/mllib/neighbors/KNeighborsRegressor.class

CLASSES := ${JCLASSES} ${SCLASSES}

HEADER := main/cpp/JNISupport.hpp

all: ${CLASSES} ${HEADER}
	cd bin; ${JAVA_HOME}/bin/jar -cf ../lib/frovedis_client.jar com 

main/cpp/JNISupport.hpp: ${JCLASSES} bin/com/nec/frovedis/graphx/bfs_result.class bin/com/nec/frovedis/graphx/sssp_result.class
	${JAVA_HOME}/bin/javah -cp ./bin:${SPARK_HOME}/jars/* com.nec.frovedis.Jexrpc.JNISupport
	mv com_nec_frovedis_Jexrpc_JNISupport.h ./main/cpp/JNISupport.hpp

# use * to handle circular dependency
bin/com/nec/frovedis/Jexrpc/FrovedisServer.class: main/com/nec/frovedis/Jexrpc/FrovedisServer.java main/com/nec/frovedis/Jexrpc/JNISupport.java main/com/nec/frovedis/Jexrpc/MemPair.java main/com/nec/frovedis/Jexrpc/Node.java main/com/nec/frovedis/Jexrpc/SparseSVDResult.java ${JCLASSES_NOSERVER} bin/com/nec/frovedis/graphx/bfs_result.class bin/com/nec/frovedis/graphx/sssp_result.class
	${JAVA_HOME}/bin/javac -cp "./bin:${SPARK_HOME}/jars/*:${HDPATH}" -sourcepath . -d bin ./main/com/nec/frovedis/Jexrpc/*.java 
bin/com/nec/frovedis/Jmatrix/DummyEvdResult.class: main/com/nec/frovedis/Jmatrix/DummyEvdResult.java
	${JAVA_HOME}/bin/javac -cp "./bin:${SPARK_HOME}/jars/*:${HDPATH}" -sourcepath . -d bin ./main/com/nec/frovedis/Jmatrix/DummyEvdResult.java
bin/com/nec/frovedis/Jmatrix/DummyFreqItemset.class: main/com/nec/frovedis/Jmatrix/DummyFreqItemset.java
	${JAVA_HOME}/bin/javac -cp "./bin:${SPARK_HOME}/jars/*:${HDPATH}" -sourcepath . -d bin ./main/com/nec/frovedis/Jmatrix/DummyFreqItemset.java
bin/com/nec/frovedis/Jmatrix/DummyGesvdResult.class: main/com/nec/frovedis/Jmatrix/DummyGesvdResult.java
	${JAVA_HOME}/bin/javac -cp "./bin:${SPARK_HOME}/jars/*:${HDPATH}" -sourcepath . -d bin ./main/com/nec/frovedis/Jmatrix/DummyGesvdResult.java
bin/com/nec/frovedis/Jmatrix/DummyGetrfResult.class: main/com/nec/frovedis/Jmatrix/DummyGetrfResult.java
	${JAVA_HOME}/bin/javac -cp "./bin:${SPARK_HOME}/jars/*:${HDPATH}" -sourcepath . -d bin ./main/com/nec/frovedis/Jmatrix/DummyGetrfResult.java
bin/com/nec/frovedis/Jmatrix/DummyMatrix.class: main/com/nec/frovedis/Jmatrix/DummyMatrix.java
	${JAVA_HOME}/bin/javac -cp "./bin:${SPARK_HOME}/jars/*:${HDPATH}" -sourcepath . -d bin ./main/com/nec/frovedis/Jmatrix/DummyMatrix.java
bin/com/nec/frovedis/Jmatrix/DummyPCAResult.class: main/com/nec/frovedis/Jmatrix/DummyPCAResult.java
	${JAVA_HOME}/bin/javac -cp "./bin:${SPARK_HOME}/jars/*:${HDPATH}" -sourcepath . -d bin ./main/com/nec/frovedis/Jmatrix/DummyPCAResult.java
bin/com/nec/frovedis/Jmllib/DummyDftable.class: main/com/nec/frovedis/Jmllib/DummyDftable.java
	${JAVA_HOME}/bin/javac -cp "./bin:${SPARK_HOME}/jars/*:${HDPATH}" -sourcepath . -d bin ./main/com/nec/frovedis/Jmllib/DummyDftable.java
bin/com/nec/frovedis/Jmllib/DummyGLM.class: main/com/nec/frovedis/Jmllib/DummyGLM.java
	${JAVA_HOME}/bin/javac -cp "./bin:${SPARK_HOME}/jars/*:${HDPATH}" -sourcepath . -d bin ./main/com/nec/frovedis/Jmllib/DummyGLM.java
bin/com/nec/frovedis/Jmllib/DummyKNNResult.class: main/com/nec/frovedis/Jmllib/DummyKNNResult.java
	${JAVA_HOME}/bin/javac -cp "./bin:${SPARK_HOME}/jars/*:${HDPATH}" -sourcepath . -d bin ./main/com/nec/frovedis/Jmllib/DummyKNNResult.java
bin/com/nec/frovedis/Jmllib/DummyLDAModel.class: main/com/nec/frovedis/Jmllib/DummyLDAModel.java
	${JAVA_HOME}/bin/javac -cp "./bin:${SPARK_HOME}/jars/*:${HDPATH}" -sourcepath . -d bin ./main/com/nec/frovedis/Jmllib/DummyLDAModel.java
bin/com/nec/frovedis/Jmllib/DummyLDAResult.class: main/com/nec/frovedis/Jmllib/DummyLDAResult.java
	${JAVA_HOME}/bin/javac -cp "./bin:${SPARK_HOME}/jars/*:${HDPATH}" -sourcepath . -d bin ./main/com/nec/frovedis/Jmllib/DummyLDAResult.java
bin/com/nec/frovedis/Jmllib/DummyTSNEResult.class: main/com/nec/frovedis/Jmllib/DummyTSNEResult.java
	${JAVA_HOME}/bin/javac -cp "./bin:${SPARK_HOME}/jars/*:${HDPATH}" -sourcepath . -d bin ./main/com/nec/frovedis/Jmllib/DummyTSNEResult.java
bin/com/nec/frovedis/Jmllib/IntDoublePair.class: main/com/nec/frovedis/Jmllib/IntDoublePair.java
	${JAVA_HOME}/bin/javac -cp "./bin:${SPARK_HOME}/jars/*:${HDPATH}" -sourcepath . -d bin ./main/com/nec/frovedis/Jmllib/IntDoublePair.java
bin/com/nec/frovedis/Jgraph/DummyEdge.class: main/com/nec/frovedis/Jgraph/DummyEdge.java
	${JAVA_HOME}/bin/javac -cp "./bin:${SPARK_HOME}/jars/*:${HDPATH}" -sourcepath . -d bin ./main/com/nec/frovedis/Jgraph/DummyEdge.java
bin/com/nec/frovedis/Jgraph/DummyGraph.class: main/com/nec/frovedis/Jgraph/DummyGraph.java
	${JAVA_HOME}/bin/javac -cp "./bin:${SPARK_HOME}/jars/*:${HDPATH}" -sourcepath . -d bin ./main/com/nec/frovedis/Jgraph/DummyGraph.java

# scala creates multiple classes; only one class is specified as target
bin/com/nec/frovedis/exrpc/FrovedisSparseData.class: main/com/nec/frovedis/exrpc/FrovedisSparseData.scala ${JCLASSES} ${SMATCLASSES_BASE}
	${SCALA_HOME}/bin/scalac -cp "./bin:${SPARK_HOME}/jars/*:${HDPATH}" -sourcepath . -d bin ./main/com/nec/frovedis/exrpc/FrovedisSparseData.scala
bin/com/nec/frovedis/exrpc/FrovedisLabeledPoint.class: main/com/nec/frovedis/exrpc/FrovedisLabeledPoint.scala ${JCLASSES} ${SMATCLASSES_BASE} bin/com/nec/frovedis/exrpc/FrovedisSparseData.class
	${SCALA_HOME}/bin/scalac -cp "./bin:${SPARK_HOME}/jars/*:${HDPATH}" -sourcepath . -d bin ./main/com/nec/frovedis/exrpc/FrovedisLabeledPoint.scala

bin/com/nec/frovedis/io/FrovedisIO.class: main/com/nec/frovedis/io/iohandler.scala
	${SCALA_HOME}/bin/scalac -cp "./bin:${SPARK_HOME}/jars/*:${HDPATH}" -sourcepath . -d bin ./main/com/nec/frovedis/io/iohandler.scala

bin/com/nec/frovedis/matrix/GenericUtils.class: main/com/nec/frovedis/matrix/GenericUtils.scala
	${SCALA_HOME}/bin/scalac -cp "./bin:${SPARK_HOME}/jars/*:${HDPATH}" -sourcepath . -d bin ./main/com/nec/frovedis/matrix/GenericUtils.scala
bin/com/nec/frovedis/matrix/Utils.class: main/com/nec/frovedis/matrix/repartition.scala
	${SCALA_HOME}/bin/scalac -cp "./bin:${SPARK_HOME}/jars/*:${HDPATH}" -sourcepath . -d bin ./main/com/nec/frovedis/matrix/repartition.scala
bin/com/nec/frovedis/matrix/ScalaCRS.class: main/com/nec/frovedis/matrix/ScalaCRS.scala
	${SCALA_HOME}/bin/scalac -cp "./bin:${SPARK_HOME}/jars/*:${HDPATH}" -sourcepath . -d bin ./main/com/nec/frovedis/matrix/ScalaCRS.scala
bin/com/nec/frovedis/matrix/MAT_KIND.class: main/com/nec/frovedis/matrix/MAT_KIND.scala
	${SCALA_HOME}/bin/scalac -cp "./bin:${SPARK_HOME}/jars/*:${HDPATH}" -sourcepath . -d bin ./main/com/nec/frovedis/matrix/MAT_KIND.scala
bin/com/nec/frovedis/matrix/DTYPE.class: main/com/nec/frovedis/matrix/TypedDvector.scala ${JCLASSES} bin/com/nec/frovedis/matrix/Utils.class 
	${SCALA_HOME}/bin/scalac -cp "./bin:${SPARK_HOME}/jars/*:${HDPATH}" -sourcepath . -d bin ./main/com/nec/frovedis/matrix/TypedDvector.scala
bin/com/nec/frovedis/matrix/FrovedisDenseMatrix.class: main/com/nec/frovedis/matrix/FrovedisDenseMatrix.scala ${JCLASSES} bin/com/nec/frovedis/matrix/Utils.class bin/com/nec/frovedis/matrix/MAT_KIND.class
	${SCALA_HOME}/bin/scalac -cp "./bin:${SPARK_HOME}/jars/*:${HDPATH}" -sourcepath . -d bin ./main/com/nec/frovedis/matrix/FrovedisDenseMatrix.scala
bin/com/nec/frovedis/matrix/BLAS.class: main/com/nec/frovedis/matrix/blas_wrapper.scala ${JCLASSES} ${SMATCLASSES_BASE}
	${SCALA_HOME}/bin/scalac -cp "./bin:${SPARK_HOME}/jars/*:${HDPATH}" -sourcepath . -d bin ./main/com/nec/frovedis/matrix/blas_wrapper.scala
bin/com/nec/frovedis/matrix/PBLAS.class: main/com/nec/frovedis/matrix/pblas_wrapper.scala ${JCLASSES} ${SMATCLASSES_BASE}
	${SCALA_HOME}/bin/scalac -cp "./bin:${SPARK_HOME}/jars/*:${HDPATH}" -sourcepath . -d bin ./main/com/nec/frovedis/matrix/pblas_wrapper.scala
bin/com/nec/frovedis/matrix/EvdResult.class: main/com/nec/frovedis/matrix/evd_result.scala ${JCLASSES} ${SMATCLASSES_BASE}
	${SCALA_HOME}/bin/scalac -cp "./bin:${SPARK_HOME}/jars/*:${HDPATH}" -sourcepath . -d bin ./main/com/nec/frovedis/matrix/evd_result.scala
bin/com/nec/frovedis/matrix/GetrfResult.class: main/com/nec/frovedis/matrix/scalapack_result.scala ${JCLASSES} ${SMATCLASSES_BASE}
	${SCALA_HOME}/bin/scalac -cp "./bin:${SPARK_HOME}/jars/*:${HDPATH}" -sourcepath . -d bin ./main/com/nec/frovedis/matrix/scalapack_result.scala
bin/com/nec/frovedis/matrix/FrovedisPCAModel.class: main/com/nec/frovedis/matrix/pca_result.scala ${JCLASSES} ${SMATCLASSES_BASE}
	${SCALA_HOME}/bin/scalac -cp "./bin:${SPARK_HOME}/jars/*:${HDPATH}" -sourcepath . -d bin ./main/com/nec/frovedis/matrix/pca_result.scala
bin/com/nec/frovedis/matrix/ScaLAPACK.class: main/com/nec/frovedis/matrix/scalapack_wrapper.scala ${JCLASSES} ${SMATCLASSES_BASE} bin/com/nec/frovedis/matrix/GetrfResult.class
	${SCALA_HOME}/bin/scalac -cp "./bin:${SPARK_HOME}/jars/*:${HDPATH}" -sourcepath . -d bin ./main/com/nec/frovedis/matrix/scalapack_wrapper.scala
bin/com/nec/frovedis/matrix/LAPACK.class: main/com/nec/frovedis/matrix/lapack_wrapper.scala ${JCLASSES} ${SMATCLASSES_BASE} bin/com/nec/frovedis/matrix/GetrfResult.class
	${SCALA_HOME}/bin/scalac -cp "./bin:${SPARK_HOME}/jars/*:${HDPATH}" -sourcepath . -d bin ./main/com/nec/frovedis/matrix/lapack_wrapper.scala
bin/com/nec/frovedis/matrix/RowMatrixUtils.class: main/com/nec/frovedis/matrix/RowMatrixUtils.scala ${JCLASSES} ${SMATCLASSES_BASE} bin/com/nec/frovedis/matrix/EvdResult.class bin/com/nec/frovedis/matrix/GetrfResult.class bin/com/nec/frovedis/matrix/FrovedisPCAModel.class bin/com/nec/frovedis/exrpc/FrovedisSparseData.class
	${SCALA_HOME}/bin/scalac -cp "./bin:${SPARK_HOME}/jars/*:${HDPATH}" -sourcepath . -d bin ./main/com/nec/frovedis/matrix/RowMatrixUtils.scala

bin/com/nec/frovedis/graphx/bfs_result.class: main/com/nec/frovedis/graphx/bfs_result.scala
	${SCALA_HOME}/bin/scalac -cp "./bin:${SPARK_HOME}/jars/*:${HDPATH}" -sourcepath . -d bin ./main/com/nec/frovedis/graphx/bfs_result.scala
bin/com/nec/frovedis/graphx/cc_result.class: main/com/nec/frovedis/graphx/cc_result.scala
	${SCALA_HOME}/bin/scalac -cp "./bin:${SPARK_HOME}/jars/*:${HDPATH}" -sourcepath . -d bin ./main/com/nec/frovedis/graphx/cc_result.scala
bin/com/nec/frovedis/graphx/sssp_result.class: main/com/nec/frovedis/graphx/sssp_result.scala
	${SCALA_HOME}/bin/scalac -cp "./bin:${SPARK_HOME}/jars/*:${HDPATH}" -sourcepath . -d bin ./main/com/nec/frovedis/graphx/sssp_result.scala
bin/com/nec/frovedis/graphx/Graph.class: main/com/nec/frovedis/graphx/Graph.scala ${JCLASSES} bin/com/nec/frovedis/exrpc/FrovedisSparseData.class bin/com/nec/frovedis/graphx/cc_result.class
	${SCALA_HOME}/bin/scalac -cp "./bin:${SPARK_HOME}/jars/*:${HDPATH}" -sourcepath . -d bin ./main/com/nec/frovedis/graphx/Graph.scala
bin/com/nec/frovedis/graphx/GraphLoader.class: main/com/nec/frovedis/graphx/GraphLoader.scala bin/com/nec/frovedis/graphx/Graph.class
	${SCALA_HOME}/bin/scalac -cp "./bin:${SPARK_HOME}/jars/*:${HDPATH}" -sourcepath . -d bin ./main/com/nec/frovedis/graphx/GraphLoader.scala

bin/com/nec/frovedis/sql/FrovedisColumn.class: main/com/nec/frovedis/sql/FrovedisColumn.scala bin/com/nec/frovedis/matrix/DTYPE.class 
	${SCALA_HOME}/bin/scalac -cp "./bin:${SPARK_HOME}/jars/*:${HDPATH}" -sourcepath . -d bin ./main/com/nec/frovedis/sql/FrovedisColumn.scala
bin/com/nec/frovedis/sql/FrovedisGroupedDF.class: main/com/nec/frovedis/sql/FrovedisGroupedDF.scala bin/com/nec/frovedis/exrpc/FrovedisSparseData.class bin/com/nec/frovedis/sql/DFtoSparseInfo.class bin/com/nec/frovedis/Jsql/jDFTransfer.class bin/com/nec/frovedis/Jsql/jPlatform.class
	${SCALA_HOME}/bin/scalac -cp "./bin:${SPARK_HOME}/jars/*:${HDPATH}" -sourcepath . -d bin ./main/com/nec/frovedis/sql/FrovedisDataframe.scala ./main/com/nec/frovedis/sql/FrovedisGroupedDF.scala
bin/com/nec/frovedis/Jsql/jPlatform.class: ./main/com/nec/frovedis/Jsql/jPlatform.java 
	${JAVA_HOME}/bin/javac -cp "./bin:${SPARK_HOME}/jars/*:${HDPATH}" -sourcepath . -d bin ./main/com/nec/frovedis/Jsql/jPlatform.java
bin/com/nec/frovedis/Jsql/jDFTransfer.class: ./main/com/nec/frovedis/Jsql/jDFTransfer.java ./bin/com/nec/frovedis/Jsql/jPlatform.class ./bin/com/nec/frovedis/Jexrpc/FrovedisServer.class
	${JAVA_HOME}/bin/javac -cp "./bin:${SPARK_HOME}/jars/*:${HDPATH}" -sourcepath . -d bin ./main/com/nec/frovedis/Jsql/jDFTransfer.java
bin/com/nec/frovedis/sql/DFtoSparseInfo.class: main/com/nec/frovedis/sql/DFtoSparseInfo.scala ${JCLASSES} bin/com/nec/frovedis/mllib/GenericModel.class
	${SCALA_HOME}/bin/scalac -cp "./bin:${SPARK_HOME}/jars/*:${HDPATH}" -sourcepath . -d bin ./main/com/nec/frovedis/sql/DFtoSparseInfo.scala

bin/com/nec/frovedis/mllib/GenericModel.class: main/com/nec/frovedis/mllib/GenericModel.scala ${JCLASSES} ${SMATCLASSES} bin/com/nec/frovedis/exrpc/FrovedisSparseData.class
	${SCALA_HOME}/bin/scalac -cp "./bin:${SPARK_HOME}/jars/*:${HDPATH}" -sourcepath . -d bin ./main/com/nec/frovedis/mllib/GenericModel.scala
bin/com/nec/frovedis/mllib/regression/GeneralizedLinearModel.class: main/com/nec/frovedis/mllib/regression/GeneralizedLinearAlgorithm.scala bin/com/nec/frovedis/mllib/GenericModel.class
	${SCALA_HOME}/bin/scalac -cp "./bin:${SPARK_HOME}/jars/*:${HDPATH}" -sourcepath . -d bin ./main/com/nec/frovedis/mllib/regression/GeneralizedLinearAlgorithm.scala
bin/com/nec/frovedis/mllib/regression/LinearRegressionModel.class: main/com/nec/frovedis/mllib/regression/LinearRegression.scala bin/com/nec/frovedis/exrpc/FrovedisLabeledPoint.class bin/com/nec/frovedis/mllib/regression/GeneralizedLinearModel.class
	${SCALA_HOME}/bin/scalac -cp "./bin:${SPARK_HOME}/jars/*:${HDPATH}" -sourcepath . -d bin ./main/com/nec/frovedis/mllib/regression/LinearRegression.scala
bin/com/nec/frovedis/mllib/regression/LassoWithSGD.class: main/com/nec/frovedis/mllib/regression/Lasso.scala bin/com/nec/frovedis/mllib/regression/LinearRegressionModel.class
	${SCALA_HOME}/bin/scalac -cp "./bin:${SPARK_HOME}/jars/*:${HDPATH}" -sourcepath . -d bin ./main/com/nec/frovedis/mllib/regression/Lasso.scala
bin/com/nec/frovedis/mllib/regression/RidgeRegressionWithSGD.class: main/com/nec/frovedis/mllib/regression/RidgeRegression.scala bin/com/nec/frovedis/mllib/regression/LinearRegressionModel.class
	${SCALA_HOME}/bin/scalac -cp "./bin:${SPARK_HOME}/jars/*:${HDPATH}" -sourcepath . -d bin ./main/com/nec/frovedis/mllib/regression/RidgeRegression.scala
bin/com/nec/frovedis/mllib/regression/SVMRegressionWithSGD.class: main/com/nec/frovedis/mllib/regression/SVMRegression.scala bin/com/nec/frovedis/mllib/regression/LinearRegressionModel.class bin/com/nec/frovedis/io/FrovedisIO.class
	${SCALA_HOME}/bin/scalac -cp "./bin:${SPARK_HOME}/jars/*:${HDPATH}" -sourcepath . -d bin ./main/com/nec/frovedis/mllib/regression/SVMRegression.scala
bin/com/nec/frovedis/mllib/regression/LinearSVR.class: main/com/nec/frovedis/mllib/regression/LinearSVR.scala bin/com/nec/frovedis/exrpc/FrovedisLabeledPoint.class bin/com/nec/frovedis/mllib/regression/GeneralizedLinearModel.class bin/com/nec/frovedis/io/FrovedisIO.class
	${SCALA_HOME}/bin/scalac -cp "./bin:${SPARK_HOME}/jars/*:${HDPATH}" -sourcepath . -d bin ./main/com/nec/frovedis/mllib/regression/LinearSVR.scala

bin/com/nec/frovedis/mllib/classification/LogisticRegression.class: main/com/nec/frovedis/mllib/classification/LogisticRegression.scala bin/com/nec/frovedis/exrpc/FrovedisLabeledPoint.class bin/com/nec/frovedis/mllib/regression/GeneralizedLinearModel.class bin/com/nec/frovedis/io/FrovedisIO.class
	${SCALA_HOME}/bin/scalac -cp "./bin:${SPARK_HOME}/jars/*:${HDPATH}" -sourcepath . -d bin ./main/com/nec/frovedis/mllib/classification/LogisticRegression.scala
bin/com/nec/frovedis/mllib/classification/SVMModel.class: main/com/nec/frovedis/mllib/classification/LinearSVC.scala bin/com/nec/frovedis/exrpc/FrovedisLabeledPoint.class bin/com/nec/frovedis/mllib/regression/GeneralizedLinearModel.class bin/com/nec/frovedis/io/FrovedisIO.class
	${SCALA_HOME}/bin/scalac -cp "./bin:${SPARK_HOME}/jars/*:${HDPATH}" -sourcepath . -d bin ./main/com/nec/frovedis/mllib/classification/LinearSVC.scala
bin/com/nec/frovedis/mllib/classification/SVC.class: main/com/nec/frovedis/mllib/classification/SVC.scala bin/com/nec/frovedis/exrpc/FrovedisLabeledPoint.class bin/com/nec/frovedis/mllib/regression/GeneralizedLinearModel.class bin/com/nec/frovedis/io/FrovedisIO.class
	${SCALA_HOME}/bin/scalac -cp "./bin:${SPARK_HOME}/jars/*:${HDPATH}" -sourcepath . -d bin ./main/com/nec/frovedis/mllib/classification/SVC.scala
bin/com/nec/frovedis/mllib/classification/NaiveBayes.class: main/com/nec/frovedis/mllib/classification/NaiveBayes.scala bin/com/nec/frovedis/exrpc/FrovedisLabeledPoint.class bin/com/nec/frovedis/mllib/GenericModel.class
	${SCALA_HOME}/bin/scalac -cp "./bin:${SPARK_HOME}/jars/*:${HDPATH}" -sourcepath . -d bin ./main/com/nec/frovedis/mllib/classification/NaiveBayes.scala
bin/com/nec/frovedis/mllib/recommendation/MatrixFactorizationModel.class: main/com/nec/frovedis/mllib/recommendation/MatrixFactorizationModel.scala bin/com/nec/frovedis/mllib/GenericModel.class
	${SCALA_HOME}/bin/scalac -cp "./bin:${SPARK_HOME}/jars/*:${HDPATH}" -sourcepath . -d bin ./main/com/nec/frovedis/mllib/recommendation/MatrixFactorizationModel.scala
bin/com/nec/frovedis/mllib/recommendation/ALS.class: main/com/nec/frovedis/mllib/recommendation/ALS.scala bin/com/nec/frovedis/mllib/recommendation/MatrixFactorizationModel.class
	${SCALA_HOME}/bin/scalac -cp "./bin:${SPARK_HOME}/jars/*:${HDPATH}" -sourcepath . -d bin ./main/com/nec/frovedis/mllib/recommendation/ALS.scala
bin/com/nec/frovedis/mllib/clustering/KMeansModel.class: main/com/nec/frovedis/mllib/clustering/KMeansModel.scala bin/com/nec/frovedis/exrpc/FrovedisSparseData.class bin/com/nec/frovedis/mllib/GenericModel.class
	${SCALA_HOME}/bin/scalac -cp "./bin:${SPARK_HOME}/jars/*:${HDPATH}" -sourcepath . -d bin ./main/com/nec/frovedis/mllib/clustering/KMeansModel.scala
bin/com/nec/frovedis/mllib/clustering/KMeans.class: main/com/nec/frovedis/mllib/clustering/KMeans.scala bin/com/nec/frovedis/mllib/clustering/KMeansModel.class
	${SCALA_HOME}/bin/scalac -cp "./bin:${SPARK_HOME}/jars/*:${HDPATH}" -sourcepath . -d bin ./main/com/nec/frovedis/mllib/clustering/KMeans.scala
bin/com/nec/frovedis/mllib/clustering/SpectralClustering.class: main/com/nec/frovedis/mllib/clustering/SpectralClustering.scala bin/com/nec/frovedis/exrpc/FrovedisSparseData.class bin/com/nec/frovedis/mllib/GenericModel.class
	${SCALA_HOME}/bin/scalac -cp "./bin:${SPARK_HOME}/jars/*:${HDPATH}" -sourcepath . -d bin ./main/com/nec/frovedis/mllib/clustering/SpectralClustering.scala
bin/com/nec/frovedis/mllib/clustering/SpectralEmbedding.class: main/com/nec/frovedis/mllib/clustering/SpectralEmbedding.scala bin/com/nec/frovedis/exrpc/FrovedisSparseData.class bin/com/nec/frovedis/mllib/GenericModel.class
	${SCALA_HOME}/bin/scalac -cp "./bin:${SPARK_HOME}/jars/*:${HDPATH}" -sourcepath . -d bin ./main/com/nec/frovedis/mllib/clustering/SpectralEmbedding.scala
bin/com/nec/frovedis/mllib/clustering/AgglomerativeClustering.class: main/com/nec/frovedis/mllib/clustering/AgglomerativeClustering.scala bin/com/nec/frovedis/exrpc/FrovedisSparseData.class bin/com/nec/frovedis/mllib/GenericModel.class
	${SCALA_HOME}/bin/scalac -cp "./bin:${SPARK_HOME}/jars/*:${HDPATH}" -sourcepath . -d bin ./main/com/nec/frovedis/mllib/clustering/AgglomerativeClustering.scala
bin/com/nec/frovedis/mllib/clustering/DBSCAN.class: main/com/nec/frovedis/mllib/clustering/DBSCAN.scala bin/com/nec/frovedis/exrpc/FrovedisSparseData.class bin/com/nec/frovedis/mllib/GenericModel.class
	${SCALA_HOME}/bin/scalac -cp "./bin:${SPARK_HOME}/jars/*:${HDPATH}" -sourcepath . -d bin ./main/com/nec/frovedis/mllib/clustering/DBSCAN.scala
bin/com/nec/frovedis/mllib/clustering/LDA.class: main/com/nec/frovedis/mllib/clustering/LDA.scala bin/com/nec/frovedis/exrpc/FrovedisSparseData.class bin/com/nec/frovedis/mllib/GenericModel.class
	${SCALA_HOME}/bin/scalac -cp "./bin:${SPARK_HOME}/jars/*:${HDPATH}" -sourcepath . -d bin ./main/com/nec/frovedis/mllib/clustering/LDA.scala
bin/com/nec/frovedis/mllib/clustering/GaussianMixtureModel.class: main/com/nec/frovedis/mllib/clustering/GaussianMixtureModel.scala bin/com/nec/frovedis/mllib/GenericModel.class bin/com/nec/frovedis/io/FrovedisIO.class
	${SCALA_HOME}/bin/scalac -cp "./bin:${SPARK_HOME}/jars/*:${HDPATH}" -sourcepath . -d bin ./main/com/nec/frovedis/mllib/clustering/GaussianMixtureModel.scala
bin/com/nec/frovedis/mllib/clustering/GaussianMixture.class: main/com/nec/frovedis/mllib/clustering/GaussianMixture.scala bin/com/nec/frovedis/mllib/clustering/GaussianMixtureModel.class
	${SCALA_HOME}/bin/scalac -cp "./bin:${SPARK_HOME}/jars/*:${HDPATH}" -sourcepath . -d bin ./main/com/nec/frovedis/mllib/clustering/GaussianMixture.scala
bin/com/nec/frovedis/mllib/manifold/TSNE.class: main/com/nec/frovedis/mllib/manifold/TSNE.scala bin/com/nec/frovedis/exrpc/FrovedisSparseData.class
	${SCALA_HOME}/bin/scalac -cp "./bin:${SPARK_HOME}/jars/*:${HDPATH}" -sourcepath . -d bin ./main/com/nec/frovedis/mllib/manifold/TSNE.scala
bin/com/nec/frovedis/mllib/fpm/FPUtil.class: main/com/nec/frovedis/mllib/fpm/FPUtil.scala
	${SCALA_HOME}/bin/scalac -cp "./bin:${SPARK_HOME}/jars/*:${HDPATH}" -sourcepath . -d bin ./main/com/nec/frovedis/mllib/fpm/FPUtil.scala
bin/com/nec/frovedis/mllib/fpm/FPGrowth.class: main/com/nec/frovedis/mllib/fpm/FPGrowth.scala bin/com/nec/frovedis/sql/FrovedisGroupedDF.class
	${SCALA_HOME}/bin/scalac -cp "./bin:${SPARK_HOME}/jars/*:${HDPATH}" -sourcepath . -d bin ./main/com/nec/frovedis/mllib/fpm/FPGrowth.scala
bin/com/nec/frovedis/mllib/fm/FMConfig.class: main/com/nec/frovedis/mllib/fm/FMConfig.scala
	${SCALA_HOME}/bin/scalac -cp "./bin:${SPARK_HOME}/jars/*:${HDPATH}" -sourcepath . -d bin ./main/com/nec/frovedis/mllib/fm/FMConfig.scala
bin/com/nec/frovedis/mllib/fm/FactorizationMachine.class: main/com/nec/frovedis/mllib/fm/FactorizationMachine.scala bin/com/nec/frovedis/mllib/fm/FMConfig.class bin/com/nec/frovedis/exrpc/FrovedisLabeledPoint.class bin/com/nec/frovedis/mllib/GenericModel.class
	${SCALA_HOME}/bin/scalac -cp "./bin:${SPARK_HOME}/jars/*:${HDPATH}" -sourcepath . -d bin ./main/com/nec/frovedis/mllib/fm/FactorizationMachine.scala
bin/com/nec/frovedis/mllib/tree/Impurities.class: main/com/nec/frovedis/mllib/tree/Impurities.scala
	${SCALA_HOME}/bin/scalac -cp "./bin:${SPARK_HOME}/jars/*:${HDPATH}" -sourcepath . -d bin ./main/com/nec/frovedis/mllib/tree/Impurities.scala
bin/com/nec/frovedis/mllib/tree/DecisionTreeModel.class: main/com/nec/frovedis/mllib/tree/DecisionTreeModel.scala bin/com/nec/frovedis/matrix/FrovedisDenseMatrix.class bin/com/nec/frovedis/mllib/GenericModel.class bin/com/nec/frovedis/io/FrovedisIO.class
	${SCALA_HOME}/bin/scalac -cp "./bin:${SPARK_HOME}/jars/*:${HDPATH}" -sourcepath . -d bin ./main/com/nec/frovedis/mllib/tree/DecisionTreeModel.scala
bin/com/nec/frovedis/mllib/tree/DecisionTree.class: main/com/nec/frovedis/mllib/tree/DecisionTree.scala bin/com/nec/frovedis/mllib/tree/DecisionTreeModel.class bin/com/nec/frovedis/mllib/tree/Impurities.class bin/com/nec/frovedis/exrpc/FrovedisLabeledPoint.class
	${SCALA_HOME}/bin/scalac -cp "./bin:${SPARK_HOME}/jars/*:${HDPATH}" -sourcepath . -d bin ./main/com/nec/frovedis/mllib/tree/DecisionTree.scala
bin/com/nec/frovedis/mllib/tree/RandomForestModel.class: main/com/nec/frovedis/mllib/tree/RandomForestModel.scala bin/com/nec/frovedis/exrpc/FrovedisSparseData.class bin/com/nec/frovedis/mllib/GenericModel.class bin/com/nec/frovedis/io/FrovedisIO.class
	${SCALA_HOME}/bin/scalac -cp "./bin:${SPARK_HOME}/jars/*:${HDPATH}" -sourcepath . -d bin ./main/com/nec/frovedis/mllib/tree/RandomForestModel.scala
bin/com/nec/frovedis/mllib/tree/RandomForest.class: main/com/nec/frovedis/mllib/tree/RandomForest.scala bin/com/nec/frovedis/mllib/tree/RandomForestModel.class bin/com/nec/frovedis/mllib/tree/Impurities.class bin/com/nec/frovedis/exrpc/FrovedisLabeledPoint.class
	${SCALA_HOME}/bin/scalac -cp "./bin:${SPARK_HOME}/jars/*:${HDPATH}" -sourcepath . -d bin ./main/com/nec/frovedis/mllib/tree/RandomForest.scala
bin/com/nec/frovedis/mllib/tree/Losses.class: main/com/nec/frovedis/mllib/tree/Losses.scala
	${SCALA_HOME}/bin/scalac -cp "./bin:${SPARK_HOME}/jars/*:${HDPATH}" -sourcepath . -d bin ./main/com/nec/frovedis/mllib/tree/Losses.scala
bin/com/nec/frovedis/mllib/tree/GradientBoostedTreesModel.class: main/com/nec/frovedis/mllib/tree/GradientBoostedTreesModel.scala bin/com/nec/frovedis/exrpc/FrovedisSparseData.class bin/com/nec/frovedis/mllib/GenericModel.class bin/com/nec/frovedis/io/FrovedisIO.class
	${SCALA_HOME}/bin/scalac -cp "./bin:${SPARK_HOME}/jars/*:${HDPATH}" -sourcepath . -d bin ./main/com/nec/frovedis/mllib/tree/GradientBoostedTreesModel.scala
bin/com/nec/frovedis/mllib/tree/GradientBoostedTrees.class: main/com/nec/frovedis/mllib/tree/GradientBoostedTrees.scala bin/com/nec/frovedis/mllib/tree/GradientBoostedTreesModel.class bin/com/nec/frovedis/exrpc/FrovedisLabeledPoint.class bin/com/nec/frovedis/mllib/tree/Losses.class
	${SCALA_HOME}/bin/scalac -cp "./bin:${SPARK_HOME}/jars/*:${HDPATH}" -sourcepath . -d bin ./main/com/nec/frovedis/mllib/tree/GradientBoostedTrees.scala
bin/com/nec/frovedis/mllib/feature/Word2Vec.class: main/com/nec/frovedis/mllib/feature/Word2Vec.scala bin/com/nec/frovedis/matrix/FrovedisDenseMatrix.class bin/com/nec/frovedis/mllib/GenericModel.class
	${SCALA_HOME}/bin/scalac -cp "./bin:${SPARK_HOME}/jars/*:${HDPATH}" -sourcepath . -d bin ./main/com/nec/frovedis/mllib/feature/Word2Vec.scala
bin/com/nec/frovedis/mllib/feature/StandardScaler.class: main/com/nec/frovedis/mllib/feature/StandardScaler.scala bin/com/nec/frovedis/exrpc/FrovedisSparseData.class bin/com/nec/frovedis/mllib/GenericModel.class
	${SCALA_HOME}/bin/scalac -cp "./bin:${SPARK_HOME}/jars/*:${HDPATH}" -sourcepath . -d bin ./main/com/nec/frovedis/mllib/feature/StandardScaler.scala
bin/com/nec/frovedis/mllib/neighbors/NearestNeighbors.class: main/com/nec/frovedis/mllib/neighbors/NearestNeighbors.scala bin/com/nec/frovedis/exrpc/FrovedisSparseData.class bin/com/nec/frovedis/mllib/GenericModel.class
	${SCALA_HOME}/bin/scalac -cp "./bin:${SPARK_HOME}/jars/*:${HDPATH}" -sourcepath . -d bin ./main/com/nec/frovedis/mllib/neighbors/NearestNeighbors.scala
bin/com/nec/frovedis/mllib/neighbors/KNeighborsClassifier.class: main/com/nec/frovedis/mllib/neighbors/KNeighborsClassifier.scala bin/com/nec/frovedis/mllib/neighbors/NearestNeighbors.class bin/com/nec/frovedis/exrpc/FrovedisLabeledPoint.class
	${SCALA_HOME}/bin/scalac -cp "./bin:${SPARK_HOME}/jars/*:${HDPATH}" -sourcepath . -d bin ./main/com/nec/frovedis/mllib/neighbors/KNeighborsClassifier.scala
bin/com/nec/frovedis/mllib/neighbors/KNeighborsRegressor.class: main/com/nec/frovedis/mllib/neighbors/KNeighborsRegressor.scala bin/com/nec/frovedis/mllib/neighbors/NearestNeighbors.class bin/com/nec/frovedis/exrpc/FrovedisLabeledPoint.class
	${SCALA_HOME}/bin/scalac -cp "./bin:${SPARK_HOME}/jars/*:${HDPATH}" -sourcepath . -d bin ./main/com/nec/frovedis/mllib/neighbors/KNeighborsRegressor.scala

clean:
	rm -fr bin/com lib/frovedis_client.jar main/cpp/JNISupport.hpp main/com/nec/frovedis/Jmatrix/*~ main/com/nec/frovedis/Jexrpc/*~ main/com/nec/frovedis/Jmllib/*~ main/com/nec/frovedis/Jsql/*~ main/com/nec/frovedis/matrix/*~ main/com/nec/frovedis/exrpc/*~ main/com/nec/frovedis/mllib/*~ main/com/nec/frovedis/mllib/regression/*~ main/com/nec/frovedis/mllib/classification/*~ main/com/nec/frovedis/mllib/recommendation/*~ main/com/nec/frovedis/mllib/clustering/*~ main/com/nec/frovedis/mllib/neighbors/*~ main/com/nec/frovedis/mllib/fpm/*~ main/com/nec/frovedis/mllib/fm/*~ main/com/nec/frovedis/mllib/tree/*~ main/com/nec/frovedis/mllib/feature/*~ main/com/nec/frovedis/sql/*~ main/com/nec/frovedis/mllib/manifold/*~
