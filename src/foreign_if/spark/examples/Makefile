include ../../../../Makefile.in.x86
#include ./Makefile.in.x86

CLIENT_JAR := ../lib/frovedis_client.jar
#CLIENT_JAR := ${INSTALLPATH}/lib/spark/frovedis_client.jar

JARS := lib/spark2frovedis_data_transfer.jar lib/spark2frovedis_mat.jar lib/spark2frovedis_df.jar lib/spark2frovedis_pblas_scalapack.jar lib/spark2frovedis_svd.jar lib/spark2frovedis_evd.jar lib/spark2frovedis_pca.jar lib/spark2frovedis_lrsgd.jar lib/spark2frovedis_lrm.jar lib/spark2frovedis_als.jar lib/spark2frovedis_nb.jar lib/spark2frovedis_dt.jar lib/spark2frovedis_fm.jar lib/spark2frovedis_kmeans.jar lib/spark2frovedis_ac.jar lib/spark2frovedis_sc.jar lib/frovedis_gmm.jar lib/spark2frovedis_se.jar lib/spark2frovedis_fp.jar lib/spark2frovedis_w2v.jar lib/frovedis_pagerank.jar lib/frovedis_dbscan.jar lib/spark2frovedis_knn.jar lib/spark2frovedis_knc.jar lib/spark2frovedis_knr.jar lib/frovedis_sssp.jar lib/frovedis_bfs.jar lib/frovedis_cc.jar lib/spark2frovedis_lda.jar lib/spark2frovedis_rf.jar lib/spark2frovedis_gbt.jar lib/spark2frovedis_svr.jar lib/spark2frovedis_svc.jar lib/frovedis_tsne.jar

JOBS := $(shell /usr/bin/nproc --all)

all:
	install -d ./lib
	install -d ./bin
	install -d ./out
	cd MANIFEST; make
	make jars -j ${JOBS}

jars: ${JARS}

bin/test/scala/DataTransferDemo.class: scala/DataTransferDemo.scala
	${SCALA_HOME}/bin/scalac -cp "${SPARK_HOME}/jars/*:${HDPATH}:${CLIENT_JAR}" -sourcepath . -d bin scala/DataTransferDemo.scala
bin/test/scala/FrovedisMatrixDemo.class: scala/FrovedisMatrixDemo.scala
	${SCALA_HOME}/bin/scalac -cp "${SPARK_HOME}/jars/*:${HDPATH}:${CLIENT_JAR}" -sourcepath . -d bin scala/FrovedisMatrixDemo.scala
bin/test/scala/FrovedisDataframeDemo.class: scala/FrovedisDataframeDemo.scala
	${SCALA_HOME}/bin/scalac -cp "${SPARK_HOME}/jars/*:${HDPATH}:${CLIENT_JAR}" -sourcepath . -d bin scala/FrovedisDataframeDemo.scala
bin/test/scala/PblasScalapackDemo.class: scala/PblasScalapackDemo.scala
	${SCALA_HOME}/bin/scalac -cp "${SPARK_HOME}/jars/*:${HDPATH}:${CLIENT_JAR}" -sourcepath . -d bin scala/PblasScalapackDemo.scala
bin/test/scala/SVDDemo.class: scala/SVDDemo.scala
	${SCALA_HOME}/bin/scalac -cp "${SPARK_HOME}/jars/*:${HDPATH}:${CLIENT_JAR}" -sourcepath . -d bin scala/SVDDemo.scala
bin/test/scala/EVDDemo.class: scala/EVDDemo.scala
	${SCALA_HOME}/bin/scalac -cp "${SPARK_HOME}/jars/*:${HDPATH}:${CLIENT_JAR}" -sourcepath . -d bin scala/EVDDemo.scala
bin/test/scala/PCADemo.class: scala/PCADemo.scala
	${SCALA_HOME}/bin/scalac -cp "${SPARK_HOME}/jars/*:${HDPATH}:${CLIENT_JAR}" -sourcepath . -d bin scala/PCADemo.scala
bin/test/scala/LRSGDDemo.class: scala/LRSGDDemo.scala
	${SCALA_HOME}/bin/scalac -cp "${SPARK_HOME}/jars/*:${HDPATH}:${CLIENT_JAR}" -sourcepath . -d bin scala/LRSGDDemo.scala
bin/test/scala/LRModelDemo.class: scala/LRModelDemo.scala
	${SCALA_HOME}/bin/scalac -cp "${SPARK_HOME}/jars/*:${HDPATH}:${CLIENT_JAR}" -sourcepath . -d bin scala/LRModelDemo.scala
bin/test/scala/ALSDemo.class: scala/ALSDemo.scala
	${SCALA_HOME}/bin/scalac -cp "${SPARK_HOME}/jars/*:${HDPATH}:${CLIENT_JAR}" -sourcepath . -d bin scala/ALSDemo.scala
bin/test/scala/NaiveBayesDemo.class: scala/NaiveBayesDemo.scala
	${SCALA_HOME}/bin/scalac -cp "${SPARK_HOME}/jars/*:${HDPATH}:${CLIENT_JAR}" -sourcepath . -d bin scala/NaiveBayesDemo.scala
bin/test/scala/DecisionTreeDemo.class: scala/DecisionTreeDemo.scala
	${SCALA_HOME}/bin/scalac -cp "${SPARK_HOME}/jars/*:${HDPATH}:${CLIENT_JAR}" -sourcepath . -d bin scala/DecisionTreeDemo.scala
bin/test/scala/FactorizationMachineDemo.class: scala/FactorizationMachineDemo.scala
	${SCALA_HOME}/bin/scalac -cp "${SPARK_HOME}/jars/*:${HDPATH}:${CLIENT_JAR}" -sourcepath . -d bin scala/FactorizationMachineDemo.scala
bin/test/scala/KMeansDemo.class: scala/KMeansDemo.scala
	${SCALA_HOME}/bin/scalac -cp "${SPARK_HOME}/jars/*:${HDPATH}:${CLIENT_JAR}" -sourcepath . -d bin scala/KMeansDemo.scala
bin/test/scala/ACDemo.class: scala/ACDemo.scala
	${SCALA_HOME}/bin/scalac -cp "${SPARK_HOME}/jars/*:${HDPATH}:${CLIENT_JAR}" -sourcepath . -d bin scala/ACDemo.scala
bin/test/scala/SCADemo.class: scala/SCADemo.scala
	${SCALA_HOME}/bin/scalac -cp "${SPARK_HOME}/jars/*:${HDPATH}:${CLIENT_JAR}" -sourcepath . -d bin scala/SCADemo.scala
bin/test/scala/GMMDemo.class: scala/GMMDemo.scala
	${SCALA_HOME}/bin/scalac -cp "${SPARK_HOME}/jars/*:${HDPATH}:${CLIENT_JAR}" -sourcepath . -d bin scala/GMMDemo.scala
bin/test/scala/SEADemo.class: scala/SEADemo.scala
	${SCALA_HOME}/bin/scalac -cp "${SPARK_HOME}/jars/*:${HDPATH}:${CLIENT_JAR}" -sourcepath . -d bin scala/SEADemo.scala
bin/test/scala/FPGrowthDemo.class: scala/FPGrowthDemo.scala
	${SCALA_HOME}/bin/scalac -cp "${SPARK_HOME}/jars/*:${HDPATH}:${CLIENT_JAR}" -sourcepath . -d bin scala/FPGrowthDemo.scala
bin/test/scala/Word2VecDemo.class: scala/Word2VectorDemo.scala
	${SCALA_HOME}/bin/scalac -cp "${SPARK_HOME}/jars/*:${HDPATH}:${CLIENT_JAR}" -sourcepath . -d bin scala/Word2VectorDemo.scala
bin/test/scala/FrovedisPageRank.class: scala/FrovedisPageRank.scala
	${SCALA_HOME}/bin/scalac -cp "${SPARK_HOME}/jars/*:${HDPATH}:${CLIENT_JAR}" -sourcepath . -d bin scala/FrovedisPageRank.scala
bin/test/scala/DBSCANDemo.class: scala/DBSCANDemo.scala
	${SCALA_HOME}/bin/scalac -cp "${SPARK_HOME}/jars/*:${HDPATH}:${CLIENT_JAR}" -sourcepath . -d bin scala/DBSCANDemo.scala
bin/test/scala/KNNDemo.class: scala/KNNDemo.scala
	${SCALA_HOME}/bin/scalac -cp "${SPARK_HOME}/jars/*:${HDPATH}:${CLIENT_JAR}" -sourcepath . -d bin scala/KNNDemo.scala
bin/test/scala/KNCDemo.class: scala/KNCDemo.scala
	${SCALA_HOME}/bin/scalac -cp "${SPARK_HOME}/jars/*:${HDPATH}:${CLIENT_JAR}" -sourcepath . -d bin scala/KNCDemo.scala
bin/test/scala/KNRDemo.class: scala/KNRDemo.scala
	${SCALA_HOME}/bin/scalac -cp "${SPARK_HOME}/jars/*:${HDPATH}:${CLIENT_JAR}" -sourcepath . -d bin scala/KNRDemo.scala
bin/test/scala/FrovedisSSSP.class: scala/FrovedisSSSP.scala
	${SCALA_HOME}/bin/scalac -cp "${SPARK_HOME}/jars/*:${HDPATH}:${CLIENT_JAR}" -sourcepath . -d bin scala/FrovedisSSSP.scala
bin/test/scala/FrovedisBFS.class: scala/FrovedisBFS.scala
	${SCALA_HOME}/bin/scalac -cp "${SPARK_HOME}/jars/*:${HDPATH}:${CLIENT_JAR}" -sourcepath . -d bin scala/FrovedisBFS.scala
bin/test/scala/FrovedisCC.class: scala/FrovedisCC.scala
	${SCALA_HOME}/bin/scalac -cp "${SPARK_HOME}/jars/*:${HDPATH}:${CLIENT_JAR}" -sourcepath . -d bin scala/FrovedisCC.scala
bin/test/scala/LDADemo.class: scala/LDADemo.scala
	${SCALA_HOME}/bin/scalac -cp "${SPARK_HOME}/jars/*:${HDPATH}:${CLIENT_JAR}" -sourcepath . -d bin scala/LDADemo.scala
bin/test/scala/RFDemo.class: scala/RFDemo.scala
	${SCALA_HOME}/bin/scalac -cp "${SPARK_HOME}/jars/*:${HDPATH}:${CLIENT_JAR}" -sourcepath . -d bin scala/RFDemo.scala
bin/test/scala/GBTDemo.class: scala/GBTDemo.scala
	${SCALA_HOME}/bin/scalac -cp "${SPARK_HOME}/jars/*:${HDPATH}:${CLIENT_JAR}" -sourcepath . -d bin scala/GBTDemo.scala
bin/test/scala/SVRDemo.class: scala/SVRDemo.scala
	${SCALA_HOME}/bin/scalac -cp "${SPARK_HOME}/jars/*:${HDPATH}:${CLIENT_JAR}" -sourcepath . -d bin scala/SVRDemo.scala
bin/test/scala/SVMKernelDemo.class: scala/SVMKernelDemo.scala
	${SCALA_HOME}/bin/scalac -cp "${SPARK_HOME}/jars/*:${HDPATH}:${CLIENT_JAR}" -sourcepath . -d bin scala/SVMKernelDemo.scala
bin/test/scala/TSNEDemo.class: scala/TSNEDemo.scala
	${SCALA_HOME}/bin/scalac -cp "${SPARK_HOME}/jars/*:${HDPATH}:${CLIENT_JAR}" -sourcepath . -d bin scala/TSNEDemo.scala

lib/spark2frovedis_data_transfer.jar: bin/test/scala/DataTransferDemo.class
	cd bin; ${JAVA_HOME}/bin/jar -cfm ../lib/spark2frovedis_data_transfer.jar ../MANIFEST/dtransfer.mf test/scala/DataTransfer* 
lib/spark2frovedis_mat.jar: bin/test/scala/FrovedisMatrixDemo.class
	cd bin; ${JAVA_HOME}/bin/jar -cfm ../lib/spark2frovedis_mat.jar ../MANIFEST/frov_mat.mf test/scala/FrovedisMatrixDemo*
lib/spark2frovedis_df.jar: bin/test/scala/FrovedisDataframeDemo.class
	cd bin; ${JAVA_HOME}/bin/jar -cfm ../lib/spark2frovedis_df.jar ../MANIFEST/frov_df.mf test/scala/FrovedisDataframeDemo*
lib/spark2frovedis_pblas_scalapack.jar: bin/test/scala/PblasScalapackDemo.class
	cd bin; ${JAVA_HOME}/bin/jar -cfm ../lib/spark2frovedis_pblas_scalapack.jar ../MANIFEST/pblas_scalapack.mf test/scala/PblasScalapackDemo*
lib/spark2frovedis_svd.jar: bin/test/scala/SVDDemo.class
	cd bin; ${JAVA_HOME}/bin/jar -cfm ../lib/spark2frovedis_svd.jar ../MANIFEST/svd.mf test/scala/SVDDemo*
lib/spark2frovedis_evd.jar: bin/test/scala/EVDDemo.class
	cd bin; ${JAVA_HOME}/bin/jar -cfm ../lib/spark2frovedis_evd.jar ../MANIFEST/evd.mf test/scala/EVDDemo*
lib/spark2frovedis_pca.jar: bin/test/scala/PCADemo.class
	cd bin; ${JAVA_HOME}/bin/jar -cfm ../lib/spark2frovedis_pca.jar ../MANIFEST/pca.mf test/scala/PCADemo*
lib/spark2frovedis_lrsgd.jar: bin/test/scala/LRSGDDemo.class
	cd bin; ${JAVA_HOME}/bin/jar -cfm ../lib/spark2frovedis_lrsgd.jar ../MANIFEST/lrsgd.mf test/scala/LRSGD*
lib/spark2frovedis_lrm.jar: bin/test/scala/LRModelDemo.class
	cd bin; ${JAVA_HOME}/bin/jar -cfm ../lib/spark2frovedis_lrm.jar ../MANIFEST/lrm.mf test/scala/LRModel*
lib/spark2frovedis_als.jar: bin/test/scala/ALSDemo.class
	cd bin; ${JAVA_HOME}/bin/jar -cfm ../lib/spark2frovedis_als.jar ../MANIFEST/als.mf test/scala/ALSDemo*
lib/spark2frovedis_nb.jar: bin/test/scala/NaiveBayesDemo.class
	cd bin; ${JAVA_HOME}/bin/jar -cfm ../lib/spark2frovedis_nb.jar ../MANIFEST/nb.mf test/scala/NaiveBayesDemo*
lib/spark2frovedis_dt.jar: bin/test/scala/DecisionTreeDemo.class
	cd bin; ${JAVA_HOME}/bin/jar -cfm ../lib/spark2frovedis_dt.jar ../MANIFEST/dt.mf test/scala/DecisionTreeDemo*
lib/spark2frovedis_fm.jar: bin/test/scala/FactorizationMachineDemo.class
	cd bin; ${JAVA_HOME}/bin/jar -cfm ../lib/spark2frovedis_fm.jar ../MANIFEST/fm.mf test/scala/FactorizationMachineDemo*
lib/spark2frovedis_kmeans.jar: bin/test/scala/KMeansDemo.class
	cd bin; ${JAVA_HOME}/bin/jar -cfm ../lib/spark2frovedis_kmeans.jar ../MANIFEST/kmeans.mf test/scala/KMeansDemo*
lib/spark2frovedis_ac.jar: bin/test/scala/ACDemo.class
	cd bin; ${JAVA_HOME}/bin/jar -cfm ../lib/spark2frovedis_ac.jar ../MANIFEST/ac.mf test/scala/ACDemo*
lib/spark2frovedis_sc.jar: bin/test/scala/SCADemo.class
	cd bin; ${JAVA_HOME}/bin/jar -cfm ../lib/spark2frovedis_sc.jar ../MANIFEST/sc.mf test/scala/SCADemo*
lib/frovedis_gmm.jar: bin/test/scala/GMMDemo.class
	cd bin; ${JAVA_HOME}/bin/jar -cfm ../lib/frovedis_gmm.jar ../MANIFEST/gmm.mf test/scala/GMMDemo*    
lib/spark2frovedis_se.jar: bin/test/scala/SEADemo.class
	cd bin; ${JAVA_HOME}/bin/jar -cfm ../lib/spark2frovedis_se.jar ../MANIFEST/se.mf test/scala/SEADemo*
lib/spark2frovedis_fp.jar: bin/test/scala/FPGrowthDemo.class
	cd bin; ${JAVA_HOME}/bin/jar -cfm ../lib/spark2frovedis_fp.jar ../MANIFEST/fp.mf test/scala/FPGrowthDemo*
lib/spark2frovedis_w2v.jar: bin/test/scala/Word2VecDemo.class
	cd bin; ${JAVA_HOME}/bin/jar -cfm ../lib/spark2frovedis_w2v.jar ../MANIFEST/w2v.mf test/scala/Word2VecDemo*
lib/frovedis_pagerank.jar: bin/test/scala/FrovedisPageRank.class
	cd bin; ${JAVA_HOME}/bin/jar -cfm ../lib/frovedis_pagerank.jar ../MANIFEST/page_rank.mf test/scala/FrovedisPageRank*
lib/frovedis_dbscan.jar: bin/test/scala/DBSCANDemo.class
	cd bin; ${JAVA_HOME}/bin/jar -cfm ../lib/frovedis_dbscan.jar ../MANIFEST/dbscan.mf test/scala/DBSCANDemo*
lib/spark2frovedis_knn.jar: bin/test/scala/KNNDemo.class
	cd bin; ${JAVA_HOME}/bin/jar -cfm ../lib/spark2frovedis_knn.jar ../MANIFEST/knn.mf test/scala/KNNDemo*
lib/spark2frovedis_knc.jar: bin/test/scala/KNCDemo.class
	cd bin; ${JAVA_HOME}/bin/jar -cfm ../lib/spark2frovedis_knc.jar ../MANIFEST/knc.mf test/scala/KNCDemo*
lib/spark2frovedis_knr.jar: bin/test/scala/KNRDemo.class
	cd bin; ${JAVA_HOME}/bin/jar -cfm ../lib/spark2frovedis_knr.jar ../MANIFEST/knr.mf test/scala/KNRDemo*
lib/frovedis_sssp.jar: bin/test/scala/FrovedisSSSP.class
	cd bin; ${JAVA_HOME}/bin/jar -cfm ../lib/frovedis_sssp.jar ../MANIFEST/sssp.mf test/scala/FrovedisSSSP*
lib/frovedis_bfs.jar: bin/test/scala/FrovedisBFS.class
	cd bin; ${JAVA_HOME}/bin/jar -cfm ../lib/frovedis_bfs.jar ../MANIFEST/bfs.mf test/scala/FrovedisBFS*
lib/frovedis_cc.jar: bin/test/scala/FrovedisCC.class
	cd bin; ${JAVA_HOME}/bin/jar -cfm ../lib/frovedis_cc.jar ../MANIFEST/cc.mf test/scala/FrovedisCC*
lib/spark2frovedis_lda.jar: bin/test/scala/LDADemo.class
	cd bin; ${JAVA_HOME}/bin/jar -cfm ../lib/spark2frovedis_lda.jar ../MANIFEST/lda.mf test/scala/LDADemo*
lib/spark2frovedis_rf.jar: bin/test/scala/RFDemo.class
	cd bin; ${JAVA_HOME}/bin/jar -cfm ../lib/spark2frovedis_rf.jar ../MANIFEST/rf.mf test/scala/RFDemo*
lib/spark2frovedis_gbt.jar: bin/test/scala/GBTDemo.class
	cd bin; ${JAVA_HOME}/bin/jar -cfm ../lib/spark2frovedis_gbt.jar ../MANIFEST/gbt.mf test/scala/GBTDemo*
lib/spark2frovedis_svr.jar: bin/test/scala/SVRDemo.class
	cd bin; ${JAVA_HOME}/bin/jar -cfm ../lib/spark2frovedis_svr.jar ../MANIFEST/svr.mf test/scala/SVRDemo*
lib/spark2frovedis_svc.jar: bin/test/scala/SVMKernelDemo.class
	cd bin; ${JAVA_HOME}/bin/jar -cfm ../lib/spark2frovedis_svc.jar ../MANIFEST/svc.mf test/scala/SVMKernelDemo*
lib/frovedis_tsne.jar: bin/test/scala/TSNEDemo.class
	cd bin; ${JAVA_HOME}/bin/jar -cfm ../lib/frovedis_tsne.jar ../MANIFEST/tsne.mf test/scala/TSNEDemo*

clean:
	rm -fr *~ bin/test scala/*~ MANIFEST/*.mf MANIFEST/append MANIFEST/*~ out/* lib/*
