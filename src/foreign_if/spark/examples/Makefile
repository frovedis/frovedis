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
	${SCALA_HOME}/bin/scalac -cp "${SPARK_HOME}/jars/*:${HDPATH}:${CLIENT_JAR}" -sourcepath . -d bin scala/SVDDemo.scala
	${SCALA_HOME}/bin/scalac -cp "${SPARK_HOME}/jars/*:${HDPATH}:${CLIENT_JAR}" -sourcepath . -d bin scala/PCADemo.scala
	${SCALA_HOME}/bin/scalac -cp "${SPARK_HOME}/jars/*:${HDPATH}:${CLIENT_JAR}" -sourcepath . -d bin scala/LRSGDDemo.scala
	${SCALA_HOME}/bin/scalac -cp "${SPARK_HOME}/jars/*:${HDPATH}:${CLIENT_JAR}" -sourcepath . -d bin scala/LRModelDemo.scala
	${SCALA_HOME}/bin/scalac -cp "${SPARK_HOME}/jars/*:${HDPATH}:${CLIENT_JAR}" -sourcepath . -d bin scala/ALSDemo.scala
	${SCALA_HOME}/bin/scalac -cp "${SPARK_HOME}/jars/*:${HDPATH}:${CLIENT_JAR}" -sourcepath . -d bin scala/NaiveBayesDemo.scala
	${SCALA_HOME}/bin/scalac -cp "${SPARK_HOME}/jars/*:${HDPATH}:${CLIENT_JAR}" -sourcepath . -d bin scala/DecisionTreeDemo.scala
	${SCALA_HOME}/bin/scalac -cp "${SPARK_HOME}/jars/*:${HDPATH}:${CLIENT_JAR}" -sourcepath . -d bin scala/FactorizationMachineDemo.scala
	${SCALA_HOME}/bin/scalac -cp "${SPARK_HOME}/jars/*:${HDPATH}:${CLIENT_JAR}" -sourcepath . -d bin scala/KMeansDemo.scala
	${SCALA_HOME}/bin/scalac -cp "${SPARK_HOME}/jars/*:${HDPATH}:${CLIENT_JAR}" -sourcepath . -d bin scala/ACDemo.scala
	${SCALA_HOME}/bin/scalac -cp "${SPARK_HOME}/jars/*:${HDPATH}:${CLIENT_JAR}" -sourcepath . -d bin scala/SCADemo.scala
	${SCALA_HOME}/bin/scalac -cp "${SPARK_HOME}/jars/*:${HDPATH}:${CLIENT_JAR}" -sourcepath . -d bin scala/SEADemo.scala
	${SCALA_HOME}/bin/scalac -cp "${SPARK_HOME}/jars/*:${HDPATH}:${CLIENT_JAR}" -sourcepath . -d bin scala/FPGrowthDemo.scala
	${SCALA_HOME}/bin/scalac -cp "${SPARK_HOME}/jars/*:${HDPATH}:${CLIENT_JAR}" -sourcepath . -d bin scala/Word2VectorDemo.scala
	${SCALA_HOME}/bin/scalac -cp "${SPARK_HOME}/jars/*:${HDPATH}:${CLIENT_JAR}" -sourcepath . -d bin scala/FrovedisPageRank.scala
	${SCALA_HOME}/bin/scalac -cp "${SPARK_HOME}/jars/*:${HDPATH}:${CLIENT_JAR}" -sourcepath . -d bin scala/DBSCANDemo.scala
	${SCALA_HOME}/bin/scalac -cp "${SPARK_HOME}/jars/*:${HDPATH}:${CLIENT_JAR}" -sourcepath . -d bin scala/KNNDemo.scala
	${SCALA_HOME}/bin/scalac -cp "${SPARK_HOME}/jars/*:${HDPATH}:${CLIENT_JAR}" -sourcepath . -d bin scala/KNCDemo.scala
	${SCALA_HOME}/bin/scalac -cp "${SPARK_HOME}/jars/*:${HDPATH}:${CLIENT_JAR}" -sourcepath . -d bin scala/KNRDemo.scala
	${SCALA_HOME}/bin/scalac -cp "${SPARK_HOME}/jars/*:${HDPATH}:${CLIENT_JAR}" -sourcepath . -d bin scala/FrovedisSSSP.scala
	${SCALA_HOME}/bin/scalac -cp "${SPARK_HOME}/jars/*:${HDPATH}:${CLIENT_JAR}" -sourcepath . -d bin scala/FrovedisBFS.scala
	${SCALA_HOME}/bin/scalac -cp "${SPARK_HOME}/jars/*:${HDPATH}:${CLIENT_JAR}" -sourcepath . -d bin scala/FrovedisCC.scala
	${SCALA_HOME}/bin/scalac -cp "${SPARK_HOME}/jars/*:${HDPATH}:${CLIENT_JAR}" -sourcepath . -d bin scala/LDADemo.scala
	${SCALA_HOME}/bin/scalac -cp "${SPARK_HOME}/jars/*:${HDPATH}:${CLIENT_JAR}" -sourcepath . -d bin scala/RFDemo.scala
	${SCALA_HOME}/bin/scalac -cp "${SPARK_HOME}/jars/*:${HDPATH}:${CLIENT_JAR}" -sourcepath . -d bin scala/GBTDemo.scala
	${SCALA_HOME}/bin/scalac -cp "${SPARK_HOME}/jars/*:${HDPATH}:${CLIENT_JAR}" -sourcepath . -d bin scala/SVRDemo.scala


	cd MANIFEST; make
	cd bin; ${JAVA_HOME}/bin/jar -cfm ../lib/spark2frovedis_data_transfer.jar ../MANIFEST/dtransfer.mf test/scala/DataTransfer* 
	cd bin; ${JAVA_HOME}/bin/jar -cfm ../lib/spark2frovedis_mat.jar ../MANIFEST/frov_mat.mf test/scala/FrovedisMatrixDemo*
	cd bin; ${JAVA_HOME}/bin/jar -cfm ../lib/spark2frovedis_df.jar ../MANIFEST/frov_df.mf test/scala/FrovedisDataframeDemo*
	cd bin; ${JAVA_HOME}/bin/jar -cfm ../lib/spark2frovedis_pblas_scalapack.jar ../MANIFEST/pblas_scalapack.mf test/scala/PblasScalapackDemo*
	cd bin; ${JAVA_HOME}/bin/jar -cfm ../lib/spark2frovedis_svd.jar ../MANIFEST/svd.mf test/scala/SVDDemo*
	cd bin; ${JAVA_HOME}/bin/jar -cfm ../lib/spark2frovedis_pca.jar ../MANIFEST/pca.mf test/scala/PCADemo*
	cd bin; ${JAVA_HOME}/bin/jar -cfm ../lib/spark2frovedis_lrsgd.jar ../MANIFEST/lrsgd.mf test/scala/LRSGD*
	cd bin; ${JAVA_HOME}/bin/jar -cfm ../lib/spark2frovedis_lrm.jar ../MANIFEST/lrm.mf test/scala/LRModel*
	cd bin; ${JAVA_HOME}/bin/jar -cfm ../lib/spark2frovedis_als.jar ../MANIFEST/als.mf test/scala/ALSDemo*
	cd bin; ${JAVA_HOME}/bin/jar -cfm ../lib/spark2frovedis_nb.jar ../MANIFEST/nb.mf test/scala/NaiveBayesDemo*
	cd bin; ${JAVA_HOME}/bin/jar -cfm ../lib/spark2frovedis_dt.jar ../MANIFEST/dt.mf test/scala/DecisionTreeDemo*
	cd bin; ${JAVA_HOME}/bin/jar -cfm ../lib/spark2frovedis_fm.jar ../MANIFEST/fm.mf test/scala/FactorizationMachineDemo*
	cd bin; ${JAVA_HOME}/bin/jar -cfm ../lib/spark2frovedis_kmeans.jar ../MANIFEST/kmeans.mf test/scala/KMeansDemo*
	cd bin; ${JAVA_HOME}/bin/jar -cfm ../lib/spark2frovedis_ac.jar ../MANIFEST/ac.mf test/scala/ACDemo*
	cd bin; ${JAVA_HOME}/bin/jar -cfm ../lib/spark2frovedis_sc.jar ../MANIFEST/sc.mf test/scala/SCADemo*
	cd bin; ${JAVA_HOME}/bin/jar -cfm ../lib/spark2frovedis_se.jar ../MANIFEST/se.mf test/scala/SEADemo*
	cd bin; ${JAVA_HOME}/bin/jar -cfm ../lib/spark2frovedis_fp.jar ../MANIFEST/fp.mf test/scala/FPGrowthDemo*
	cd bin; ${JAVA_HOME}/bin/jar -cfm ../lib/spark2frovedis_w2v.jar ../MANIFEST/w2v.mf test/scala/Word2VecDemo*
	cd bin; ${JAVA_HOME}/bin/jar -cfm ../lib/frovedis_pagerank.jar ../MANIFEST/page_rank.mf test/scala/FrovedisPageRank*
	cd bin; ${JAVA_HOME}/bin/jar -cfm ../lib/frovedis_dbscan.jar ../MANIFEST/dbscan.mf test/scala/DBSCANDemo*
	cd bin; ${JAVA_HOME}/bin/jar -cfm ../lib/spark2frovedis_knn.jar ../MANIFEST/knn.mf test/scala/KNNDemo*
	cd bin; ${JAVA_HOME}/bin/jar -cfm ../lib/spark2frovedis_knc.jar ../MANIFEST/knc.mf test/scala/KNCDemo*
	cd bin; ${JAVA_HOME}/bin/jar -cfm ../lib/spark2frovedis_knr.jar ../MANIFEST/knr.mf test/scala/KNRDemo*
	cd bin; ${JAVA_HOME}/bin/jar -cfm ../lib/frovedis_sssp.jar ../MANIFEST/sssp.mf test/scala/FrovedisSSSP*
	cd bin; ${JAVA_HOME}/bin/jar -cfm ../lib/frovedis_bfs.jar ../MANIFEST/bfs.mf test/scala/FrovedisBFS*
	cd bin; ${JAVA_HOME}/bin/jar -cfm ../lib/frovedis_cc.jar ../MANIFEST/cc.mf test/scala/FrovedisCC*
	cd bin; ${JAVA_HOME}/bin/jar -cfm ../lib/spark2frovedis_lda.jar ../MANIFEST/lda.mf test/scala/LDADemo*
	cd bin; ${JAVA_HOME}/bin/jar -cfm ../lib/spark2frovedis_rf.jar ../MANIFEST/rf.mf test/scala/RFDemo*
	cd bin; ${JAVA_HOME}/bin/jar -cfm ../lib/spark2frovedis_gbt.jar ../MANIFEST/gbt.mf test/scala/GBTDemo*
	cd bin; ${JAVA_HOME}/bin/jar -cfm ../lib/spark2frovedis_svr.jar ../MANIFEST/svr.mf test/scala/SVRDemo*

clean:
	rm -fr *~ bin/test scala/*~ MANIFEST/*.mf MANIFEST/append MANIFEST/*~ out/* lib/*
