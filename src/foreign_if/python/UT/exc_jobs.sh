#!/bin/bash

export PYTHONPATH=../main/python:$PYTHONPATH
export LD_LIBRARY_PATH=../lib:$LD_LIBRARY_PATH

#list of source targets
src=( nb dt fm bfs cc pagerank sssp tsne eigen scaler )

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
REPORT="python_test_report.csv"

# report header
echo "TARGET, TEST_ID, FROV_WORKER, STATUS" > ${REPORT}

# loop iterating over all tests, build them and execute one-by-one
for each in "${src[@]}"; do
  all=`ls src/$each/test*`
  for i in ${all}; do
    name=`echo ${i} | cut -d '/' -f 3 | cut -d '.' -f 1`
    for k in "${frov_worker[@]}"; do
      out="out/$each/nproc_$k/${name}.frovout"
      err="err/$each/nproc_$k/${name}.froverr"
      CMD="mpirun -np $k ../../server/frovedis_server"
      python $i "$CMD" >${out} 2>${err}
      grep "Passed" $out >& /dev/null
      pass_stat=$? 
      grep "Exception" $out >& /dev/null
      excp_stat=$? 
      if [ $pass_stat -eq 0 ]; then stat="Passed"
      elif [ $excp_stat -eq 0 ]; then stat="Exception"
      else stat="Failed"
      fi
      echo "$each, $name, $k, $stat" >> ${REPORT}
    done
  done
done

