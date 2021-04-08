"""
Demo for TSNE algorithms
"""

import os
import time
import pandas as pd
import numpy as np
import tensorflow as tf
from sklearn import metrics
import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d import Axes3D
import seaborn as sns
from collections import OrderedDict

def preprocess_data():
    # Load MNIST Digits dataset
    (x_train, y_train), (x_test, y_test) = tf.keras.datasets.mnist.load_data()
    x = np.concatenate((x_train, x_test))
    y = np.concatenate((y_train, y_test))
    x = x.reshape((x.shape[0], -1)).astype('float32')
    x = np.divide(x, 255.)
    return x, y

#---- Data Loading and Preparation ----

X, y = preprocess_data()
X = X[:5000]
y = y[:5000]
n_clusters = len(np.unique(y))
print("Dataset contains {} samples".format(X.shape[0]))
print("Dataset contains {} classes".format(n_clusters))

train_time = []
estimator_name = []
cluster_img = []

def get_result_image(Y, labels):
    df = pd.DataFrame({'X':Y[:,0],
                       'Y':Y[:,1],
                       'labels':labels})

    target_ids = range(len(df.labels))

    plt.figure(figsize=(6, 5))
    colors = 'r', 'g', 'b', 'c', 'm', 'y', 'k', 'w', 'orange', 'purple'
    for i, c, label in zip(target_ids, colors, df.labels.unique()):
        plt.scatter(Y[labels == i, 0], Y[labels == i, 1], c=c, label=label) 
    plt.legend()
    plt.show()

def evaluate(estimator, estimator_nm, X, y):
    estimator_name.append(estimator_nm)

    start_time = time.time()
    y_pred = estimator.fit_transform(X)
    train_time.append(round(time.time() - start_time, 4))
    cluster_img.append((y_pred, y))

# 1. Demo : TSNE
from frovedis.exrpc.server import FrovedisServer
FrovedisServer.initialize("mpirun -np 8 -ve 2 " + os.environ["FROVEDIS_SERVER"])
from frovedis.mllib.manifold import TSNE as frovTSNE
f_est = frovTSNE(n_components=2, method="exact")
evaluate(f_est, "frovedis_tsne", X, y)
FrovedisServer.shut_down()

from sklearn.manifold import TSNE as skTSNE
s_est = skTSNE(n_components=2, method="exact")
evaluate(s_est, "sklearn_tsne", X, y)

s_est = skTSNE(n_components=2, method="barnes_hut")
evaluate(s_est, "sklearn_tsne_barnes_hut", X, y)

# ---- evaluation summary ----
summary = pd.DataFrame(OrderedDict({ "estimator": estimator_name,
                                     "train time": train_time, 
                                  }))
print(summary)

print("Frovedis cluster")
get_result_image(*cluster_img[0])
print("Sklearn cluster")
get_result_image(*cluster_img[1])
print("Sklearn Barneshut cluster")
get_result_image(*cluster_img[2])
