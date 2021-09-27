#!/usr/bin/env python

import sys
import numpy as np
np.set_printoptions(threshold=5)

#from sklearn.svm import LinearSVC
from frovedis.mllib.svm import LinearSVC

# initializing the Frovedis server
argvs = sys.argv
argc = len(argvs)
if (argc < 2):
    print ('Please give frovedis_server calling command as the first argument \n(e.g. "mpirun -np 2 /opt/nec/frovedis/ve/bin/frovedis_server")')
    quit()

from frovedis.exrpc.server import FrovedisServer
FrovedisServer.initialize(argvs[1])

# classification data
from sklearn.datasets import load_breast_cancer
mat, lbl = load_breast_cancer(return_X_y=True)

# fitting input matrix and label on linear svm object
svm = LinearSVC(solver='sag').fit(mat,lbl)

# predicting on loaded model
print("predicting on sgd svm classification model: ")
print(svm.predict(mat))
print("score: %.2f " % (svm.score(mat, lbl)))

# saving the model
svm.save("./out/SVMModel")

# loading the same model
svm.load("./out/SVMModel")

# debug_print()
svm.debug_print()
svm.release()

FrovedisServer.shut_down()
