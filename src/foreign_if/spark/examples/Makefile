include ../../../../Makefile.in.x86
#include ./Makefile.in.x86

CLIENT_JAR := ../lib/frovedis_client.jar
#CLIENT_JAR := ${INSTALLPATH}/lib/spark/frovedis_client.jar

all: 
	install -d ./lib
	instlal -d ./bin
	install -d ./out
	${SCALA_HOME}/bin/scalac -cp "${SPARK_HOME}/jars/*:${HDPATH}:${CLIENT_JAR}" -sourcepath . -d bin scala/DataTransferDemo.scala
	${SCALA_HOME}/bin/scalac -cp "${SPARK_HOME}/jars/*:${HDPATH}:${CLIENT_JAR}" -sourcepath . -d bin scala/LRSGDDemo.scala
	${SCALA_HOME}/bin/scalac -cp "${SPARK_HOME}/jars/*:${HDPATH}:${CLIENT_JAR}" -sourcepath . -d bin scala/LRModelDemo.scala
	${SCALA_HOME}/bin/scalac -cp "${SPARK_HOME}/jars/*:${HDPATH}:${CLIENT_JAR}" -sourcepath . -d bin scala/ALSDemo.scala
	${SCALA_HOME}/bin/scalac -cp "${SPARK_HOME}/jars/*:${HDPATH}:${CLIENT_JAR}" -sourcepath . -d bin scala/KMeansDemo.scala
	${SCALA_HOME}/bin/scalac -cp "${SPARK_HOME}/jars/*:${HDPATH}:${CLIENT_JAR}" -sourcepath . -d bin scala/FrovedisMatrixDemo.scala
	${SCALA_HOME}/bin/scalac -cp "${SPARK_HOME}/jars/*:${HDPATH}:${CLIENT_JAR}" -sourcepath . -d bin scala/PblasScalapackDemo.scala
	${SCALA_HOME}/bin/scalac -cp "${SPARK_HOME}/jars/*:${HDPATH}:${CLIENT_JAR}" -sourcepath . -d bin scala/ArpackDemo.scala
	cd MANIFEST; make
	cd bin; ${JAVA_HOME}/bin/jar -cfm ../lib/spark_frovedis_data_transfer.jar ../MANIFEST/dtransfer.mf test/scala/DataTransfer* 
	cd bin; ${JAVA_HOME}/bin/jar -cfm ../lib/spark2frovedis_lrsgd.jar ../MANIFEST/lrsgd.mf test/scala/LRSGD*
	cd bin; ${JAVA_HOME}/bin/jar -cfm ../lib/spark2frovedis_lrm.jar ../MANIFEST/lrm.mf test/scala/LRModel*
	cd bin; ${JAVA_HOME}/bin/jar -cfm ../lib/spark2frovedis_als.jar ../MANIFEST/als.mf test/scala/ALSDemo*
	cd bin; ${JAVA_HOME}/bin/jar -cfm ../lib/spark2frovedis_kmeans.jar ../MANIFEST/kmeans.mf test/scala/KMeansDemo*
	cd bin; ${JAVA_HOME}/bin/jar -cfm ../lib/spark2frovedis_fel_mat.jar ../MANIFEST/fel_mat.mf test/scala/FrovedisMatrixDemo*
	cd bin; ${JAVA_HOME}/bin/jar -cfm ../lib/spark2frovedis_pblas_scalapack.jar ../MANIFEST/pblas_scalapack.mf test/scala/PblasScalapackDemo*
	cd bin; ${JAVA_HOME}/bin/jar -cfm ../lib/spark2frovedis_arpack.jar ../MANIFEST/arpack.mf test/scala/ArpackDemo*

clean:
	rm -fr *~ bin/test scala/*~ MANIFEST/*.mf MANIFEST/append MANIFEST/*~ out/* lib/*
