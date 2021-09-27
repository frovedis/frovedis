#!/usr/bin/env python

import sys
import numpy as np
from frovedis.matrix.dense import FrovedisRowmajorMatrix
from frovedis.exrpc.server import FrovedisServer
from frovedis.mllib.decomposition import PCA
#from sklearn.decomposition import PCA

# initializing the Frovedis server
argvs = sys.argv
argc = len(argvs)
if (argc < 2):
    print ('Please give frovedis_server calling command as the first argument \n(e.g. "mpirun -np 2 /opt/nec/frovedis/ve/bin/frovedis_server")')
    quit()
FrovedisServer.initialize(argvs[1])

# sample numpy dense data (3x3)
mat = np.matrix([[1.0, 0.0, 7.0, 0.0, 0.0], 
                 [2.0, 0.0, 3.0, 4.0, 5.0], 
                 [4.0, 0.0, 0.0, 6.0, 7.0]], 
                #dtype=np.float32)
                dtype=np.float64)

#pca = PCA(n_components=2, svd_solver='arpack', copy=True)
pca = PCA(n_components=2, svd_solver='arpack', whiten=True)

print("input matrix: ")
print(mat) # for numpy matrix
# #mat = FrovedisRowmajorMatrix(mat)
# #print(mat.to_numpy_matrix()) # for frovedis rowmajor matrix

pca.fit(mat)  
score = pca.fit_transform(mat)

print("Mean: ")
print(pca.mean_)

print("input matrix after fit: ")
print(mat) # for numpy matrix
#print(mat.to_numpy_matrix()) # for frovedis rowmajor matrix

print("score:")
print(score)

print("n_components_: ")
print(pca.n_components_)

print("n_samples_: ")
print(pca.n_samples_)

print("n_features_: ")
print(pca.n_features_)

print("components_: ")
print(pca.components_)

print("explained variance: ")
print(pca.explained_variance_)

print("explained variance_ratio: ")
print(pca.explained_variance_ratio_)

print("singular_values: ")
print(pca.singular_values_)

print("noise variance: ")
print(pca.noise_variance_)

# a = pca.pca_res_.to_numpy_results()
# print("to_numpy_results")
# print(a)
# for debug purpose: showing results on server 
#pca.pca_res_.debug_print()

# releasing results from server
#pca.pca_res_.release()
a = np.array([[1.0, 0.0, 7.0, 0.0, 0.0], 
              [2.0, 0.0, 3.0, 4.0, 5.0], 
              [4.0, 0.0, 0.0, 6.0, 7.0]], 
             #dtype=np.float32)
             dtype=np.float64)

X1 = pca.transform(a)
#X1 = pca.transform(FrovedisRowmajorMatrix(a))
print("transformed data")
print(X1)

X2 = pca.inverse_transform(X1)
print("inverse_transform data")
print(X2)

pca.save("./out/PCAModel")

FrovedisServer.shut_down()
