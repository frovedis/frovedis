#!/bin/sh

export PYTHONPATH=../main/python
export LD_LIBRARY_PATH=../lib
COMMAND="mpirun -np 2 ../../server/frovedis_server"
#COMMAND="mpirun -np 1 $FROVEDIS_SERVER" 

if [ ! -d out ]; then
	mkdir out
fi
./crs_matrix_demo.py "$COMMAND"
./lr_demo.py "$COMMAND"
./lnr_demo.py "$COMMAND"
./svm_demo.py "$COMMAND"
./sparse_svd_demo.py "$COMMAND"
./kmeans_demo.py "$COMMAND"
./als_demo.py "$COMMAND"
./dense_matrix_demo.py "$COMMAND"
./pblas_demo.py "$COMMAND"
./scalapack_demo.py "$COMMAND"
