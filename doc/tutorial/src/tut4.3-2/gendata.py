from sklearn import datasets
import numpy

n_samples = 100
centers = [[1, 1], [-1, -1], [1, -1]]
X, labels = datasets.make_blobs(n_samples=n_samples, centers=centers, cluster_std=0.4, random_state=0)
numpy.savetxt("./train.mat", X, fmt="%.3f", delimiter=" ")
numpy.savetxt("./train.label", labels, fmt="%.0f", delimiter=" ")

