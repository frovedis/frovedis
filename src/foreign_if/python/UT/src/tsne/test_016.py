#!/usr/bin/env python

import sys
import numpy as np
from frovedis.exrpc.server import FrovedisServer
from frovedis.matrix.dense import FrovedisRowmajorMatrix
from frovedis.mllib.manifold import TSNE

desc = "Testing fit_transform for FrovedisRowmajorMatrix : "

# initializing the frovedis server
argvs = sys.argv
argc = len(argvs)
if (argc < 2):
    print ('Please give frovedis_server calling command as the first argument\n\
           (e.g. "mpirun -np 2 -x /opt/nec/nosupport/frovedis/ve/bin/frovedis_server")')
    quit()
FrovedisServer.initialize(argvs[1])

# sample numpy dense data (3x3)
mat = np.matrix([[0, 0, 0, 0], 
                 [0, 1, 1, 1], 
                 [1, 0, 1, 0],
                 [1, 1, 1, 0],
                 [1, 1, 1, 1]], 
                dtype=np.float64)
mat = FrovedisRowmajorMatrix(mat)
tsne = TSNE(n_components=2)

try:
    emb = tsne.fit_transform(mat)
    if (type(emb).__name__ == "FrovedisRowmajorMatrix"):
        print(desc, "Passed")
        emb.release()
    else:
        print(desc, "Failed")
except:
    print(desc, "Failed")

FrovedisServer.shut_down()

