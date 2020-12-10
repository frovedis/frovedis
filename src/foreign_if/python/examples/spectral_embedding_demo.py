#!/usr/bin/env python

import sys
import numpy as np
from frovedis.exrpc.server import FrovedisServer
from frovedis.matrix.dense import FrovedisRowmajorMatrix
from frovedis.mllib.cluster import SpectralEmbedding, KMeans

# initializing the Frovedis server
argvs = sys.argv
argc = len(argvs)
if (argc < 2):
    print ('Please give frovedis_server calling command as the first argument \n(e.g. "mpirun -np 2 -x /opt/nec/nosupport/frovedis/ve/bin/frovedis_server")')
    quit()
FrovedisServer.initialize(argvs[1])

train_mat = FrovedisRowmajorMatrix(dtype=np.float64).load("./input/spectral_data.txt")

# creating spectral embedding object
sem = SpectralEmbedding(n_components=2, verbose=0, drop_first=False)

# fitting the training matrix on spectral embedding object
sem.fit(train_mat)
sem.debug_print()

#extracting the embedding matrix from created model
embed = sem.get_embedding_matrix()
#embed.debug_print()

#calling kmeans on spectral embed to assign cluster
kmeans = KMeans(n_clusters=2, max_iter=100)
print("label: ", kmeans.fit(embed).predict(embed)) #kind of fit_predict(embed)

# extracting the affinity matrix from created model
aff = sem.get_affinity_matrix()
#aff.debug_print()

# fitting the precomputed affinity matrix for new model creation
sem.affinity = 'precomputed'
sem.fit(aff)
sem.debug_print()

# saving the trained model
sem.save("./out/MySemModel")
sem.release()

# loading the same trained model
sem.load("./out/MySemModel",dtype=np.float64)
sem.debug_print()

embed.release()
aff.release()
sem.release()

FrovedisServer.shut_down()
