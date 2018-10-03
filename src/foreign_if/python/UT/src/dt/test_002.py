#!/usr/bin/env python

from frovedis.exrpc.server import FrovedisServer
from frovedis.mllib.tree import DecisionTreeClassifier
from frovedis.mllib.tree import DecisionTreeRegressor
import sys
import numpy as np
import pandas as pd

#Objective: Test when max_depth is None

# initializing the Frovedis server
argvs = sys.argv
argc = len(argvs)
if (argc < 2):
    print 'Please give frovedis_server calling command as the first argument \n(e.g. "mpirun -np 2 -x /opt/nec/nosupport/frovedis/ve/bin/frovedis_server")'
    quit()
FrovedisServer.initialize(argvs[1])

mat = pd.DataFrame([[1.0, 0.0, 1.0, 0.0, 0.0, 1.0, 0.0],
                    [0.0, 1.0, 0.0, 1.0, 0.0, 1.0, 0.0],
                    [0.0, 1.0, 0.0, 0.0, 1.0, 0.0, 1.0],
                    [1.0, 0.0, 0.0, 1.0, 0.0, 1.0, 0.0]])
lbl = np.array([0.0, 1.0, 1.0, 0.0])

# fitting input matrix and label on DecisionTree Classifier object
dtc1 = DecisionTreeClassifier(max_depth=None)
dtc = dtc1.fit(mat,lbl)
dtc.debug_print()

# predicting on train model
print("predicting on DecisionTree classifier model: ")
dtcm = dtc.predict(mat)
print dtcm
print("Accuracy score for predicted DecisionTree Classifier model")
print dtc.score(mat,lbl)

# fitting input matrix and label on DecisionTree Regressor object
dtr1 = DecisionTreeRegressor(max_depth=None)
lbl1 = np.array([1.2,0.3,1.1,1.9])
dtr = dtr1.fit(mat,lbl1)
dtr.debug_print()

# predicting on train model
print("predicting on  DecisionTree Regressor model: ")
dtrm = dtr.predict(mat)
print dtrm
print("Root mean square for predicted DecisionTree Regressor model")
print dtr.score(mat,lbl1)

if (lbl== dtcm).all() and (dtrm ==  lbl1).all(): print("Status: Passed")
else: print("Status: Failed")

#clean-up
dtc.release()
dtr.release()
FrovedisServer.shut_down()
