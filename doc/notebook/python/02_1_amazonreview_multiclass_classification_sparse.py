
#1. Description
'''
Sentiment classification using Amazon review
dataset (multi class classification). Dataset can be downloaded
from https://s3.amazonaws.com/amazon-reviews-pds/tsv/amazon_reviews_us_Books_v1_02.tsv.gz

The consumer reviews serve as feedback for businesses in terms of performance,
product quality, and consumer service. An online review typically consists of
free-form text and a star rating out of 5.

The problem of predicting a user’s star rating for a product,
given the user’s text review for that product is lately become a popular,
albeit hard, problem in machine learning.

Using this dataset, we train a classifier to predict product rating based on the review text.

Predicting the ratings based on the text is particulary difficult tasks.
The primary reason for the difficulty is that two person can provide 
different ratings for writing similar reviews.

As the scale for ratings increases (scale of 5 to scale of 10),
the tasks become increasingly difficult.
'''

#2. Data Preprocessing

import os
from collections import OrderedDict
import time
import pandas as pd
import sklearn
from sklearn.linear_model import LogisticRegression as skLogisticRegression
from sklearn import metrics
from sklearn.model_selection import train_test_split
from sklearn.feature_extraction.text import CountVectorizer, TfidfTransformer
from sklearn.naive_bayes import MultinomialNB as sMNB
from sklearn.naive_bayes import BernoulliNB as skNB
import frovedis
from frovedis.exrpc.server import FrovedisServer
from frovedis.mllib.linear_model import LogisticRegression as frovLogisticRegression
from frovedis.mllib.naive_bayes import MultinomialNB as fMNB
from frovedis.mllib.naive_bayes import BernoulliNB as frovNB

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
    For amazon review classification we will perform some data preparation and data cleaning steps.
    '''
    df = pd.read_csv(fname, sep='\t', error_bad_lines=False)
    df = df[["review_body", "star_rating"]]
    df = df.dropna().drop_duplicates().sample(frac=1) # why sampling?
    print("Dataset contains {} reviews".format(df.shape[0]))
    rating_categories = df["star_rating"].value_counts()
    x_train, x_test, y_train, y_test = train_test_split(df["review_body"],
                                                        df["star_rating"],
                                                        random_state = 42)
    x_train, x_test = create_embed(x_train, x_test)
    return x_train, x_test, y_train, y_test, rating_categories

#---- Data Preparation ----
# Please uncomment the below lines to download and unzip the dataset.
#!wget -N https://s3.amazonaws.com/amazon-reviews-pds/tsv/amazon_reviews_us_Books_v1_02.tsv.gz
#!gunzip amazon_reviews_us_Books_v1_02.tsv.gz
#!mv amazon_reviews_us_Books_v1_02.tsv datasets

DATA_FILE = "datasets/amazon_reviews_us_Books_v1_02.tsv/amazon_reviews_us_Books_v1_02.tsv"
x_train, x_test, y_train, y_test, rating_categories = preprocess_data(DATA_FILE)
print("shape of train data: {}".format(x_train.shape))
print("shape of test data: {}".format(x_test.shape))


# Label distribution summary
ax = rating_categories.plot(kind='bar', title='Label Distribution').set(xlabel="Rating Id's", ylabel="No. of reviewes")

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
    precision.append(metrics.precision_score(y_test, pred_y, average='macro'))
    recall.append(metrics.recall_score(y_test, pred_y, average='macro'))
    f1.append(metrics.f1_score(y_test, pred_y, average='macro'))

    target_names = ['rating 1.0', 'rating 2.0', 'rating 3.0', 'rating 4.0', 'rating 5.0']
    return metrics.classification_report(y_test, pred_y, target_names=target_names)


#3.1 Multinomial LogisticRegression

TARGET = "multinomial_logistic_regression"
FrovedisServer.initialize("mpirun -np 8 " + os.environ["FROVEDIS_SERVER"])
f_est = frovLogisticRegression(max_iter=3100, penalty='none', lr_rate=0.001, tol=1e-8)
E_NM = TARGET + "_frovedis_" + frovedis.__version__
f_report = evaluate(f_est, E_NM, x_train, y_train, x_test, y_test)
f_est.release()
FrovedisServer.shut_down()

s_est = skLogisticRegression(max_iter = 3100, penalty='none', tol = 1e-8, n_jobs = 12)
E_NM = TARGET + "_sklearn_" + sklearn.__version__
s_report = evaluate(s_est, E_NM, x_train, y_train, x_test, y_test)

# LogisticRegression: Precision, Recall and F1 score for each class
print("Frovedis LogisticRegression metrices: ")
print(f_report)
print("Sklearn LogisticRegression metrices: ")
print(s_report)


#3.2 MultinomialNB

TARGET = "multinomial_naive_bayes"
FrovedisServer.initialize("mpirun -np 8 " + os.environ["FROVEDIS_SERVER"])
f_est = fMNB()
E_NM = TARGET + "_frovedis_" + frovedis.__version__
f_report = evaluate(f_est, E_NM, x_train, y_train, x_test, y_test)
f_est.release()
FrovedisServer.shut_down()

s_est = sMNB()
E_NM = TARGET + "_sklearn_" + sklearn.__version__
s_report = evaluate(s_est, E_NM, x_train, y_train, x_test, y_test)

# MultinomialNB: Precision, Recall and F1 score for each class
print("Frovedis MultinomialNB metrices: ")
print(f_report)
print("Sklearn MultinomialNB metrices: ")
print(s_report)

#3.3 Bernoulli Naive Bayes

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

#4. Performance summary

summary = pd.DataFrame(OrderedDict({ "estimator": estimator_name,
                                     "train time": train_time,
                                     "test time": test_time,
                                     "accuracy": accuracy,
                                     "precision": precision,
                                     "recall": recall,
                                     "f1-score": f1
                                  }))
print(summary)
