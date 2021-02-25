#!/usr/bin/env python
'''
   *** Useful Links ***
https://github.com/scikit-learn/scikit-learn/blob/55bf5d9/sklearn/cluster/spectral.py#L275
https://github.com/scikit-learn/scikit-learn/blob/55bf5d9/sklearn/manifold/spectral_embedding_.py#L136
https://github.com/scipy/scipy/blob/v1.2.0/scipy/sparse/csgraph/_laplacian.py#L18-L81
https://docs.scipy.org/doc/scipy/reference/generated/scipy.sparse.linalg.eigsh.html
'''

import numpy as np
from sklearn.cluster import SpectralClustering
from sklearn.manifold import spectral_embedding
from scipy.sparse.csgraph import laplacian
from scipy.sparse.linalg import eigsh
from sklearn.cluster.k_means_ import k_means

X = np.loadtxt("./test_data")
if(X.ndim == 1): X = X.reshape(-1, 1)
clustering = SpectralClustering(n_clusters=2).fit(X)
labels = clustering.labels_
print("[sklearn] labels: ")
print(labels)


'''
 *** intermediate ***

affinity = clustering.affinity_matrix_
print("[sklearn] affinity: ")
print(affinity)

lap_norm = laplacian(affinity, normed=True)
print("[sklearn] normalized_laplace: ")
print(lap_norm)

lambdas, vecs = eigsh(lap_norm, k=2,sigma=1.0,which='LM')
_,l,_ = k_means(vecs,2)
print(l)

maps = spectral_embedding(affinity, n_components=2)
_,l,_ = k_means(maps,2)
print(l)
'''
