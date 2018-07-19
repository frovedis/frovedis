#!/bin/sh

LIBRARY_PATH=../lib/
JARS=../lib/frovedis_client.jar
COMMAND="mpirun -np 2 ../../server/frovedis_server"

spark-submit --driver-java-options "-Djava.library.path=$LIBRARY_PATH" --jars $JARS --conf spark.driver.memory=8g lib/spark2frovedis_df.jar "$COMMAND"
