include ../../../../Makefile.in.x86
#include ./Makefile.in.x86

CLIENT_JAR := ../lib/frovedis_client.jar
#CLIENT_JAR := ${INSTALLPATH}/lib/spark/frovedis_client.jar

all: 
	install -d ./lib
	install -d ./bin
	install -d ./out
	${SCALA_HOME}/bin/scalac -cp "${SPARK_HOME}/jars/*:${HDPATH}:${CLIENT_JAR}" -sourcepath . -d bin scala/DataTransferDemo.scala
	${SCALA_HOME}/bin/scalac -cp "${SPARK_HOME}/jars/*:${HDPATH}:${CLIENT_JAR}" -sourcepath . -d bin scala/FrovedisMatrixDemo.scala
	${SCALA_HOME}/bin/scalac -cp "${SPARK_HOME}/jars/*:${HDPATH}:${CLIENT_JAR}" -sourcepath . -d bin scala/FrovedisDataframeDemo.scala
	${SCALA_HOME}/bin/scalac -cp "${SPARK_HOME}/jars/*:${HDPATH}:${CLIENT_JAR}" -sourcepath . -d bin scala/PblasScalapackDemo.scala
	${SCALA_HOME}/bin/scalac -cp "${SPARK_HOME}/jars/*:${HDPATH}:${CLIENT_JAR}" -sourcepath . -d bin scala/ArpackDemo.scala
	${SCALA_HOME}/bin/scalac -cp "${SPARK_HOME}/jars/*:${HDPATH}:${CLIENT_JAR}" -sourcepath . -d bin scala/LRSGDDemo.scala
	${SCALA_HOME}/bin/scalac -cp "${SPARK_HOME}/jars/*:${HDPATH}:${CLIENT_JAR}" -sourcepath . -d bin scala/LRModelDemo.scala
	${SCALA_HOME}/bin/scalac -cp "${SPARK_HOME}/jars/*:${HDPATH}:${CLIENT_JAR}" -sourcepath . -d bin scala/ALSDemo.scala
	${SCALA_HOME}/bin/scalac -cp "${SPARK_HOME}/jars/*:${HDPATH}:${CLIENT_JAR}" -sourcepath . -d bin scala/KMeansDemo.scala
	${SCALA_HOME}/bin/scalac -cp "${SPARK_HOME}/jars/*:${HDPATH}:${CLIENT_JAR}" -sourcepath . -d bin scala/NaiveBayesDemo.scala
	${SCALA_HOME}/bin/scalac -cp "${SPARK_HOME}/jars/*:${HDPATH}:${CLIENT_JAR}" -sourcepath . -d bin scala/DecisionTreeDemo.scala
	${SCALA_HOME}/bin/scalac -cp "${SPARK_HOME}/jars/*:${HDPATH}:${CLIENT_JAR}" -sourcepath . -d bin scala/FactorizationMachineDemo.scala
	cd MANIFEST; make
	cd bin; ${JAVA_HOME}/bin/jar -cfm ../lib/spark2frovedis_data_transfer.jar ../MANIFEST/dtransfer.mf test/scala/DataTransfer* 
	cd bin; ${JAVA_HOME}/bin/jar -cfm ../lib/spark2frovedis_mat.jar ../MANIFEST/frov_mat.mf test/scala/FrovedisMatrixDemo*
	cd bin; ${JAVA_HOME}/bin/jar -cfm ../lib/spark2frovedis_df.jar ../MANIFEST/frov_df.mf test/scala/FrovedisDataframeDemo*
	cd bin; ${JAVA_HOME}/bin/jar -cfm ../lib/spark2frovedis_pblas_scalapack.jar ../MANIFEST/pblas_scalapack.mf test/scala/PblasScalapackDemo*
	cd bin; ${JAVA_HOME}/bin/jar -cfm ../lib/spark2frovedis_arpack.jar ../MANIFEST/arpack.mf test/scala/ArpackDemo*
	cd bin; ${JAVA_HOME}/bin/jar -cfm ../lib/spark2frovedis_lrsgd.jar ../MANIFEST/lrsgd.mf test/scala/LRSGD*
	cd bin; ${JAVA_HOME}/bin/jar -cfm ../lib/spark2frovedis_lrm.jar ../MANIFEST/lrm.mf test/scala/LRModel*
	cd bin; ${JAVA_HOME}/bin/jar -cfm ../lib/spark2frovedis_als.jar ../MANIFEST/als.mf test/scala/ALSDemo*
	cd bin; ${JAVA_HOME}/bin/jar -cfm ../lib/spark2frovedis_kmeans.jar ../MANIFEST/kmeans.mf test/scala/KMeansDemo*
	cd bin; ${JAVA_HOME}/bin/jar -cfm ../lib/spark2frovedis_nb.jar ../MANIFEST/nb.mf test/scala/NaiveBayesDemo*
	cd bin; ${JAVA_HOME}/bin/jar -cfm ../lib/spark2frovedis_dt.jar ../MANIFEST/dt.mf test/scala/DecisionTreeDemo*
	cd bin; ${JAVA_HOME}/bin/jar -cfm ../lib/spark2frovedis_fm.jar ../MANIFEST/fm.mf test/scala/FactorizationMachineDemo*

clean:
	rm -fr *~ bin/test scala/*~ MANIFEST/*.mf MANIFEST/append MANIFEST/*~ out/* lib/*
