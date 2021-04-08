
#1. Description

'''
Sentiment classification using Rotten Tomatoes (Movie) review dataset (binary classification).
Dataset can be downloaded from
https://drive.google.com/file/d/1w1TsJB-gmIkZ28d1j7sf1sqcPmHXw352/view

Please dowonload the data manually with your browser and
store it to `datasets` directory.

The Rotten Tomatoes movie review dataset is a corpus of movie reviews used for sentiment analysis.
We will classify a review to be positive ('fresh')
or negative ('rotten') on the basis of review text.

Using this dataset, we train a classifier to predict movie rating based on the review text.
'''

#2. Data Preprocessing

import os
import time
from collections import OrderedDict
import pandas as pd
import sklearn
from sklearn.linear_model import LogisticRegression as skLogisticRegression
from sklearn import metrics
from sklearn.feature_extraction.text import CountVectorizer, TfidfTransformer
from sklearn.model_selection import train_test_split
import frovedis
from frovedis.exrpc.server import FrovedisServer
from frovedis.mllib.linear_model import LogisticRegression as frovLogisticRegression

def create_embed(x_train, x_test):
    '''
    We will generate feature vectors using sklearn TF-IDF for review text.
    '''
    count_vect = CountVectorizer()
    x_train_counts = count_vect.fit_transform(x_train)
    x_test_counts = count_vect.transform(x_test)

    tfidf_transformer = TfidfTransformer()
    x_train_tfidf = tfidf_transformer.fit_transform(x_train_counts)
    x_test_tfidf = tfidf_transformer.transform(x_test_counts)
    return x_train_tfidf, x_test_tfidf

def preprocess_data(fname):
    '''
    For RT review classification we will perform some data preparation and data cleaning steps.
    '''
    df = pd.read_csv(fname, encoding="ISO-8859-1")
    df = df.dropna().drop_duplicates().sample(frac=1)
    print("Dataset contains {} reviews".format(df.shape[0]))
    mapping = {'fresh': 1, 'rotten': 0}
    df['Freshness'] = df.replace({'Freshness': mapping})
    x_train, x_test, y_train, y_test = train_test_split(df["Review"],
                                                        df["Freshness"],
                                                        random_state = 42)
    x_train, x_test = create_embed(x_train, x_test)
    y_train = y_train.to_numpy(dtype='int64')
    y_test = y_test.to_numpy(dtype='int64')
    return x_train, x_test, y_train, y_test

#---- Data Preparation ----

DATA_FILE = "datasets/rt_reviews.csv"
x_train, x_test, y_train, y_test = preprocess_data(DATA_FILE)
print("shape of train data: {}".format(x_train.shape))
print("shape of test data: {}".format(x_test.shape))


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
    estimator.fit(x_train, list(y_train))
    train_time.append(round(time.time() - start_time, 4))

    start_time = time.time()
    pred_y = estimator.predict(x_test)
    test_time.append(round(time.time() - start_time, 4))

    accuracy.append(metrics.accuracy_score(list(y_test), list(pred_y)))
    precision.append(metrics.precision_score(list(y_test), list(pred_y), average='macro'))
    recall.append(metrics.recall_score(list(y_test), list(pred_y), average='macro'))
    f1.append(metrics.f1_score(list(y_test), list(pred_y), average='macro'))

    return metrics.classification_report(list(y_test), list(pred_y))


#3.1 Binary LogisticRegression

TARGET = "binary_logistic_regression_sag"
FrovedisServer.initialize("mpirun -np 8 " + os.environ["FROVEDIS_SERVER"])
f_est = frovLogisticRegression(solver='sag', multi_class='auto',
                               max_iter=3500, penalty='none', \
                               lr_rate=0.005)
E_NM = TARGET + "_frovedis_" + frovedis.__version__
f_report = evaluate(f_est, E_NM, x_train, y_train, x_test, y_test)
f_est.release()
FrovedisServer.shut_down()

s_est = skLogisticRegression(solver ='sag', multi_class = 'auto',
                             max_iter=3500, penalty='none', \
                             n_jobs = 12)
E_NM = TARGET + "_sklearn_" + sklearn.__version__
s_report = evaluate(s_est, E_NM, x_train, y_train, x_test, y_test)

print("Frovedis LogisticRegression metrices: ")
print(f_report)
print("Sklearn LogisticRegression metrices: ")
print(s_report)


#4. Performance summary<font>

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
