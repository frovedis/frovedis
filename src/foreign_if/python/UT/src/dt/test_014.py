#!/usr/bin/env python

from frovedis.exrpc.server import FrovedisServer
from frovedis.matrix.crs import FrovedisCRSMatrix
from frovedis.matrix.dvector import FrovedisDoubleDvector
from frovedis.mllib.tree import DecisionTreeClassifier
from frovedis.mllib.tree import DecisionTreeRegressor
import sys
import numpy as np
import pandas as pd
import os

#Objective: Run without error for predict over a loaded model

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

# fitting input matrix and label on DecisionTree Classifier object
dtc1 = DecisionTreeClassifier(criterion='gini', splitter='best', max_depth=None,
                 min_samples_split=2, min_samples_leaf=1, min_weight_fraction_leaf=0.0,
                 max_features=None, random_state=None, max_leaf_nodes=1,
                 min_impurity_decrease=0.0,
                 class_weight=None, presort=False, verbose = 0)
dtc = dtc1.fit(mat,lbl)
dtc.debug_print()

# predicting on loaded model
print("predicting on DecisionTree classifier model: ")
dtcm = dtc.predict(mat)
print dtcm
print("Accuracy score for predicted DecisionTree Classifier model")
print dtc.score(mat,lbl)

# saving the model
os.system("rm -rf ./out/DTMModel") # deleting model, if any
dtc.save("./out/DTMModel")

# loading the same model
dtcl = dtc.load("./out/DTMModel").predict(mat)


# fitting input matrix and label on DecisionTree Regressor object
dtr1 = DecisionTreeRegressor(criterion='mse', splitter='best',
                 max_depth=None, min_samples_split=2, min_samples_leaf=1,
                 min_weight_fraction_leaf=0.0, max_features=None, random_state=None,
                 max_leaf_nodes=1, min_impurity_decrease=0.0, min_impurity_split=None,
                 class_weight=None, presort=False, verbose = 0)
lbl1 = lbl1 = np.array([1.2,0.3,1.1,1.9])
dtr = dtr1.fit(mat,lbl1)
dtr.debug_print()

# predicting on loaded model
print("predicting on  DecisionTree Regressor model: ")
dtrm =  dtr.predict(mat)
print dtrm
print("Root mean square for predicted DecisionTree Regressor model")
print dtr.score(mat,lbl1)

# saving the model
os.system("rm -rf ./out/DTRModel") # deleting model, if any
dtr.save("./out/DTRModel")

# loading the same model
dtrl = dtr.load("./out/DTRModel").predict(mat)

if (dtcl == dtcm).all() and (dtrl ==  dtrm).all():
  print("Status: Passed")
else:
  print("Status: Failed")

#clean-up
os.system("rm -rf ./out/DTRModel") 
os.system("rm -rf ./out/DTMModel") 
dtc.release()
dtr.release()
FrovedisServer.shut_down()
