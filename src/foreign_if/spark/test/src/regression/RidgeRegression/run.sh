#!/bin/sh

ROOT=../../../
LIBRARY_PATH=${ROOT}/../lib/
JARS=${ROOT}/../lib/frovedis_client.jar

opt="sgd"
data="../../../input/regression.txt"

SPARK_WORKER=2
COMMAND="mpirun -np 2 ${ROOT}/../../server/frovedis_server"

# by default, Spark runs as local mode
# if you use distributed mode, put the input files to HDFS
# hadoop fs -put input

echo -n "Testing Ridge Regression........" 
# --- Frovedis: CONFIGURATION ---
iter=1000
mb=1.0
alpha=0.0001
# ---------------------
spark-submit --driver-java-options "-Djava.library.path=$LIBRARY_PATH" --jars $JARS --master local[${SPARK_WORKER}] --conf spark.driver.memory=8g ${ROOT}/lib/ridge_frovedis.jar --cmd "$COMMAND" --input "$data" --num_iter $iter --minibatch_fr $mb --step_size $alpha --opt $opt 2>.stderr >.out1

# --- Spark: CONFIGURATION ---
iter=1000
mb=1.0
alpha=0.0001
# ---------------------
spark-submit --driver-java-options "-Djava.library.path=$LIBRARY_PATH" --jars $JARS --master local[${SPARK_WORKER}] --conf spark.driver.memory=8g ${ROOT}/lib/ridge_spark.jar --cmd "$COMMAND" --input "$data" --num_iter $iter --minibatch_fr $mb --step_size $alpha --opt $opt 2>.stderr >.out2

e1=`grep "MSE" .out1 | tr -s ' ' | cut -d : -f 2`
e2=`grep "MSE" .out2 | tr -s ' ' | cut -d : -f 2`
echo "[Frovedis MSE: $e1; Spark MSE: $e2]"

rm -rf .out1 .out2 .stderr spark-warehouse
