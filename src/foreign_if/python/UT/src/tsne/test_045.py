#!/usr/bin/env python

import sys
import numpy as np
from frovedis.exrpc.server import FrovedisServer
from frovedis.mllib.manifold import TSNE

desc = "Testing for accessing 'kl_divergence_' attribute before calling fit() : "

# initializing the frovedis server
argvs = sys.argv
argc = len(argvs)
if (argc < 2):
    print ('Please give frovedis_server calling command as the first argument\n\
           (e.g. "mpirun -np 2 /opt/nec/frovedis/ve/bin/frovedis_server")')
    quit()
FrovedisServer.initialize(argvs[1])

# sample numpy dense data (3x3)
mat = np.matrix([[0, 0, 0, 0], 
                 [0, 1, 1, 1], 
                 [1, 0, 1, 0],
                 [1, 1, 1, 0],
                 [1, 1, 1, 1]], 
                dtype=np.float64)
tsne = TSNE(n_components=2, method='exact')

try:
    print(tsne.kl_divergence_)
    tsne.fit(mat)
    print(desc, "Failed")
except:
    print(desc, "Passed")

FrovedisServer.shut_down()

