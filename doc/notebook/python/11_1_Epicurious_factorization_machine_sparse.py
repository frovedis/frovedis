
#Regression & Classification

#1. Description
'''
Dataset:
1. This dataset explores different factors affecting the people's
   rating for a particular food item.
2. Dataset contains over 20k recipes listed by recipe rating,
   nutritional information and assigned category (sparse).
Objective (Regression):
1. The objective is to find the rating of the food recipe based
   on its nutritional information and assigned category.
Objective (Classification):
1. The objective is to classify the food recipe as highly rated or not of the food recipe based
   on its nutritional information and assigned category.
2. Food recipe with ratings greater than 3.8 is classified as highly rated food (1)
   and food recipe with rating less than equal to 3.8 is classified as average/poorly
   rated food (0).
'''

#2. Data Preprocessing

# Importing the standard libraries
import time
import os
import warnings
from collections import OrderedDict
import scipy
import pandas as pd
from pandas.core.common import SettingWithCopyWarning

# Importing modules from Sklearn library
from sklearn.preprocessing import MinMaxScaler
from sklearn.model_selection import train_test_split

# Importing modules from Pyspark library
import pyspark
from pyspark.ml.regression import FMRegressor as pFMR
from pyspark.sql import SparkSession
from pyspark.ml.feature import VectorAssembler
from pyspark.ml.evaluation import RegressionEvaluator
from pyspark.ml.evaluation import MulticlassClassificationEvaluator
from pyspark.ml.classification import FMClassifier as pFMC

# Importing modules from frovedis library
import frovedis
from frovedis.exrpc.server import FrovedisServer
from frovedis.mllib.fm import FactorizationMachineRegressor as fFMR
from frovedis.mllib.fm import FactorizationMachineClassifier as fFMC

# Creating the Spark Session
spark = SparkSession.builder.appName("food_recipe").getOrCreate()
print("Pyspark Session Created")

def preprocess_data_regression(filename):
    # Reading the data
    data = pd.read_csv(filename)
    print("Shape of the data: ", data.shape)


    # Renaming the columns
    data = data.rename({'rating': 'label',
                        'st. louis': 'st_luis',
                        "st. patrick's day": 'st_patrick_day',
                        "washington, d.c.": "washington_dc"}, axis='columns')

    # Supressing the SettingWithCopyWarning
    warnings.simplefilter(action="ignore", category=SettingWithCopyWarning)

    # Typecasting the columns
    data.label = data.label.astype('float64')
    data.calories = data.calories.astype('float64')

    # Removing the title column
    data = data.drop(['title'], axis=1)

    # Replacing the null values of columns with their respective medians
    data['label'].fillna(data['label'].median(), inplace=True)
    data['calories'].fillna(data['calories'].median(), inplace=True)
    data['protein'].fillna(data['protein'].median(), inplace=True)
    data['fat'].fillna(data['fat'].median(), inplace=True)
    data['sodium'].fillna(data['sodium'].median(), inplace=True)

    # Splitting the data into train_data (80%) and test_data (20%)
    train_data, test_data = train_test_split(data, test_size=0.20, random_state=42)

    # Scaling the train_data and test_data
    # Initializing the scalar
    scaler = MinMaxScaler()

    # Scaling all the columns of the train data
    scaling_columns = train_data.columns[1:]

    # Scaling the training data
    scaler.fit(train_data[scaling_columns])
    train_data[scaling_columns] = scaler.transform(train_data[scaling_columns])

    # Scaling the testing data
    test_data[scaling_columns] = scaler.transform(test_data[scaling_columns])

    # Saving the test_data and train_data as CSVs
    train_data.to_csv("epr_train_reg.csv", index=None)
    test_data.to_csv("epr_test_reg.csv", index=None)

    print("Shape of the training data: ", train_data.shape)
    print("Shape of the testing data: ", test_data.shape)

    print("Training data and testing data created successfully. \n")


def preprocess_data_classification(filename):
    # Reading the data
    data = pd.read_csv(filename)
    print("Shape of the data: ", data.shape)

    # Renaming the columns
    data = data.rename({'rating': 'label',
                        'st. louis': 'st_luis',
                        "st. patrick's day": 'st_patrick_day',
                        "washington, d.c.": "washington_dc"}, axis='columns')

    # Supressing the SettingWithCopyWarning
    warnings.simplefilter(action="ignore", category=SettingWithCopyWarning)

    # Typecasting the columns
    data.label = data.label.astype('float64')
    data.calories = data.calories.astype('float64')

    # Removing the title column
    data = data.drop(['title'], axis=1)

    # Replacing the null values of columns with their respective medians
    data['label'].fillna(data['label'].median(), inplace=True)
    data['calories'].fillna(data['calories'].median(), inplace=True)
    data['protein'].fillna(data['protein'].median(), inplace=True)
    data['fat'].fillna(data['fat'].median(), inplace=True)
    data['sodium'].fillna(data['sodium'].median(), inplace=True)

    # Converting the label <= 4 to 0 and label > 4 to 1
    data.loc[data['label'] <= 4, 'new_label'] = 0
    data.loc[data['label'] > 4, 'new_label'] = 1

    # Removing the old label column
    data = data.drop(['label'], axis=1)

    # Splitting the data into train_data (80%) and test_data (20%)
    train_data, test_data = train_test_split(data, test_size=0.20, random_state=42)

    # Scaling the train_data and test_data
    # Initializing the scalar
    scaler = MinMaxScaler()

    # Scaling all the columns of the train data
    scaling_columns = train_data.columns[1:]

    # Scaling the training data
    scaler.fit(train_data[scaling_columns])
    train_data[scaling_columns] = scaler.transform(train_data[scaling_columns])

    # Scaling the testing data
    test_data[scaling_columns] = scaler.transform(test_data[scaling_columns])

    # Saving the test_data and train_data as CSVs
    train_data.to_csv("epr_train_cl.csv", index=None)
    test_data.to_csv("epr_test_cl.csv", index=None)

    print("Shape of the training data: ", train_data.shape)
    print("Shape of the testing data: ", test_data.shape)

    print("Training data and testing data created successfully. \n")


#3. Algorithm Evaluation

train_time = []
test_time = []
train_score = []
test_score = []
estimator_name = []


def evaluate_regression(estimator, estimator_nm, train_data, test_data):
    estimator_name.append(estimator_nm)

    if "pyspark" in estimator_nm:
        # Creating the object for feature vector
        featureAssembler = VectorAssembler(inputCols=train_data.columns[1:], outputCol="Features")

        # Freature Vector of train Data
        temp_output = featureAssembler.transform(train_data)
        FeatureVec_train_data = temp_output.select("Features", "label")

        # Freature Vector of test Data
        temp_output1 = featureAssembler.transform(test_data)
        FeatureVec_test_data = temp_output1.select("Features", "label")

        start_time = time.time()
        pyspark_FM_reg_model = estimator.fit(FeatureVec_train_data)
        train_time.append(round(time.time() - start_time, 4))
        start_time = time.time()
        # Testing the model on train_data
        predictions_train = pyspark_FM_reg_model.transform(FeatureVec_train_data)
        # Creating the object of evaluator
        evaluator_r2_train = RegressionEvaluator(labelCol="label", predictionCol="prediction", metricName="r2")
        train_score.append(evaluator_r2_train.evaluate(predictions_train))

        # Testing the model on test_data
        predictions_test = pyspark_FM_reg_model.transform(FeatureVec_test_data)
        # Creating the object of evaluator
        evaluator_r2 = RegressionEvaluator(labelCol="label", predictionCol="prediction", metricName="r2")
        test_score.append(evaluator_r2.evaluate(predictions_test))

        test_time.append(round(time.time() - start_time, 4))

    elif "frovedis" in estimator_nm:
        # Features
        train_features = train_data.drop(columns=["label"])
        test_features = test_data.drop(columns=["label"])
        # Ratings (target variable)
        train_rating = train_data["label"]
        test_rating = test_data["label"]
        X_train_sparse_matrix = scipy.sparse.csr_matrix(train_features.values)
        X_test_sparse_matrix = scipy.sparse.csr_matrix(test_features.values)

        start_time = time.time()
        estimator.fit(X_train_sparse_matrix, train_rating)
        train_time.append(round(time.time() - start_time, 4))

        start_time = time.time()
        train_score.append(estimator.score(X_train_sparse_matrix, train_rating))
        test_score.append(estimator.score(X_test_sparse_matrix, test_rating))
        test_time.append(round(time.time() - start_time, 4))


def evaluate_classifier(estimator, estimator_nm, train_data, test_data):
    estimator_name.append(estimator_nm)

    if "pyspark" in estimator_nm:
        # Creating the object for feature vector
        featureAssembler = VectorAssembler(inputCols=train_data.columns[:-1], outputCol="Features")

        # Freature Vector of train Data
        temp_output = featureAssembler.transform(train_data)
        FeatureVec_train_data = temp_output.select("Features", "new_label")

        # Freature Vector of test Data
        temp_output1 = featureAssembler.transform(test_data)
        FeatureVec_test_data = temp_output1.select("Features", "new_label")

        start_time = time.time()
        pyspark_FM_cl_model = estimator.fit(FeatureVec_train_data)
        train_time.append(round(time.time() - start_time, 4))

        start_time = time.time()
        # Testing the model on train_data
        predictions_train = pyspark_FM_cl_model.transform(FeatureVec_train_data)
        # Creating the object of evaluator
        evaluator_classification = MulticlassClassificationEvaluator(
            labelCol="new_label", predictionCol="prediction", metricName="accuracy")
        train_score.append(evaluator_classification.evaluate(predictions_train))

        # Testing the model on test_data
        predictions_test = pyspark_FM_cl_model.transform(FeatureVec_test_data)
        # Creating the object of evaluator
        evaluator_classification_test = MulticlassClassificationEvaluator(
            labelCol="new_label", predictionCol="prediction", metricName="accuracy")
        test_score.append(evaluator_classification_test.evaluate(predictions_test))

        test_time.append(round(time.time() - start_time, 4))

    elif "frovedis" in estimator_nm:
        # Features
        train_features = train_data.drop(columns=["new_label"])
        test_features = test_data.drop(columns=["new_label"])
        # Ratings (target variable)
        train_rating = train_data["new_label"]
        test_rating = test_data["new_label"]
        X_train_sparse_matrix = scipy.sparse.csr_matrix(train_features.values)
        X_test_sparse_matrix = scipy.sparse.csr_matrix(test_features.values)

        start_time = time.time()
        estimator.fit(X_train_sparse_matrix, train_rating)
        train_time.append(round(time.time() - start_time, 4))

        start_time = time.time()
        train_score.append(estimator.score(X_train_sparse_matrix, train_rating))
        test_score.append(estimator.score(X_test_sparse_matrix, test_rating))
        test_time.append(round(time.time() - start_time, 4))


#---- Data Preparation for Regression----

preprocess_data_regression('datasets/epi_r.csv')
# Loading the dataset for Pyspark
train_data_nm_reg = 'epr_train_reg.csv'
test_data_nm_reg = 'epr_test_reg.csv'
train_data_pyspark = spark.read.csv(train_data_nm_reg, inferSchema=True, header=True)
test_data_pyspark = spark.read.csv(test_data_nm_reg, inferSchema=True, header=True)

# Loading the dataset for Frovedis Regression
train_df = pd.read_csv(train_data_nm_reg)
test_df = pd.read_csv(test_data_nm_reg)


#---- Data Preparation for Classification----

preprocess_data_classification('datasets/epi_r.csv')
# Loading the dataset for Pyspark
train_data_nm_cl = 'epr_train_cl.csv'
test_data_nm_cl = 'epr_test_cl.csv'
train_data_pyspark_cl = spark.read.csv(train_data_nm_cl, inferSchema=True, header=True)
test_data_pyspark_cl = spark.read.csv(test_data_nm_cl, inferSchema=True, header=True)

# Loading the dataset for Frovedis Regression
train_df_cl = pd.read_csv(train_data_nm_cl)
test_df_cl = pd.read_csv(test_data_nm_cl)


#3.1 FactorizationMachineRegressor

TARGET = "factorization_machines_regressor"
FrovedisServer.initialize("mpirun -np 8 " +  os.environ["FROVEDIS_SERVER"])
f_est = fFMR()
E_NM = TARGET + "_frovedis_" + frovedis.__version__
evaluate_regression(f_est, E_NM, train_df, test_df)
f_est.release()
FrovedisServer.shut_down()

p_est = pFMR(featuresCol="Features", stepSize=0.01)
E_NM = TARGET + "_pyspark_" + pyspark.__version__
evaluate_regression(p_est, E_NM, train_data_pyspark, test_data_pyspark)


#3.2 FactorizationMachineClassifier

TARGET = "factorization_machines_classifier"
FrovedisServer.initialize("mpirun -np 8 " +  os.environ["FROVEDIS_SERVER"])
f_est = fFMC()
E_NM = TARGET + "_frovedis_" + frovedis.__version__
evaluate_classifier(f_est, E_NM, train_df_cl, test_df_cl)
f_est.release()
FrovedisServer.shut_down()

p_est = pFMC(labelCol="new_label", featuresCol="Features")
E_NM = TARGET + "_pyspark_" + pyspark.__version__
evaluate_classifier(p_est, E_NM, train_data_pyspark_cl, test_data_pyspark_cl)


summary = pd.DataFrame(OrderedDict({ "estimator": estimator_name,
                                     "train time": train_time,
                                     "test time": test_time,
                                     "train-score": train_score,
                                     "test-score": test_score
                                  }))
print(summary)
