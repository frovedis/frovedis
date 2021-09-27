# Test frovedis niters and sklearn niters
import sys
import numpy as np
from frovedis.exrpc.server import FrovedisServer
from frovedis.matrix.dense import FrovedisRowmajorMatrix
from frovedis.mllib.gmm import GaussianMixture
import sklearn.mixture as sk


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
    f_model = GaussianMixture(n_components=n_components)
    # fitting the training matrix on gaussian mixture object
    f_model.fit(train_mat)
    f_niter = f_model.n_iter_
except Exception as e:
    print ("status=Exception: " + str(e))
    sys.exit(1)

try:
    sk_model = sk.GaussianMixture(n_components=n_components, random_state=0).fit(train_mat)
    s_niter = sk_model.n_iter_
except Exception as e:
    print ("status=Exception: " + str(e))
    sys.exit(1)    
    

if(f_niter == s_niter):
    print("status=Passed")
else:
    print("status=Failed")