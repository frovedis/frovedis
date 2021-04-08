#1. Description

'''
Click though rate prediction using logistic regression.
Please download the data from https://www.kaggle.com/c/avazu-ctr-prediction/
manually (registration required) and place `CTR_train` file in `datasets` directory.
Only part of the data is used, because it takes too large memory for one VE card.

In online advertising, click-through rate (CTR) is a very important metric
for evaluating ad performance.

As a result, click prediction systems are essential and widely used for
sponsored search and real-time bidding.

This data is 11 days worth of Avazu data to build and test prediction models.
'''

#2. Data Preprocessing

from collections import OrderedDict
import os
import time
import seaborn as sns
import sklearn
from sklearn.model_selection import train_test_split
from sklearn import metrics
from sklearn.preprocessing import OneHotEncoder
from sklearn.linear_model import LogisticRegression as skLogisticRegression
from sklearn.svm import LinearSVC as skSVC
import pandas as pd
import frovedis
from frovedis.mllib.linear_model import LogisticRegression as frovLogisticRegression
from frovedis.mllib.svm import LinearSVC as frovSVC
from frovedis.exrpc.server import FrovedisServer

def preprocess_data(fname):
    '''
    For CTR classification we will perform some data preparation and data cleaning steps.
    '''
    #Sample size of 1 million has been taken for execution from CTR dataset
    n_rows =  800000
    df = pd.read_csv(fname, nrows=n_rows)
    class_names = {0:'Not click', 1:'Click'}
    print(df.click.value_counts().rename(index = class_names))
    # We dropped 'click', 'id', 'hour', 'device_id', 'device_ip' from the dataset,
    # which does not contribute the prediction.
    x = df.drop(['click', 'id', 'hour', 'device_id', 'device_ip'], axis=1).values
    y = df['click'].values
    n_rows = df.shape[0]
    x_train, x_test, y_train, y_test = train_test_split(x,y, test_size = 0.05)
    # Other features are one-hot encoded; so the feature matrix becomes sparse matrix
    enc = OneHotEncoder(handle_unknown='ignore')
    x_train_enc = enc.fit_transform(x_train)
    x_test_enc = enc.transform(x_test)
    return x_train_enc, x_test_enc, y_train, y_test

#---- Data Preparation ----

DATA_FILE = "datasets/ctr_train.csv"
x_train, x_test, y_train, y_test = preprocess_data(DATA_FILE)
print("shape of train data: {}".format(x_train.shape))
print("shape of test data: {}".format(x_test.shape))

sns.countplot(y_train)

#3. Algorithm Evaluation

train_time = []
test_time = []
accuracy = []
precision = []
recall = []
f1 = []
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
    pred_y = estimator.predict(x_test)
    test_time.append(round(time.time() - start_time, 4))
    accuracy.append(metrics.accuracy_score(y_test, pred_y))
    precision.append(metrics.precision_score(y_test, pred_y))
    recall.append(metrics.recall_score(y_test, pred_y))
    f1.append(metrics.f1_score(y_test, pred_y))
    return metrics.classification_report(y_test, pred_y)

#3.1 Binary LogisticRegression with sag solver

TARGET = "binary_logistic_regression_sag"
FrovedisServer.initialize("mpirun -np 8 " + os.environ["FROVEDIS_SERVER"])
f_est = frovLogisticRegression(penalty='l2', solver='sag')
E_NM = TARGET + "_frovedis_" + frovedis.__version__
f_report = evaluate(f_est, E_NM, x_train, y_train, x_test, y_test)
f_est.release()
FrovedisServer.shut_down()

s_est = skLogisticRegression(penalty='l2', solver='sag')
E_NM = TARGET + "_sklearn_" + sklearn.__version__
s_report = evaluate(s_est, E_NM, x_train, y_train, x_test, y_test)

print("Frovedis LogisticRegression matrices: ")
print(f_report)
print("Sklearn LogisticRegression matrices: ")
print(s_report)


#3.2 Linear SVC
TARGET = "Linear_SVC"
FrovedisServer.initialize("mpirun -np 8 " + os.environ["FROVEDIS_SERVER"])
f_est = frovSVC(loss='hinge', max_iter=10000)
E_NM = TARGET + "_frovedis_" + frovedis.__version__
f_report = evaluate(f_est, E_NM, x_train, y_train, x_test, y_test)
f_est.release()
FrovedisServer.shut_down()

s_est = skSVC(loss='hinge', max_iter=10000)
E_NM = TARGET + "_sklearn_" + sklearn.__version__
s_report = evaluate(s_est, E_NM, x_train, y_train, x_test, y_test)

# SVC: Precision, Recall and F1 score for each class
print("Frovedis Linear SVC metrices: ")
print(f_report)
print("Sklearn Linear SVC metrices: ")
print(s_report)

#4. Performance summary

# ---- evaluation summary ----
summary = pd.DataFrame(OrderedDict({ "estimator": estimator_name,
                                     "train time": train_time,
                                     "test time": test_time,
                                     "accuracy": accuracy,
                                     "precision": precision,
                                     "recall": recall,
                                     "f1-score": f1
                                  }))
print(summary)
