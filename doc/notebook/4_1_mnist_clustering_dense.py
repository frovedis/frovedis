"""
Demo for clustering algorithms
"""
import os
import time
from collections import OrderedDict
import pandas as pd
import matplotlib.pyplot as plt
import numpy as np
import tensorflow as tf
import sklearn
from sklearn.cluster import KMeans as skKM
from sklearn import metrics
from sklearn.cluster import AgglomerativeClustering as skAgglomerative
from sklearn.cluster import DBSCAN as skDB
import frovedis
from frovedis.exrpc.server import FrovedisServer
from frovedis.mllib.decomposition import PCA
from frovedis.mllib.cluster import KMeans as frovKM
from frovedis.mllib.cluster import AgglomerativeClustering as frovAgglomerative
from frovedis.mllib.cluster import DBSCAN as frovDB

def preprocess_data():
    '''
    Load the mnist data and perform pre-processing
    '''
    # Load MNIST Digits dataset
    (x_train, y_train), (x_test, y_test) = tf.keras.datasets.mnist.load_data()
    x = np.concatenate((x_train, x_test))
    y = np.concatenate((y_train, y_test))
    x = x.reshape((x.shape[0], -1)).astype('float32')
    x = np.divide(x, 255.)
    return x, y

train_time = []
homogeneity = []
estimator_name = []

def evaluate(estimator, estimator_nm, X, y):
    '''
    To generate performance report for both frovedis and sklearn estimators
    '''
    estimator_name.append(estimator_nm)

    start_time = time.time()
    y_pred = estimator.fit_predict(X)
    train_time.append(round(time.time() - start_time, 4))

    homogeneity.append(metrics.homogeneity_score(y.ravel(), y_pred))

#---- Data Loading and Preparation ----

X, y = preprocess_data()
n_clusters = len(np.unique(y))
print("Dataset contains {} samples".format(X.shape))
print("Dataset contains {} classes".format(n_clusters))

# Label distribution summary
val, count = np.unique(y, return_counts=True)
label = np.asarray((val, count)).T
labels_df = pd.DataFrame(data=label)
ax = labels_df[1].plot(kind='bar', title='Label Distribution').\
     set(xlabel="Digit", ylabel="Count of digit images")

plt.imshow(X[0].reshape(28, 28), cmap = 'binary')
plt.title('Digit Label = {}'.format(y[0]))
plt.show()
plt.imshow(X[1000].reshape(28, 28), cmap = 'magma')
plt.title('Digit Label = {}'.format(y[1000]))
plt.show()

#1. Demo: KMeans Clustering
TARGET = "kmeans"
FrovedisServer.initialize("mpirun -np 8 " + os.environ["FROVEDIS_SERVER"])
f_est = frovKM(n_clusters=n_clusters, init='random', algorithm='full', n_init=1)
E_NM = TARGET + "_frovedis_" + frovedis.__version__
evaluate(f_est, E_NM, X, y)
f_est.release()
FrovedisServer.shut_down()

s_est = skKM(n_clusters=n_clusters, init='random', algorithm='full', n_init=1)
E_NM = TARGET + "_sklearn_" + sklearn.__version__
evaluate(s_est, E_NM, X, y)

#2. Demo: Agglomerative Clustering

TARGET = "agglomerative"
FrovedisServer.initialize("mpirun -np 8 " + os.environ["FROVEDIS_SERVER"] )

# for Agglomerative performed PCA decomposition
x_pca = PCA(n_components=100).fit_transform(X)

f_est = frovAgglomerative(n_clusters=n_clusters, linkage="average")
E_NM = TARGET + "_frovedis_" + frovedis.__version__
evaluate(f_est, E_NM, x_pca, y)
f_est.release()
FrovedisServer.shut_down()

s_est = skAgglomerative(n_clusters=n_clusters, linkage="average")
E_NM = TARGET + "_sklearn_" + sklearn.__version__
evaluate(s_est, E_NM, x_pca, y)

#3. Demo: DBSCAN Clustering
TARGET = "dbscan"
FrovedisServer.initialize("mpirun -np 8 " + os.environ["FROVEDIS_SERVER"] )
f_est = frovDB(eps=4.52, metric="euclidean", algorithm="brute")
E_NM = TARGET + "_frovedis_" + frovedis.__version__

# we are evaluating DBSCAN with first 50k samples due to memeory issue
# at Frovedis Server when trying with full data

evaluate(f_est, E_NM, X[:50000,:], y[:50000])
f_est.release()
FrovedisServer.shut_down()

s_est = skDB(eps=4.52, metric="euclidean", algorithm="brute", n_jobs=12)
E_NM = TARGET + "_sklearn_" + sklearn.__version__
evaluate(s_est, E_NM, X[:50000,:], y[:50000])

# ---- evaluation summary ----
summary = pd.DataFrame(OrderedDict({ "estimator": estimator_name,
                                     "train time": train_time,
                                     "homogeneity_score": homogeneity
                                  }))
print(summary)
