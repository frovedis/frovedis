#Regression (Sparse)

#Dataset - E2006-tfidf

#1. Description
'''
Financial10-K reports from thousands of publicly traded U.S. companies,
published in 1996–2006 and stock return volatility measurements in the
twelve-month period before and the twelve-month period after each report.

Here the target variable (y) is the risk associated with buying a particular stock.
And the features (X) are the volatility of the stock price return over different periods of time.

Download Link:
Train Data: https://www.csie.ntu.edu.tw/~cjlin/libsvmtools/datasets/regression/E2006.train.bz2 <br>
Test Data: https://www.csie.ntu.edu.tw/~cjlin/libsvmtools/datasets/regression/E2006.test.bz2


Data source:
1. http://www.cs.cmu.edu/~ark/10K/
2. http://www.cs.cmu.edu/~nasmith/papers/kogan+levin+routledge+sagi+smith.naacl09.pdf
3. https://www.csie.ntu.edu.tw/~cjlin/libsvmtools/datasets/
'''

#2. Data Preprocessing <font>

import os
import time
from collections import OrderedDict
import numpy as np
import pandas as pd
import sklearn
from sklearn.linear_model import Lasso as sLR
from sklearn import datasets
import frovedis
from frovedis.exrpc.server import FrovedisServer
from frovedis.mllib.linear_model import Lasso as fLR


def preprocess_data(filename_train, filename_test, feature_dim):
    '''
    For E2006-tfidf regression we will perform some data preparation and data cleaning steps.
    '''
    x_train, y_train = datasets.load_svmlight_file(
        filename_train,
        n_features=feature_dim,
        dtype=np.float32)
    x_test, y_test = datasets.load_svmlight_file(
        filename_test,
        n_features=feature_dim,
        dtype=np.float32)
    return x_train, y_train, x_test, y_test


#---- Data Preparation ----
FILENAME_TRAIN = "datasets/E2006.train"
FILENAME_TEST = "datasets/E2006.test"
FEATURE_DIM = 150360
x_train, y_train, x_test, y_test = preprocess_data(FILENAME_TRAIN, FILENAME_TEST, FEATURE_DIM)
print("shape of train data: {}".format(x_train.shape))
print("shape of test data: {}".format(x_test.shape))


#3. Algorithm Evaluation

train_time = []
test_time = []
train_score = []
test_score = []
estimator_name = []

def evaluate(estimator, estimator_nm,
             x_train, y_train,
             x_test, y_test):
    '''
    To generate performance report for both frovedis and sklearn estimators
    '''
    estimator_name.append(estimator_nm)

    start_time = time.time()
    estimator.fit(x_train, y_train)
    train_time.append(round(time.time() - start_time, 4))

    start_time = time.time()
    train_score.append(estimator.score(x_train, y_train))
    test_score.append(estimator.score(x_test, y_test))
    test_time.append(round(time.time() - start_time, 4))


#3.1 LassoRegressor

TARGET = "lasso_regressor"
FrovedisServer.initialize("mpirun -np 8 " +  os.environ["FROVEDIS_SERVER"])
f_est = fLR(lr_rate=4.89E-05)
E_NM = TARGET + "_frovedis_" + frovedis.__version__
evaluate(f_est, E_NM, x_train, y_train, x_test, y_test)
f_est.release()
FrovedisServer.shut_down()

s_est = sLR(alpha=0.01)
E_NM = TARGET + "_sklearn_" + sklearn.__version__
evaluate(s_est, E_NM, x_train, y_train, x_test, y_test)


#4. Performance Summary <font>

summary = pd.DataFrame(OrderedDict({ "estimator": estimator_name,
                                     "train time": train_time,
                                     "test time": test_time,
                                     "train-score": train_score,
                                     "test-score": test_score
                                  }))
print(summary)
