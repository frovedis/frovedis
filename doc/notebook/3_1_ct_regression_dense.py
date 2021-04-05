
#Regression (Dense)

#1. Description
'''
Relative location of CT slices on axial axis data set using 53500 CT images from 74 different patients (43 male, 31 female).
Dataset can be downloaded from https://archive.ics.uci.edu/ml/machine-learning-databases/00206/slice_localization_data.zip

Each CT slice is described by two histograms in polar space. The first histogram describes the location of bone structures in the image, the second the location of air inclusions inside of the body.
 
The class variable (relative location of an image on the axial axis) was constructed by manually annotating up to 10 different distinct landmarks in each CT Volume with known location. The location of slices in between landmarks was interpolated.
'''

#2. Data Preprocessing

import os
import time
from collections import OrderedDict
import pandas as pd
import sklearn
from sklearn.linear_model import LinearRegression as sLNR
from sklearn.model_selection import train_test_split
from sklearn.linear_model import SGDRegressor as sSGDReg
from sklearn.neighbors import KNeighborsRegressor as sKNR
from sklearn.linear_model import Ridge as sRR
from sklearn.tree import DecisionTreeRegressor as sDTR
from sklearn.linear_model import Lasso as sLR
from sklearn import svm as sSVMR
import frovedis
from frovedis.exrpc.server import FrovedisServer
from frovedis.mllib.linear_model import LinearRegression as fLNR
from frovedis.mllib.linear_model import SGDRegressor as fSGDReg
from frovedis.mllib.neighbors import KNeighborsRegressor as fKNR
from frovedis.mllib.linear_model import Ridge as fRR
from frovedis.mllib.svm import LinearSVR as fLSVR
from frovedis.mllib.tree import DecisionTreeRegressor as fDTR
from frovedis.mllib.linear_model import Lasso as fLR

def preprocess_data(fname):
    '''
    For CT slices regression we will perform some data preparation and data cleaning steps.
    '''
    df = pd.read_csv(fname)
    df = df.drop(columns=["patientId"])
    x = df.iloc[:,:-1]
    y = df.iloc[:, -1]
    return train_test_split(x, y, test_size=0.20, random_state=42)

#---- Data Preparation ----
# Please uncomment the below lines to download and unzip the dataset.
#!wget -N https://archive.ics.uci.edu/ml/machine-learning-databases/00206/slice_localization_data.zip
#!unzip -o slice_localization_data.zip
#!mv slice_localization_data.csv datasets

DATA_FILE = "datasets/slice_localization_data.csv"
x_train, x_test, y_train, y_test = preprocess_data(DATA_FILE)
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


#3.1 LinearRegression

TARGET = "lnr"
FrovedisServer.initialize("mpirun -np 8 " +  os.environ["FROVEDIS_SERVER"])
f_est = fLNR()
E_NM = TARGET + "_frovedis_" + frovedis.__version__
evaluate(f_est, E_NM, x_train, y_train, x_test, y_test)
f_est.release()
FrovedisServer.shut_down()

s_est = sLNR()
E_NM = TARGET + "_sklearn_" + sklearn.__version__
evaluate(s_est, E_NM, x_train, y_train, x_test, y_test)


#3.2 SGDRegressor

TARGET = "sgd"
FrovedisServer.initialize("mpirun -np 8 " +  os.environ["FROVEDIS_SERVER"])
f_est = fSGDReg(loss="squared_loss", penalty="l2",
                eta0=0.00001)
E_NM = TARGET + "_frovedis_" + frovedis.__version__
evaluate(f_est, E_NM, x_train, y_train, x_test, y_test)
f_est.release()
FrovedisServer.shut_down()

s_est = sSGDReg(loss="squared_loss", penalty="l2",
                eta0=0.00001, random_state=42)
E_NM = TARGET + "_sklearn_" + sklearn.__version__
evaluate(s_est, E_NM, x_train, y_train, x_test, y_test)


#3.3 KNNRegressor

TARGET = "knn"
FrovedisServer.initialize("mpirun -np 8 " +  os.environ["FROVEDIS_SERVER"])
f_est = fKNR(n_neighbors=1000, metric="euclidean",
             algorithm="brute")
E_NM = TARGET + "_frovedis_" + frovedis.__version__
evaluate(f_est, E_NM, x_train, y_train, x_test, y_test)
f_est.release()
FrovedisServer.shut_down()

s_est = sKNR(n_neighbors=1000, metric="euclidean",
             algorithm="brute")
E_NM = TARGET + "_sklearn_" + sklearn.__version__
evaluate(s_est, E_NM, x_train, y_train, x_test, y_test)


#3.4 RidgeRegressor

TARGET = "ridge"
FrovedisServer.initialize("mpirun -np 8 " +  os.environ["FROVEDIS_SERVER"])
f_est = fRR(alpha=0.001, lr_rate=1e-05, max_iter=200)
E_NM = TARGET + "_frovedis_" + frovedis.__version__
evaluate(f_est, E_NM, x_train, y_train, x_test, y_test)
f_est.release()
FrovedisServer.shut_down()

s_est = sRR(alpha=0.001, solver='sag', max_iter=200)
E_NM = TARGET + "_sklearn_" + sklearn.__version__
evaluate(s_est, E_NM, x_train, y_train, x_test, y_test)


#3.5 LinearSupportVectorRegressor

TARGET = "svr"
FrovedisServer.initialize("mpirun -np 8 " +  os.environ["FROVEDIS_SERVER"])
f_est = fLSVR(lr_rate=0.0008)
E_NM = TARGET + "_frovedis_" + frovedis.__version__
evaluate(f_est, E_NM, x_train, y_train, x_test, y_test)
f_est.release()
FrovedisServer.shut_down()

s_est = sSVMR.SVR(epsilon=0.0, max_iter=1000, tol=0.0001)
E_NM = TARGET + "_sklearn_" + sklearn.__version__
evaluate(s_est, E_NM, x_train, y_train, x_test, y_test)


#3.6 DecisionTreeRegressor

TARGET = "dtr"
FrovedisServer.initialize("mpirun -np 8 " +  os.environ["FROVEDIS_SERVER"])
f_est = fDTR()
E_NM = TARGET + "_frovedis_" + frovedis.__version__
evaluate(f_est, E_NM, x_train, y_train, x_test, y_test)
f_est.release()
FrovedisServer.shut_down()

s_est = sDTR(max_depth=5)
E_NM = TARGET + "_sklearn_" + sklearn.__version__
evaluate(s_est, E_NM, x_train, y_train, x_test, y_test)


#3.7 LassoRegressor

TARGET = "lasso"
FrovedisServer.initialize("mpirun -np 8 " +  os.environ["FROVEDIS_SERVER"])
f_est = fLR(alpha=0.001, lr_rate=1e-05, max_iter=800)
E_NM = TARGET + "_frovedis_" + frovedis.__version__
evaluate(f_est, E_NM, x_train, y_train, x_test, y_test)
f_est.release()
FrovedisServer.shut_down()

s_est = sLR(alpha=0.001, max_iter=800)
E_NM = TARGET + "_sklearn_" + sklearn.__version__
evaluate(s_est, E_NM, x_train, y_train, x_test, y_test)


#4. Performance Summary
summary = pd.DataFrame(OrderedDict({ "estimator": estimator_name,
                                     "train time": train_time,
                                     "test time": test_time,
                                     "train-score": train_score,
                                     "test-score": test_score
                                  }))
print(summary)
