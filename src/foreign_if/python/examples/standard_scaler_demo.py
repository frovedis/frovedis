#!/usr/bin/env python

import sys
import numpy as np
from frovedis.exrpc.server import FrovedisServer
from frovedis.mllib.preprocessing import StandardScaler
from frovedis.matrix.crs import FrovedisCRSMatrix
from scipy.sparse import csr_matrix
from scipy import sparse

# initializing the Frovedis server
argvs = sys.argv
argc = len(argvs)
if (argc < 2):
    print ('Please give frovedis_server calling command as the first argument \n(e.g. "mpirun -np 2 /opt/nec/frovedis/ve/bin/frovedis_server")')
    quit()
FrovedisServer.initialize(argvs[1])

#dense data
train_mat = np.loadtxt("./input/kmeans_data.txt")

#sparse data
#train_mat = csr_matrix(train_mat)
  
# creating Standard Scaler object
ss = StandardScaler(with_mean=True, with_std=True)

# fitting the training matrix on Standard Scaler object
ss.fit(train_mat)

#attributes
print("Mean: ")
print(ss.mean_)
print("Var: ")
print(ss.var_)

#transform
trans_mat = ss.transform(train_mat)
print("transformed data::")
print(trans_mat)

#inverse transform
inverse_trans_mat = ss.inverse_transform(trans_mat)
print("inversed data")
print(inverse_trans_mat)

ss.release()
FrovedisServer.shut_down()
