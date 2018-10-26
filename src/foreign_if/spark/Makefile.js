include ../../../Makefile.in.x86

all:
	 ${JAVA_HOME}/bin/javac -cp "./bin:${SPARK_HOME}/jars/*:${HDPATH}" -sourcepath . -d bin ./main/com/nec/frovedis/Jmatrix/*.java
	 ${JAVA_HOME}/bin/javac -cp "./bin:${SPARK_HOME}/jars/*:${HDPATH}" -sourcepath . -d bin ./main/com/nec/frovedis/Jmllib/*.java
	 ${JAVA_HOME}/bin/javac -cp "./bin:${SPARK_HOME}/jars/*:${HDPATH}" -sourcepath . -d bin ./main/com/nec/frovedis/Jexrpc/*.java
	 ${JAVA_HOME}/bin/javah -cp ./bin:${SPARK_HOME}/jars/* com.nec.frovedis.Jexrpc.JNISupport
	 mv com_nec_frovedis_Jexrpc_JNISupport.h ./main/cpp/JNISupport.hpp
	 ${SCALA_HOME}/bin/scalac -cp "./bin:${SPARK_HOME}/jars/*:${HDPATH}" -sourcepath . -d bin ./main/com/nec/frovedis/matrix/repartition.scala
	 ${SCALA_HOME}/bin/scalac -cp "./bin:${SPARK_HOME}/jars/*:${HDPATH}" -sourcepath . -d bin ./main/com/nec/frovedis/matrix/ScalaCRS.scala
	 ${SCALA_HOME}/bin/scalac -cp "./bin:${SPARK_HOME}/jars/*:${HDPATH}" -sourcepath . -d bin ./main/com/nec/frovedis/matrix/MAT_KIND.scala
	 ${SCALA_HOME}/bin/scalac -cp "./bin:${SPARK_HOME}/jars/*:${HDPATH}" -sourcepath . -d bin ./main/com/nec/frovedis/matrix/TypedDvector.scala
	 ${SCALA_HOME}/bin/scalac -cp "./bin:${SPARK_HOME}/jars/*:${HDPATH}" -sourcepath . -d bin ./main/com/nec/frovedis/matrix/FrovedisDenseMatrix.scala
	 ${SCALA_HOME}/bin/scalac -cp "./bin:${SPARK_HOME}/jars/*:${HDPATH}" -sourcepath . -d bin ./main/com/nec/frovedis/exrpc/FrovedisSparseData.scala
	 ${SCALA_HOME}/bin/scalac -cp "./bin:${SPARK_HOME}/jars/*:${HDPATH}" -sourcepath . -d bin ./main/com/nec/frovedis/exrpc/FrovedisLabeledPoint.scala
	 ${SCALA_HOME}/bin/scalac -cp "./bin:${SPARK_HOME}/jars/*:${HDPATH}" -sourcepath . -d bin ./main/com/nec/frovedis/matrix/blas_wrapper.scala
	 ${SCALA_HOME}/bin/scalac -cp "./bin:${SPARK_HOME}/jars/*:${HDPATH}" -sourcepath . -d bin ./main/com/nec/frovedis/matrix/pblas_wrapper.scala
	 ${SCALA_HOME}/bin/scalac -cp "./bin:${SPARK_HOME}/jars/*:${HDPATH}" -sourcepath . -d bin ./main/com/nec/frovedis/matrix/scalapack_result.scala
	 ${SCALA_HOME}/bin/scalac -cp "./bin:${SPARK_HOME}/jars/*:${HDPATH}" -sourcepath . -d bin ./main/com/nec/frovedis/matrix/pca_result.scala
	 ${SCALA_HOME}/bin/scalac -cp "./bin:${SPARK_HOME}/jars/*:${HDPATH}" -sourcepath . -d bin ./main/com/nec/frovedis/matrix/scalapack_wrapper.scala
	 ${SCALA_HOME}/bin/scalac -cp "./bin:${SPARK_HOME}/jars/*:${HDPATH}" -sourcepath . -d bin ./main/com/nec/frovedis/matrix/lapack_wrapper.scala
	 ${SCALA_HOME}/bin/scalac -cp "./bin:${SPARK_HOME}/jars/*:${HDPATH}" -sourcepath . -d bin ./main/com/nec/frovedis/matrix/RowMatrixUtils.scala
	 ${SCALA_HOME}/bin/scalac -cp "./bin:${SPARK_HOME}/jars/*:${HDPATH}" -sourcepath . -d bin ./main/com/nec/frovedis/sql/Expr.scala
	 ${SCALA_HOME}/bin/scalac -cp "./bin:${SPARK_HOME}/jars/*:${HDPATH}" -sourcepath . -d bin ./main/com/nec/frovedis/sql/DFOperator.scala
	 ${SCALA_HOME}/bin/scalac -cp "./bin:${SPARK_HOME}/jars/*:${HDPATH}" -sourcepath . -d bin ./main/com/nec/frovedis/sql/FrovedisTypedColumn.scala
	 ${SCALA_HOME}/bin/scalac -cp "./bin:${SPARK_HOME}/jars/*:${HDPATH}" -sourcepath . -d bin ./main/com/nec/frovedis/sql/FrovedisColumn.scala
	 ${SCALA_HOME}/bin/scalac -cp "./bin:${SPARK_HOME}/jars/*:${HDPATH}" -sourcepath . -d bin ./main/com/nec/frovedis/sql/FrovedisDataframe.scala
	 ${SCALA_HOME}/bin/scalac -cp "./bin:${SPARK_HOME}/jars/*:${HDPATH}" -sourcepath . -d bin ./main/com/nec/frovedis/mllib/GenericModel.scala
	 ${SCALA_HOME}/bin/scalac -cp "./bin:${SPARK_HOME}/jars/*:${HDPATH}" -sourcepath . -d bin ./main/com/nec/frovedis/mllib/regression/GeneralizedLinearAlgorithm.scala
	 ${SCALA_HOME}/bin/scalac -cp "./bin:${SPARK_HOME}/jars/*:${HDPATH}" -sourcepath . -d bin ./main/com/nec/frovedis/mllib/regression/LinearRegression.scala
	 ${SCALA_HOME}/bin/scalac -cp "./bin:${SPARK_HOME}/jars/*:${HDPATH}" -sourcepath . -d bin ./main/com/nec/frovedis/mllib/regression/Lasso.scala
	 ${SCALA_HOME}/bin/scalac -cp "./bin:${SPARK_HOME}/jars/*:${HDPATH}" -sourcepath . -d bin ./main/com/nec/frovedis/mllib/regression/RidgeRegression.scala
	 ${SCALA_HOME}/bin/scalac -cp "./bin:${SPARK_HOME}/jars/*:${HDPATH}" -sourcepath . -d bin ./main/com/nec/frovedis/mllib/classification/LogisticRegression.scala
	 ${SCALA_HOME}/bin/scalac -cp "./bin:${SPARK_HOME}/jars/*:${HDPATH}" -sourcepath . -d bin ./main/com/nec/frovedis/mllib/classification/SVM.scala
	 ${SCALA_HOME}/bin/scalac -cp "./bin:${SPARK_HOME}/jars/*:${HDPATH}" -sourcepath . -d bin ./main/com/nec/frovedis/mllib/classification/NaiveBayes.scala
	 ${SCALA_HOME}/bin/scalac -cp "./bin:${SPARK_HOME}/jars/*:${HDPATH}" -sourcepath . -d bin ./main/com/nec/frovedis/mllib/recommendation/MatrixFactorizationModel.scala
	 ${SCALA_HOME}/bin/scalac -cp "./bin:${SPARK_HOME}/jars/*:${HDPATH}" -sourcepath . -d bin ./main/com/nec/frovedis/mllib/recommendation/ALS.scala
	 ${SCALA_HOME}/bin/scalac -cp "./bin:${SPARK_HOME}/jars/*:${HDPATH}" -sourcepath . -d bin ./main/com/nec/frovedis/mllib/clustering/KMeansModel.scala
	 ${SCALA_HOME}/bin/scalac -cp "./bin:${SPARK_HOME}/jars/*:${HDPATH}" -sourcepath . -d bin ./main/com/nec/frovedis/mllib/clustering/KMeans.scala
	 ${SCALA_HOME}/bin/scalac -cp "./bin:${SPARK_HOME}/jars/*:${HDPATH}" -sourcepath . -d bin ./main/com/nec/frovedis/mllib/fm/FMConfig.scala
	 ${SCALA_HOME}/bin/scalac -cp "./bin:${SPARK_HOME}/jars/*:${HDPATH}" -sourcepath . -d bin ./main/com/nec/frovedis/mllib/fm/FactorizationMachine.scala
	 ${SCALA_HOME}/bin/scalac -cp "./bin:${SPARK_HOME}/jars/*:${HDPATH}" -sourcepath . -d bin ./main/com/nec/frovedis/mllib/tree/Impurities.scala
	 ${SCALA_HOME}/bin/scalac -cp "./bin:${SPARK_HOME}/jars/*:${HDPATH}" -sourcepath . -d bin ./main/com/nec/frovedis/mllib/tree/DecisionTreeModel.scala
	 ${SCALA_HOME}/bin/scalac -cp "./bin:${SPARK_HOME}/jars/*:${HDPATH}" -sourcepath . -d bin ./main/com/nec/frovedis/mllib/tree/DecisionTree.scala
	 cd bin; ${JAVA_HOME}/bin/jar -cf ../lib/frovedis_client.jar com 

clean:
	rm -fr bin/com lib/frovedis_client.jar main/cpp/JNISupport.hpp main/com/nec/frovedis/Jmatrix/*~ main/com/nec/frovedis/Jexrpc/*~ main/com/nec/frovedis/Jmllib/*~ main/com/nec/frovedis/matrix/*~ main/com/nec/frovedis/exrpc/*~ main/com/nec/frovedis/mllib/*~ main/com/nec/frovedis/mllib/regression/*~ main/com/nec/frovedis/mllib/classification/*~ main/com/nec/frovedis/mllib/recommendation/*~ main/com/nec/frovedis/clustering/*~ main/com/nec/frovedis/fm/*~ main/com/nec/frovedis/tree/*~ main/com/nec/frovedis/sql/*~
