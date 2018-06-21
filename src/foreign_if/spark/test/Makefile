include ../../../../Makefile.in.x86
#include ./Makefile.in.x86

CLIENT_JAR := ../lib/frovedis_client.jar
#CLIENT_JAR := ${INSTALLPATH}/lib/spark/frovedis_client.jar

all: LogisticRegression LinearSVM LinearRegression LassoRegression RidgeRegression ALS KMeans

# --------
LogisticRegression: 
	${SCALA_HOME}/bin/scalac -cp "${SPARK_HOME}/jars/*:${HDPATH}:${CLIENT_JAR}" -sourcepath . -d bin src/classification/LogisticRegression/LRFrovedisDemo.scala
	${SCALA_HOME}/bin/scalac -cp "${SPARK_HOME}/jars/*:${HDPATH}:${CLIENT_JAR}" -sourcepath . -d bin src/classification/LogisticRegression/LRSparkDemo.scala
	cd MANIFEST; make lr_frovedis.mf; make lr_spark.mf
	cd bin; ${JAVA_HOME}/bin/jar -cfm ../lib/lr_frovedis.jar ../MANIFEST/lr_frovedis.mf test/classification/LogisticRegression/LRFrovedis*
	cd bin; ${JAVA_HOME}/bin/jar -cfm ../lib/lr_spark.jar ../MANIFEST/lr_spark.mf test/classification/LogisticRegression/LRSpark*
# --------
LinearSVM:
	${SCALA_HOME}/bin/scalac -cp "${SPARK_HOME}/jars/*:${HDPATH}:${CLIENT_JAR}" -sourcepath . -d bin src/classification/LinearSVM/SVMFrovedisDemo.scala
	${SCALA_HOME}/bin/scalac -cp "${SPARK_HOME}/jars/*:${HDPATH}:${CLIENT_JAR}" -sourcepath . -d bin src/classification/LinearSVM/SVMSparkDemo.scala
	cd MANIFEST; make svm_frovedis.mf; make svm_spark.mf
	cd bin; ${JAVA_HOME}/bin/jar -cfm ../lib/svm_frovedis.jar ../MANIFEST/svm_frovedis.mf test/classification/LinearSVM/SVMFrovedis*
	cd bin; ${JAVA_HOME}/bin/jar -cfm ../lib/svm_spark.jar ../MANIFEST/svm_spark.mf test/classification/LinearSVM/SVMSpark*
# --------
LinearRegression:
	${SCALA_HOME}/bin/scalac -cp "${SPARK_HOME}/jars/*:${HDPATH}:${CLIENT_JAR}" -sourcepath . -d bin src/regression/LinearRegression/LNRFrovedisDemo.scala
	${SCALA_HOME}/bin/scalac -cp "${SPARK_HOME}/jars/*:${HDPATH}:${CLIENT_JAR}" -sourcepath . -d bin src/regression/LinearRegression/LNRSparkDemo.scala
	cd MANIFEST; make lnr_frovedis.mf; make lnr_spark.mf
	cd bin; ${JAVA_HOME}/bin/jar -cfm ../lib/lnr_frovedis.jar ../MANIFEST/lnr_frovedis.mf test/regression/LinearRegression/LNRFrovedis*
	cd bin; ${JAVA_HOME}/bin/jar -cfm ../lib/lnr_spark.jar ../MANIFEST/lnr_spark.mf test/regression/LinearRegression/LNRSpark*
# --------
LassoRegression:
	${SCALA_HOME}/bin/scalac -cp "${SPARK_HOME}/jars/*:${HDPATH}:${CLIENT_JAR}" -sourcepath . -d bin src/regression/LassoRegression/LassoFrovedisDemo.scala
	${SCALA_HOME}/bin/scalac -cp "${SPARK_HOME}/jars/*:${HDPATH}:${CLIENT_JAR}" -sourcepath . -d bin src/regression/LassoRegression/LassoSparkDemo.scala
	cd MANIFEST; make lasso_frovedis.mf; make lasso_spark.mf
	cd bin; ${JAVA_HOME}/bin/jar -cfm ../lib/lasso_frovedis.jar ../MANIFEST/lasso_frovedis.mf test/regression/LassoRegression/LassoFrovedis*
	cd bin; ${JAVA_HOME}/bin/jar -cfm ../lib/lasso_spark.jar ../MANIFEST/lasso_spark.mf test/regression/LassoRegression/LassoSpark*
# --------
RidgeRegression:
	${SCALA_HOME}/bin/scalac -cp "${SPARK_HOME}/jars/*:${HDPATH}:${CLIENT_JAR}" -sourcepath . -d bin src/regression/RidgeRegression/RidgeFrovedisDemo.scala
	${SCALA_HOME}/bin/scalac -cp "${SPARK_HOME}/jars/*:${HDPATH}:${CLIENT_JAR}" -sourcepath . -d bin src/regression/RidgeRegression/RidgeSparkDemo.scala
	cd MANIFEST; make ridge_frovedis.mf; make ridge_spark.mf
	cd bin; ${JAVA_HOME}/bin/jar -cfm ../lib/ridge_frovedis.jar ../MANIFEST/ridge_frovedis.mf test/regression/RidgeRegression/RidgeFrovedis*
	cd bin; ${JAVA_HOME}/bin/jar -cfm ../lib/ridge_spark.jar ../MANIFEST/ridge_spark.mf test/regression/RidgeRegression/RidgeSpark*
# --------
ALS:
	${SCALA_HOME}/bin/scalac -cp "${SPARK_HOME}/jars/*:${HDPATH}:${CLIENT_JAR}" -sourcepath . -d bin src/recommendation/ALS/ALSFrovedisDemo.scala
	${SCALA_HOME}/bin/scalac -cp "${SPARK_HOME}/jars/*:${HDPATH}:${CLIENT_JAR}" -sourcepath . -d bin src/recommendation/ALS/ALSSparkDemo.scala
	cd MANIFEST; make als_frovedis.mf; make als_spark.mf
	cd bin; ${JAVA_HOME}/bin/jar -cfm ../lib/als_frovedis.jar ../MANIFEST/als_frovedis.mf test/recommendation/ALS/ALSFrovedis*
	cd bin; ${JAVA_HOME}/bin/jar -cfm ../lib/als_spark.jar ../MANIFEST/als_spark.mf test/recommendation/ALS/ALSSpark*
# --------
KMeans:
	${SCALA_HOME}/bin/scalac -cp "${SPARK_HOME}/jars/*:${HDPATH}:${CLIENT_JAR}" -sourcepath . -d bin src/clustering/KMeans/KmeansFrovedisDemo.scala
	${SCALA_HOME}/bin/scalac -cp "${SPARK_HOME}/jars/*:${HDPATH}:${CLIENT_JAR}" -sourcepath . -d bin src/clustering/KMeans/KmeansSparkDemo.scala
	cd MANIFEST; make kmeans_frovedis.mf; make kmeans_spark.mf
	cd bin; ${JAVA_HOME}/bin/jar -cfm ../lib/kmeans_frovedis.jar ../MANIFEST/kmeans_frovedis.mf test/clustering/KMeans/KmeansFrovedis*
	cd bin; ${JAVA_HOME}/bin/jar -cfm ../lib/kmeans_spark.jar ../MANIFEST/kmeans_spark.mf test/clustering/KMeans/KmeansSpark*
# --------

test: testLogisticRegression testLinearSVM testLinearRegression testLassoRegression testRidgeRegression testALS testKMeans

testLogisticRegression:
	@cd src/classification/LogisticRegression && ./run.sh

testLinearSVM:
	@cd src/classification/LinearSVM && ./run.sh

testLinearRegression:
	@cd src/regression/LinearRegression && ./run.sh

testRidgeRegression:
	@cd src/regression/RidgeRegression && ./run.sh

testLassoRegression:
	@cd src/regression/LassoRegression && ./run.sh

testALS:
	@cd src/recommendation/ALS && ./run.sh

testKMeans:
	@cd src/clustering/KMeans && ./run.sh

# --------

clean:
	rm -fr *~ lib/* bin/test src/classification/LogisticRegression/*~ src/classification/LinearSVM/*~ src/regression/LinearRegression/*~ src/regression/RidgeRegression/*~ src/regression/LassoRegression/*~  src/recommendation/ALS/*~ src/clustering/KMeans/*~ MANIFEST/*.mf MANIFEST/append MANIFEST/*~ 
