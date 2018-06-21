#!/bin/sh

ROOT=../../../
LIBRARY_PATH=${ROOT}/../lib/
JARS=${ROOT}/../lib/frovedis_client.jar

data="../../../input/kmeans_data.txt"

SPARK_WORKER=2
COMMAND="mpirun -np 2 ${ROOT}/../../server/frovedis_server"

# by default, Spark runs as local mode
# if you use distributed mode, put the input files to HDFS
# hadoop fs -put input

echo -n "Testing KMeans........" 
# --- Frovedis: CONFIGURATION ---
k=4
iter=20
epsilon=0.01
seed=0
# ---------------------
spark-submit --driver-java-options "-Djava.library.path=$LIBRARY_PATH" --jars $JARS --master local[${SPARK_WORKER}] --conf spark.driver.memory=8g ${ROOT}/lib/kmeans_frovedis.jar --cmd "$COMMAND" --input "$data" --k $k --num_iter $iter --epsilon $epsilon --seed $seed 2>.stderr >.out1

# --- Spark: CONFIGURATION ---
k=4
iter=20
epsilon=0.01
seed=0
# ---------------------
spark-submit --driver-java-options "-Djava.library.path=$LIBRARY_PATH" --jars $JARS --master local[${SPARK_WORKER}] --conf spark.driver.memory=8g ${ROOT}/lib/kmeans_spark.jar --cmd "$COMMAND" --input "$data" --k $k --num_iter $iter --epsilon $epsilon --seed $seed 2>.stderr >.out2

diff .out1 .out2 > error

if [ $? -eq 0 ]; then
  echo "[Passed]"
else
  echo "[Failed]"
  cat error
fi

rm -rf .out1 .out2 .stderr error spark-warehouse
