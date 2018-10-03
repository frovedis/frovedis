#!/usr/bin/env python

from frovedis.exrpc.server import FrovedisServer
from frovedis.mllib.tree import DecisionTreeClassifier
from frovedis.mllib.tree import DecisionTreeRegressor
import sys
import numpy as np
import pandas as pd

# Objective : When data contains special characters

# initializing the Frovedis server
argvs = sys.argv
argc = len(argvs)
if (argc < 2):
    print 'Please give frovedis_server calling command as the first argument \n(e.g. "mpirun -np 2 -x /opt/nec/nosupport/frovedis/ve/bin/frovedis_server")'
    quit()
FrovedisServer.initialize(argvs[1])

mat = pd.read_csv("./input/train_1.csv")
lbl = np.array([1, 0, 0, 1],dtype=np.float64)
lbl1 = np.array([0.2,0.3,0.8,0.6])
c_temp = 0
r_temp = 0

# fitting input matrix and label on DecisionTree Classifier object
dtc1 = DecisionTreeClassifier()
try:
  dtc = dtc1.fit(mat,lbl)
except TypeError, e:
  c_temp = 1

# fitting input matrix and label on DecisionTree Regressor object
dtr1 = DecisionTreeRegressor()
try:
  dtr = dtr1.fit(mat,lbl1)
except TypeError, e:
  r_temp = 1

if c_temp == 1 and r_temp == 1:
  print("Status : Passed")
else:
  print("Status : Failed")

FrovedisServer.shut_down()

