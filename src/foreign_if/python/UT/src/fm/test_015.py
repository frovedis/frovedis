#!/usr/bin/env python

import sys
import numpy as np
from frovedis.exrpc.server import FrovedisServer
from frovedis.matrix.crs import FrovedisCRSMatrix
from frovedis.mllib.fm import FactorizationMachineRegressor
from frovedis.matrix.dvector import FrovedisDoubleDvector

#obj: passing negative value of verbose
# initializing the Frovedis server
argvs = sys.argv
argc = len(argvs)
if (argc < 2):
    print ('Please give frovedis_server calling command as the first argument \n')
    quit()
FrovedisServer.initialize(argvs[1])

mat = FrovedisCRSMatrix(dtype=np.float64).load("./input/classification.txt")
lbl = FrovedisDoubleDvector([3,4,3,3,4,1,5,2,5,5])

# fitting input matrix and label on Factorization Machine Classifier object

fm_obj = FactorizationMachineRegressor(iteration = 10, init_stdev = 0.1,
               init_learn_rate = 0.1, optimizer="SGD", dim = (True, True, 8),
               reg = (0., 0., 0), batch_size_pernode = 1, verbose = -1)


try:
  model = fm_obj.fit(mat, lbl)
  print("Failed")
except:
  print("Passed")

FrovedisServer.shut_down()
