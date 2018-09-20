#!/bin/sh

LIBRARY_PATH=../lib/
JARS=../lib/frovedis_client.jar

if [ $# -ge 1 ]; then nproc=$1 
else nproc=2
fi

COMMAND="mpirun -np ${nproc} ../../server/frovedis_server"
#echo $COMMAND

spark-submit --driver-java-options "-Djava.library.path=$LIBRARY_PATH" --jars $JARS --conf spark.driver.memory=8g lib/spark2frovedis_gtest.jar "$COMMAND"
