#!/usr/bin/env python

import sys
import numpy as np
from math import sqrt
from sklearn.metrics import mean_squared_error
from frovedis.exrpc.server import FrovedisServer
from frovedis.matrix.crs import FrovedisCRSMatrix
from frovedis.matrix.dvector import FrovedisDoubleDvector
from frovedis.mllib.fm import FactorizationMachineRegressor

# initializing the Frovedis server
argvs = sys.argv
argc = len(argvs)
if (argc < 2):
    print ('Please give frovedis_server calling command as the first argument \n')
    quit()
FrovedisServer.initialize(argvs[1])

mat = FrovedisCRSMatrix(dtype=np.float64).load("./input/regression.txt")

lbl = FrovedisDoubleDvector([3,4,3,3,4,1,5,2,5,5])
# fitting input matrix and label on Factorization Machine Classifier object

fm_obj = FactorizationMachineRegressor(iteration = 100, init_stdev = 0.1,
               init_learn_rate = 0.1, optimizer="SGD", dim = (True, True, 8),
               reg = (0., 0., 0), batch_size_pernode = 1, verbose = 0)
# prediction on created model
model = fm_obj.fit(mat, lbl)

pr_res = model.predict(mat)

# saving the model
model.save("./out/FMCModel1")

model.release()
print (pr_res)
check = np.asarray([3,4,3,3,4,1,5,2,5,5], dtype=np.float64)
print check
rms = sqrt(mean_squared_error(pr_res, check))
print rms
FrovedisServer.shut_down()
out = np.in1d(pr_res, check)
if rms < 2.00:
  print("Passed") 
else:
  print("Failed") 

