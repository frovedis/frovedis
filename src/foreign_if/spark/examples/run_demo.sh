#!/bin/sh

LIBRARY_PATH=../lib/
JARS=../lib/frovedis_client.jar
COMMAND="mpirun -np 2 ../../server/frovedis_server"
#COMMAND="mpirun -np 1 $FROVEDIS_SERVER" 
#LIBRARY_PATH=$X86_INSTALLPATH/lib
#JARS=$X86_INSTALLPATH/lib/spark/frovedis_client.jar

# by default, Spark runs as local mode
# if you use distributed mode, put the input files to HDFS
# hadoop fs -put input
if [ ! -d out ]; then
	mkdir out
fi
spark-submit --driver-java-options "-Djava.library.path=$LIBRARY_PATH" --jars $JARS --conf spark.driver.memory=8g lib/spark_frovedis_data_transfer.jar "$COMMAND"
spark-submit --driver-java-options "-Djava.library.path=$LIBRARY_PATH" --jars $JARS --conf spark.driver.memory=8g lib/spark2frovedis_lrsgd.jar "$COMMAND"
spark-submit --driver-java-options "-Djava.library.path=$LIBRARY_PATH" --jars $JARS --conf spark.driver.memory=8g lib/spark2frovedis_lrm.jar  "$COMMAND"
spark-submit --driver-java-options "-Djava.library.path=$LIBRARY_PATH" --jars $JARS --conf spark.driver.memory=8g lib/spark2frovedis_als.jar "$COMMAND"
spark-submit --driver-java-options "-Djava.library.path=$LIBRARY_PATH" --jars $JARS --conf spark.driver.memory=8g lib/spark2frovedis_kmeans.jar "$COMMAND"
spark-submit --driver-java-options "-Djava.library.path=$LIBRARY_PATH" --jars $JARS --conf spark.driver.memory=8g lib/spark2frovedis_fel_mat.jar "$COMMAND"
spark-submit --driver-java-options "-Djava.library.path=$LIBRARY_PATH" --jars $JARS --conf spark.driver.memory=8g lib/spark2frovedis_pblas_scalapack.jar "$COMMAND"
spark-submit --driver-java-options "-Djava.library.path=$LIBRARY_PATH" --jars $JARS --conf spark.driver.memory=8g lib/spark2frovedis_arpack.jar "$COMMAND"
