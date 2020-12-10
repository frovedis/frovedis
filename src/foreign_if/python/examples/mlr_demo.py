#!/usr/bin/env python

import sys
import numpy as np
from frovedis.exrpc.server import FrovedisServer
from frovedis.matrix.dense import FrovedisRowmajorMatrix
from frovedis.mllib.linear_model import LogisticRegression 

# initializing the Frovedis server
argvs = sys.argv
argc = len(argvs)
if (argc < 2):
    print ('Please give frovedis_server calling command as the first argument \n(e.g. "mpirun -np 2 -x /opt/nec/nosupport/frovedis/ve/bin/frovedis_server")')
    quit()
FrovedisServer.initialize(argvs[1])

mat = np.loadtxt("./input/iris_data")
lbl = np.loadtxt("./input/iris_lbl")

# fitting input matrix and label on logistic regression object
mlr = LogisticRegression(multi_class='auto',
                         max_iter = 100, 
                         penalty='none').fit(mat,lbl)
mlr.debug_print();

# predicting on loaded model
print("predicting on mlr model: ")
mat1 = FrovedisRowmajorMatrix(mat)
pred = mlr.predict(mat1)
print(pred)

# saving the model
mlr.save("./out/MLRModel")

# loading the same model
mlr.load("./out/MLRModel")

# debug_print
mlr.debug_print()
mlr.release()

FrovedisServer.shut_down()
