#!/usr/bin/env python

import sys
import numpy as np
from sklearn.datasets import load_boston

from frovedis.exrpc.server import FrovedisServer
from frovedis.mllib.svm import LinearSVR as SVR
#from sklearn.linear_model import SGDRegressor as SVR

# initializing the Frovedis server
argvs = sys.argv
argc = len(argvs)
if (argc < 2):
    print ('Please give frovedis_server calling command as the first argument \n(e.g. "mpirun -np 2 -x /opt/nec/nosupport/frovedis/ve/bin/frovedis_server")')
    quit()
FrovedisServer.initialize(argvs[1])

mat, lbl = load_boston(return_X_y=True)

# fitting input matrix and label on svm regression object
svr = SVR(loss='epsilon_insensitive', epsilon = 0.1, 
          max_iter=1000, penalty='l2').fit(mat,lbl)

# predicting on loaded model
print("predicting on trained model: ")
print(svr.predict(mat[:10]))
print("score: %.2f" % svr.score(mat[:10], lbl[:10]))

# saving the model
svr.save("./out/SVRModel")

# loading the same model
svr.load("./out/SVRModel")

# debug_print
svr.debug_print()
svr.release()

FrovedisServer.shut_down()
