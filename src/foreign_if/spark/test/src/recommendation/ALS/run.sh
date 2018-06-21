#!/bin/sh

ROOT=../../../
LIBRARY_PATH=${ROOT}/../lib/
JARS=${ROOT}/../lib/frovedis_client.jar

data="../../../input/rating.txt"

SPARK_WORKER=2
COMMAND="mpirun -np 2 ${ROOT}/../../server/frovedis_server"

# by default, Spark runs as local mode
# if you use distributed mode, put the input files to HDFS
# hadoop fs -put input

echo -n "Testing ALS........" 
# --- Frovedis: CONFIGURATION ---
rank=4
iter=10
lambda=0.01
alpha=0.01
seed=0
# ---------------------
spark-submit --driver-java-options "-Djava.library.path=$LIBRARY_PATH" --jars $JARS --master local[${SPARK_WORKER}] --conf spark.driver.memory=8g ${ROOT}/lib/als_frovedis.jar --cmd "$COMMAND" --input "$data" --rank $rank --num_iter $iter --lambda $lambda --alpha $alpha --seed $seed 2>.stderr >.out1

# --- Spark: CONFIGURATION ---
rank=4
iter=10
lambda=0.01
alpha=0.01
seed=0
# ---------------------
spark-submit --driver-java-options "-Djava.library.path=$LIBRARY_PATH" --jars $JARS --master local[${SPARK_WORKER}] --conf spark.driver.memory=8g ${ROOT}/lib/als_spark.jar --cmd "$COMMAND" --input "$data" --rank $rank --num_iter $iter --lambda $lambda --alpha $alpha --seed $seed 2>.stderr >.out2

e1=`grep "MSE" .out1 | tr -s ' ' | cut -d : -f 2`
e2=`grep "MSE" .out2 | tr -s ' ' | cut -d : -f 2`
echo "[Frovedis MSE: $e1; Spark MSE: $e2]"

rm -rf .out1 .out2 .stderr spark-warehouse
