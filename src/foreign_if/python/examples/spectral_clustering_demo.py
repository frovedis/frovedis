#!/usr/bin/env python

import sys
import numpy as np
from frovedis.exrpc.server import FrovedisServer
from frovedis.matrix.dense import FrovedisRowmajorMatrix
from frovedis.mllib.cluster import SpectralClustering

# initializing the Frovedis server
argvs = sys.argv
argc = len(argvs)
if (argc < 2):
    print ('Please give frovedis_server calling command as the first argument \n(e.g. "mpirun -np 2 -x /opt/nec/nosupport/frovedis/ve/bin/frovedis_server")')
    quit()
FrovedisServer.initialize(argvs[1])

train_mat = FrovedisRowmajorMatrix(dtype=np.float64).load("./input/spectral_data.txt")

# creating spectral clustering object
spec = SpectralClustering(n_clusters=2, verbose=0)

# fitting the training matrix on spectral clustering object
spec.fit(train_mat)
spec.debug_print()

# extracting the affinity matrix from created model
aff = spec.affinity_matrix_
#aff.debug_print()

# fitting the precomputed affinity matrix for new model creation
spec.affinity = 'precomputed'
spec.fit(aff)
spec.debug_print()

# saving the trained model
spec.save("./out/MySpecClusteringModel")
aff.release()
spec.release()

# loading the same trained model
spec.load("./out/MySpecClusteringModel",dtype=np.float64)
spec.debug_print()

spec.release()
FrovedisServer.shut_down()
