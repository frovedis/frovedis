#!/usr/bin/env python

import sys
import numpy as np
import pandas as pd
from frovedis.matrix.dense import FrovedisRowmajorMatrix, FrovedisColmajorMatrix
from frovedis.matrix.crs import FrovedisCRSMatrix
from frovedis.exrpc.server import FrovedisServer
from frovedis.mllib.manifold import TSNE
from scipy.sparse import csr_matrix, csc_matrix, coo_matrix, bsr_matrix

# initializing the Frovedis server
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
                #dtype=np.float32)
                dtype=np.float64)
                #dtype=np.int32)
                #dtype=np.int64)
#mat = csr_matrix(mat)
#mat = csc_matrix(mat)
#mat = coo_matrix(mat)
#mat = FrovedisRowmajorMatrix(mat)
#mat = FrovedisColmajorMatrix(mat)  #Should raise an error
#mat = FrovedisCRSMatrix(mat)
#mat = pd.DataFrame(mat)
#mat = [[0, 0, 0, 0], [0, 1, 1, 1], [1, 0, 1, 0], [1, 1, 1, 0], [1, 1, 1, 1]]
#mat = ((0, 0, 0, 0), (0, 1, 1, 1), (1, 0, 1, 0), (1, 1, 1, 0), (1, 1, 1, 1))

print("input matrix: ")
print(mat) # for numpy matrix
#print(mat.debug_print()) # for row major matrix

tsne = TSNE().fit(mat)
print("embeddings_: ")
print(tsne.embedding_)
#print(tsne.embedding_.debug_print()) #for row major matrix

print("n_iter_: ")
print(tsne.n_iter_) 

print("kl_divergence_: ")
print(tsne.kl_divergence_)

# releasing results from server
#tsne.embedding_.release() #if mat is FrovedisRowmajorMatrix

# sample numpy dense data (3x3)
mat1 = np.matrix([[0, 0, 0, 0],
                  [0, 1, 1, 1],
                  [1, 0, 1, 0],
                  [1, 1, 1, 0],
                  [1, 1, 1, 1]],
                 #dtype=np.float32)
                 dtype=np.float64)

#mat = FrovedisRowmajorMatrix(mat) # for FrovedisRowmajorMatrix

print("input matrix: ")
print(mat1) # for numpy matrix
#print(mat.debug_print()) # for FrovedisRowmajorMatrix

tsne1 = TSNE()
embeddings = tsne1.fit_transform(mat1)

print("embeddings_: ")
print(embeddings) # for numpy matrix
#print(embeddings.debug_print()) for FrovedisRowmajorMatrix

# releasing results from server
#tsne1.embedding_.release() #if mat is FrovedisRowmajorMatrix

FrovedisServer.shut_down()
