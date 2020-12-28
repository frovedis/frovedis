#!/bin/bash

#list of source targets
src=( nb fm dt lr svm lnr ridge lasso kmeans tsne )
#list of target frovedis worker nodes to test with
frov_worker=( 1 2 4 )

# creating directory structure
mkdir -p err out
for each in "${src[@]}"; do
  mkdir -p out/$each
  mkdir -p out/$each
  for k in "${frov_worker[@]}"; do
    mkdir -p out/$each/nproc_$k
    mkdir -p err/$each/nproc_$k
  done
done

# report file name
REPORT="spark_test_report.csv"

# report header
echo "TARGET, TEST_ID, FROV_WORKER, STATUS" > ${REPORT}

# loop iterating over all tests, build them and execute one-by-one
for each in "${src[@]}"; do
  cp -rf src/$each scala
  all=`ls scala/test*`
  for i in ${all}; do
    cp ${i} scala/Gtest.scala
    rm -fr *~ bin/test lib/*
    make -B
    name=`echo ${i} | cut -d '/' -f 2 | cut -d '.' -f 1`
    echo "running $each/$name"
    for k in "${frov_worker[@]}"; do
      out="out/$each/nproc_$k/${name}.frovout"
      err="err/$each/nproc_$k/${name}.froverr"
      sh ./run.sh $k >${out} 2>${err}
      grep "Passed" $out >& /dev/null
      if [ $? -eq 0 ]; then stat="Passed"
      else stat="Failed"
      fi
      echo "$each, $name, $k, $stat" >> ${REPORT}
    done
  done
  rm -rf scala
done

