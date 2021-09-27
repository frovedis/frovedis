# Test weights shape
import sys
import numpy as np
from frovedis.exrpc.server import FrovedisServer
from frovedis.matrix.dense import FrovedisRowmajorMatrix
from frovedis.mllib.gmm import GaussianMixture


# initializing the Frovedis server
argvs = sys.argv
argc = len(argvs)
if (argc < 2):
    print ('Please give frovedis_server calling command as the first argument \n(e.g. "mpirun -np 2 /opt/nec/frovedis/ve/bin/frovedis_server")')
    quit()
FrovedisServer.initialize(argvs[1])

train_mat = np.loadtxt("./input/gmm_data.txt")

# creating spectral agglomerative object
n_components = 2

try:
    gmm_model = GaussianMixture(n_components=n_components)
    # fitting the training matrix on gaussian mixture object
    gmm_model.fit(train_mat)
    weights = gmm_model.weights_
except Exception as e:
    print ("status=Exception: " + str(e))
    sys.exit(1)
    

if(len(weights) == n_components):
    print("status=Passed")
else:
    print("status=Failed")
    


