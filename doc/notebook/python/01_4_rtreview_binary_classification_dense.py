
#1. Description
'''
Sentiment classification using Rotten Tomatoes (Movie) review dataset (binary classification).
Dataset can be downloaded from
https://drive.google.com/file/d/1w1TsJB-gmIkZ28d1j7sf1sqcPmHXw352/view

Please dowonload the data manually with your browser and store it to `datasets` directory.

The Rotten Tomatoes movie review dataset is a corpus of movie reviews used for sentiment analysis.

We will classify a review to be positive ('fresh')
or negative ('rotten') on the basis of review text.

Using this dataset, we train a classifier to predict movie rating based on the review text.
'''

#2. Data Preprocessing

import os
import re
import time
from collections import OrderedDict
import pandas as pd
import numpy as np
from nltk.corpus import stopwords
from nltk.tokenize import word_tokenize
import sklearn
from sklearn.svm import SVC as skSVC
from sklearn import metrics
from sklearn.model_selection import train_test_split
from sklearn.tree import DecisionTreeClassifier as skDecisionTreeClassifier
from sklearn.ensemble import RandomForestClassifier as skRandomForestClassifier
from sklearn.ensemble import GradientBoostingClassifier as skGBC
from sklearn.svm import LinearSVC as skSVC
from sklearn.naive_bayes import BernoulliNB as skNB
from sklearn.neighbors import KNeighborsClassifier as skKNC
from sklearn.linear_model import SGDClassifier as skSGD
import frovedis
from frovedis.exrpc.server import FrovedisServer
from frovedis.mllib.feature.w2v import Word2Vec as Frovedis_Word2Vec
from frovedis.mllib.svm import SVC as frovSVC
from frovedis.mllib.tree import DecisionTreeClassifier as frovDecisionTreeClassifier
from frovedis.mllib.ensemble import RandomForestClassifier as frovRandomForestClassifier
from frovedis.mllib.ensemble.gbtree import GradientBoostingClassifier as frovGBC
from frovedis.mllib.svm import LinearSVC as frovSVC
from frovedis.mllib.naive_bayes import BernoulliNB as frovNB
from frovedis.mllib.neighbors import KNeighborsClassifier as frovKNC
from frovedis.mllib.linear_model import SGDClassifier as frovSGD

def clean_review(review):
    '''
    create list of review words
    '''
    pattern = re.compile(r'[^a-zA-Z0-9 ]')
    review = pattern.sub(' ', review)
    return review

def document_vector_frovedis(doc, frov_w2v_model, frov_vocab):
    """Create document vectors by averaging word vectors. Remove out-of-vocabulary words."""
    no_embedding = np.zeros(frov_w2v_model.shape[1])
    vocab_doc = [word for word in doc if word in frov_vocab]
    if len(vocab_doc) != 0:
        return list(np.mean(frov_w2v_model.loc[vocab_doc], axis=0))
    else:
        return list(no_embedding)

def create_w2v_embed(df):
    '''
    We will generate feature vectors using Frovedis Word2Vec for review text.
    '''
    os.environ["VE_OMP_NUM_THREADS"] = '8'
    FrovedisServer.initialize("mpirun -np 1 " + os.environ["FROVEDIS_SERVER"])
    frovedis_w2v = Frovedis_Word2Vec(sentences = list(df["Review"]), hiddenSize=512, minCount=2, n_iter=100)
    x_emb = frovedis_w2v.transform(list(df["Review"]), func = np.mean)
    os.environ["VE_OMP_NUM_THREADS"] = '1'
    FrovedisServer.shut_down()
    return pd.DataFrame(x_emb)

def preprocess_data(fname):
    '''
    For RT review classification we will perform some data preparation and data cleaning steps.
    '''
    df = pd.read_csv(fname, encoding="ISO-8859-1")
    df = df.dropna().drop_duplicates().sample(frac=1, random_state=42)
    df['Review'] = df['Review'].str.lower().apply(clean_review)
    mapping = {'fresh': 1, 'rotten': 0}
    df['Freshness'] = df.replace({'Freshness': mapping})
    print("Dataset contains {} reviews".format(df.shape[0]))
    stop = stopwords.words('english')
    df['Review'] = df['Review'].apply(lambda x: [item for item in word_tokenize(x) if item not in stop])
    x = create_w2v_embed(df)
    x_train, x_test, y_train, y_test = train_test_split(x, df["Freshness"], random_state=42)
    return x_train, x_test, y_train, y_test

#---- Data Preparation ----
DATA_FILE = "datasets/rt_reviews.csv"
x_train, x_test, y_train, y_test = preprocess_data(DATA_FILE)

print("Generated vector for train data are of shape {}".format(x_train.shape))
print("Generated vector for test data are of shape {}".format(x_test.shape))

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


#3.1 Kernel SVC

TARGET = "SVC"
os.environ["VE_OMP_NUM_THREADS"] = "8"
FrovedisServer.initialize("mpirun -np 1 " + os.environ["FROVEDIS_SERVER"])

f_est = frovSVC(cache_size=2048, max_iter=10000, gamma=1.0)
E_NM = TARGET + "_frovedis_" + frovedis.__version__
f_report = evaluate(f_est, E_NM, x_train, y_train, x_test, y_test)
f_est.release()
os.environ["VE_OMP_NUM_THREADS"] = "1"
FrovedisServer.shut_down()

s_est = skSVC(cache_size=2048, max_iter=10000, gamma=1.0)
E_NM = TARGET + "_sklearn_" + sklearn.__version__
s_report = evaluate(s_est, E_NM, x_train, y_train, x_test, y_test)

# Precision, Recall and F1 score for each class
print("Frovedis SVC metrices: ")
print(f_report)
print("Sklearn SVC metrices: ")
print(s_report)


#3.2 Decision Tree

TARGET = "decision_tree"
FrovedisServer.initialize("mpirun -np 8 " + os.environ["FROVEDIS_SERVER"])

f_est = frovDecisionTreeClassifier(max_leaf_nodes=2, max_depth=8)
E_NM = TARGET + "_frovedis_" + frovedis.__version__
f_report = evaluate(f_est, E_NM, x_train, y_train, x_test, y_test)
f_est.release()
FrovedisServer.shut_down()

s_est = skDecisionTreeClassifier(max_leaf_nodes=2, max_depth=8)
E_NM = TARGET + "_sklearn_" + sklearn.__version__
s_report = evaluate(s_est, E_NM, x_train, y_train, x_test, y_test)

# Precision, Recall and F1 score for each class
print("Frovedis Decision Tree metrices: ")
print(f_report)
print("Sklearn Decision Tree metrices: ")
print(s_report)


#3.3 Random Forest

TARGET = "random_forest"
FrovedisServer.initialize("mpirun -np 8 " + os.environ["FROVEDIS_SERVER"])

f_est = frovRandomForestClassifier(n_estimators=200, criterion='entropy', max_features=0.5, max_depth=10,                                    min_samples_split=5, min_samples_leaf=2)
E_NM = TARGET + "_frovedis_" + frovedis.__version__
f_report = evaluate(f_est, E_NM, x_train, y_train, x_test, y_test)
f_est.release()
FrovedisServer.shut_down()

s_est = skRandomForestClassifier(n_estimators=200, criterion='entropy', max_features=0.5, max_depth=10,                                  min_samples_split=5, min_samples_leaf=2, n_jobs=12)
E_NM = TARGET + "_sklearn_" + sklearn.__version__
s_report = evaluate(s_est, E_NM, x_train, y_train, x_test, y_test)

# Precision, Recall and F1 score for each class
print("Frovedis Random Forest metrices: ")
print(f_report)
print("Sklearn Random Forest metrices: ")
print(s_report)


#3.4 Gradient Boosting Tree

TARGET = "gradient_boosting"
FrovedisServer.initialize("mpirun -np 8 " + os.environ["FROVEDIS_SERVER"])

f_est = frovGBC(n_estimators=100)
E_NM = TARGET + "_frovedis_" + frovedis.__version__
f_report = evaluate(f_est, E_NM, x_train, y_train, x_test, y_test)
f_est.release()
FrovedisServer.shut_down()

s_est = skGBC(n_estimators=100)
E_NM = TARGET + "_sklearn_" + sklearn.__version__
s_report = evaluate(s_est, E_NM, x_train, y_train, x_test, y_test)

# Precision, Recall and F1 score for each class
print("Frovedis Gradient Boosting metrices: ")
print(f_report)
print("Sklearn Gradient Boosting metrices: ")
print(s_report)

#3.5 Linear SVC

TARGET = "Linear_SVC"
FrovedisServer.initialize("mpirun -np 8 " + os.environ["FROVEDIS_SERVER"])

f_est = frovSVC(loss='hinge', max_iter = 60000)
E_NM = TARGET + "_frovedis_" + frovedis.__version__
f_report = evaluate(f_est, E_NM, x_train, y_train, x_test, y_test)
f_est.release()
FrovedisServer.shut_down()

s_est = skSVC(loss='hinge', max_iter = 60000)
E_NM = TARGET + "_sklearn_" + sklearn.__version__
s_report = evaluate(s_est, E_NM, x_train, y_train, x_test, y_test)

# Precision, Recall and F1 score for each class
print("Frovedis Linear SVC metrices: ")
print(f_report)
print("Sklearn Linear SVC metrices: ")
print(s_report)


#3.6 Bernoulli Naive Bayes

TARGET = "bernoulli_naive_bayes"
FrovedisServer.initialize("mpirun -np 8 " + os.environ["FROVEDIS_SERVER"])

f_est = frovNB(alpha=1.0)
E_NM = TARGET + "_frovedis_" + frovedis.__version__
f_report = evaluate(f_est, E_NM, x_train, y_train, x_test, y_test)
f_est.release()
FrovedisServer.shut_down()

s_est = skNB(alpha=1.0)
E_NM = TARGET + "_sklearn_" + sklearn.__version__
s_report = evaluate(s_est, E_NM, x_train, y_train, x_test, y_test)
# Precision, Recall and F1 score for each class
print("Frovedis Bernoulli Naive Bayes metrices: ")
print(f_report)
print("Sklearn Bernoulli Naive Bayes metrices: ")
print(s_report)


#3.7 Nearest Neighbor Classification

TARGET = "nearest_neighbor_classification"
FrovedisServer.initialize("mpirun -np 8 " + os.environ["FROVEDIS_SERVER"])

f_est = frovKNC(n_neighbors=3, algorithm='brute', metric='euclidean')
E_NM = TARGET + "_frovedis_" + frovedis.__version__

#we are using first 50k samples inorder to avoid memory issue at Frovedis Server
f_report = evaluate(f_est, E_NM, x_train[:50000], y_train[:50000], x_test[:50000], y_test[:50000])
f_est.release()
FrovedisServer.shut_down()

s_est = skKNC(n_neighbors=3, algorithm='brute', metric='euclidean', n_jobs=12)
E_NM = TARGET + "_sklearn_" + sklearn.__version__
s_report = evaluate(s_est, E_NM, x_train[:50000], y_train[:50000], x_test[:50000], y_test[:50000])

# Precision, Recall and F1 score for each class
print("Frovedis KNeighborsClassifier metrices: ")
print(f_report)
print("Sklearn KNeighborsClassifier metrices: ")
print(s_report)


#3.8 SGDClassifier

TARGET = "SGDClassifier"
FrovedisServer.initialize("mpirun -np 8 " + os.environ["FROVEDIS_SERVER"])

f_est = frovSGD(learning_rate="invscaling", eta0=1.0)
E_NM = TARGET + "_frovedis_" + frovedis.__version__
f_report = evaluate(f_est, E_NM, x_train, y_train, x_test, y_test)
f_est.release()
FrovedisServer.shut_down()

s_est = skSGD(learning_rate="invscaling", eta0=1.0, n_jobs=12)
E_NM = TARGET + "_sklearn_" + sklearn.__version__
s_report = evaluate(s_est, E_NM, x_train, y_train, x_test, y_test)

# Precision, Recall and F1 score for each class
print("Frovedis SGDClassifier metrices: ")
print(f_report)
print("Sklearn SGDClassifier metrices: ")
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
