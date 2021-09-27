#!/usr/bin/env python

from frovedis.exrpc.server import FrovedisServer
from frovedis.matrix.crs import FrovedisCRSMatrix
from frovedis.matrix.dvector import FrovedisDoubleDvector
from frovedis.mllib.tree import DecisionTreeClassifier
from frovedis.mllib.tree import DecisionTreeRegressor
import sys
import numpy as np
import pandas as pd

#Objective: Test when criterion is in lower case 

# initializing the Frovedis server
argvs = sys.argv
argc = len(argvs)
if (argc < 2):
    print 'Please give frovedis_server calling command as the first argument \n(e.g. "mpirun -np 2 /opt/nec/frovedis/ve/bin/frovedis_server")'
    quit()
FrovedisServer.initialize(argvs[1])

mat = pd.DataFrame([[1.0, 0.0, 1.0, 0.0, 0.0, 1.0, 0.0],
                    [0.0, 1.0, 0.0, 1.0, 0.0, 1.0, 0.0],
                    [0.0, 1.0, 0.0, 0.0, 1.0, 0.0, 1.0],
                    [1.0, 0.0, 0.0, 1.0, 0.0, 1.0, 0.0]])
lbl = np.array([0.0, 1.0, 1.0, 0.0])
lbl1 = np.array([0.2,0.3,0.8,0.6])
c_temp = 0
r_temp = 0

# fitting input matrix and label on DecisionTree Classifier object
dtc1 = DecisionTreeClassifier(criterion='gini', splitter='best', max_depth=None,
                 min_samples_split=2, min_samples_leaf=1, min_weight_fraction_leaf=0.0,
                 max_features=None, random_state=None, max_leaf_nodes=1,
                 min_impurity_decrease=0.0,
                 class_weight=None, presort=False, verbose = 0)
try:
  dtc = dtc1.fit(mat,lbl)
except ValueError, e:
  c_temp = 1


# fitting input matrix and label on DecisionTree Regressor object
dtr1 = DecisionTreeRegressor(criterion='mse', splitter='best',
                 max_depth=None, min_samples_split=2, min_samples_leaf=1,
                 min_weight_fraction_leaf=0.0, max_features=None, random_state=None,
                 max_leaf_nodes=1, min_impurity_decrease=0.0, min_impurity_split=None,
                 class_weight=None, presort=False, verbose = 0)
try:
  dtr = dtr1.fit(mat,lbl1)
except ValueError, e:
  r_temp = 1

if c_temp == 1 and r_temp == 1:
  print("Status : Failed")
else:
  print("Status : Passed")

#clean-up
dtc.release()
dtr.release()
FrovedisServer.shut_down()

