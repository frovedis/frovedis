#!/usr/bin/env python

import sys
import numpy as np
from frovedis.exrpc.server import FrovedisServer
from frovedis.matrix.crs import FrovedisCRSMatrix
from frovedis.mllib.fm import FactorizationMachineClassifier
from frovedis.mllib.fm import FactorizationMachineRegressor

# initializing the Frovedis server
argvs = sys.argv
argc = len(argvs)
if (argc < 2):
    print ('Please give frovedis_server calling command as the first argument \n(e.g. "mpirun -np 2 -x /opt/nec/nosupport/frovedis/ve/bin/frovedis_server")')
    quit()
FrovedisServer.initialize(argvs[1])

mat = np.random.randint(5, size=(6, 50))
mat = np.asmatrix(mat, dtype=np.float64)
cmat = FrovedisCRSMatrix(mat, dtype=np.float64)
lbl = np.array([1, 1, -1, 1, -1, 1], dtype=np.float64)

# fitting input matrix and label on Factorization Machine Classifier object
fmc = FactorizationMachineClassifier(iteration = 100, init_stdev = 0.1,
            	 init_learn_rate = 0.01, optimizer="SGD", dim = (True, True, 8),
                 reg = (False, False, 0), batch_size_pernode = 5000, verbose = 0).fit(cmat,lbl)

# prediction on created model
print("predicting on factorization machine classifier model: ")
print (fmc.predict(cmat))

# saving the model
fmc.save("./out/FMCModel")

mat2 = np.random.randint(5, size=(8, 50))
mat2 = np.asmatrix(mat2, dtype=np.float64)
cmat2 = FrovedisCRSMatrix(mat2, dtype=np.float64)
lbl2 = np.array([1.2,0.3,1.1,1.9,1.7,0.5,1.2,1.1],dtype=np.float64)
# fitting input matrix and label on Factorization Machine Regressor object
fmr = FactorizationMachineRegressor(iteration = 100, init_stdev = 0.1,
                 init_learn_rate = 0.01, optimizer="SGD", dim = (True, True, 8),
                 reg = (False, False, 0), batch_size_pernode = 5000, verbose = 0).fit(cmat2,lbl2)

# prediction on created model
print("predicting on factorization machine regressor model: ")
print (fmr.predict(cmat2))

# saving the model
fmr.save("./out/FMRModel")

# clean-up
fmc.release()
fmr.release()
FrovedisServer.shut_down()

