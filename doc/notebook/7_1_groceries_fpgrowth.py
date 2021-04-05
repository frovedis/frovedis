"""
Demo for frequent pattern mining algorithms
"""

import os
import time
from collections import OrderedDict
import itertools
import pandas as pd
import numpy as np
import frovedis
from frovedis.exrpc.server import FrovedisServer
from frovedis.mllib.fpm import FPGrowth as frovFPGrowth
import findspark
import pyspark
from pyspark.sql import SQLContext
from pyspark.ml.fpm import FPGrowth as pysparkFPGrowth

def encode_data(data):
    '''
    Encode string grocery data to numeric form
    '''
    unq = np.unique(list(itertools.chain.from_iterable(data)))
    id = np.arange(1, len(unq) + 1, 1)
    transmap = dict(zip(unq, id))
    ret = []
    for e in data:
        enc = [int(transmap[i]) for i in e]
        ret.append(enc)
    return ret

def preprocess_data(fname):
    '''
    Perform pre-processing on groceries dataset
    '''
    df = pd.read_csv(fname)
    df = df.drop(['Item(s)'], axis=1).sample(n=100)
    item_list = []
    for ilist in df.values.tolist():
        item = [itm for itm in ilist if str(itm) != 'nan']
        item_list.append(item)
    item_list = encode_data(item_list)
    return item_list

train_time = []
test_time = []
estimator_name = []

def evaluate(estimator, estimator_nm, data):
    '''
    To generate performance report for both frovedis and sklearn estimators
    '''
    estimator_name.append(estimator_nm)
    start_time = time.time()
    model = estimator.fit(data)
    if "pyspark" in estimator_nm:
        # Count is required to actually invoke the spark operation (since it is lazy)
        print("total FIS count: %d" % (model.freqItemsets.count()))
    else:
        print("total FIS count: %d" % (len(model.freqItemsets)))
    train_time.append(round(time.time() - start_time, 4))

    start_time = time.time()
    sp_rules = model.associationRules
    if "pyspark" in estimator_nm:
        # Count is required to actually invoke the spark operation (since it is lazy)
        print("total Rule count: %d" % (sp_rules.count()))
    else:
        print("total Rule count: %d" % (len(sp_rules)))
    test_time.append(round(time.time() - start_time, 4))

def create_spark_df(data):
    '''
    Construct spark dataframe
    '''
    sp_item_list = []
    cnt = 0
    for ilist in data:
        sp_item_list.append((cnt, ilist))
        cnt = cnt + 1
    sqlContext = SQLContext(sc)
    sp_df = sqlContext.createDataFrame(data=sp_item_list, \
                                       schema=["id", "items"])
    return sp_df

#---- Data Preparation ----
# Please download the dataset from below link.
# https://www.kaggle.com/irfanasrullah/groceries?select=groceries+-+groceries.csv

DATA_FILE = "datasets/groceries - groceries.csv"
item_list = preprocess_data(DATA_FILE)
print("Dataset contain {} item sets".format(len(item_list)))

#1. Demo FPGrowth
FrovedisServer.initialize("mpirun -np 8 " + os.environ["FROVEDIS_SERVER"])
f_est = frovFPGrowth(minSupport=0.01, minConfidence=0.05)
E_NM = "fpgrowth_frovedis_" + frovedis.__version__
evaluate(f_est, E_NM, item_list)
f_est.release()
FrovedisServer.shut_down()

findspark.init()
sc = pyspark.SparkContext(appName="fpgrowth")
try:
    s_est = pysparkFPGrowth(minSupport=0.01, minConfidence=0.05)
    E_NM = "fpgrowth_pyspark_" + pyspark.__version__
    evaluate(s_est, E_NM, create_spark_df(item_list))
finally:
    sc.stop()

# ---- evaluation summary ----
summary = pd.DataFrame(OrderedDict({ "estimator": estimator_name,
                                     "train time": train_time,
                                     "test time": test_time
                                  }))
print(summary)
