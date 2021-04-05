
#Retail - Frequent Pattern Mining
'''
1. Description
Frequent Pattern Mining using Retail dataset.
Dataset can be downloaded from http://fimi.uantwerpen.be/data/retail.dat
'''

#2. Data Preprocessing

import os
import time
import itertools
from collections import OrderedDict
import pandas as pd
import numpy as np
import pyspark
from pyspark.sql import SQLContext
from pyspark.ml.fpm import FPGrowth as pysparkFPGrowth
import frovedis
from frovedis.exrpc.server import FrovedisServer
from frovedis.mllib.fpm import FPGrowth as frovFPGrowth

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


def create_spark_df(data):
    sp_item_list = []
    cnt = 0
    for ilist in data:
        sp_item_list.append((cnt, ilist))
        cnt = cnt + 1
    # Construct spark dataframe
    sqlContext = SQLContext(sc)
    sp_df = sqlContext.createDataFrame(data=sp_item_list, schema=["id", "items"])
    return sp_df


def get_names(fname):
    fp = open(fname, 'r')
    line = fp.readline()
    max_ncol = 0
    while line:
        ncol = len(line.split(" "))
        if ncol > max_ncol:
            max_ncol = ncol
        line = fp.readline()
    names = ["item_" + str(i) for i in range(max_ncol)]
    return names


def preprocess_data(fname):
    df = pd.read_csv(fname, sep = " ",
                     names = get_names(fname), # variable no. of fields in each line, hence col-names are provided
                     engine = 'python')  # older pandas version has some parsing issue with c-engine
    item_list = []
    for ilist in df.values.tolist():
        item = [itm for itm in ilist if str(itm) != 'nan']
        item_list.append(item)
    item_list = encode_data(item_list)
    return item_list


#---- Data Preparation ----
# Please download the dataset from below link.
# http://fimi.uantwerpen.be/data/retail.dat

DATA_FILE = "datasets/retail.dat"
item_list = preprocess_data(DATA_FILE)
print("Dataset contains {} item sets".format(len(item_list)))


#3. Algorithm Evaluation

train_time = []
test_time = []
estimator_name = []


def evaluate(estimator, estimator_nm, data):
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


#3.1 FPGrowth

sc = pyspark.SparkContext(appName="fpgrowth")
s_est = pysparkFPGrowth(minSupport=0.001, minConfidence=0.05)
E_NM = "fpgrowth_pyspark_" + pyspark.__version__
evaluate(s_est, E_NM, create_spark_df(item_list))
sc.stop()

FrovedisServer.initialize("mpirun -np 8 " +  os.environ["FROVEDIS_SERVER"])
f_est = frovFPGrowth(minSupport=0.001, minConfidence=0.05, mem_opt_level = 1)
E_NM = "fpgrowth_frovedis_" + frovedis.__version__
evaluate(f_est, E_NM, item_list)
f_est.release()
FrovedisServer.shut_down()


#4. Performance summary

summary = pd.DataFrame(OrderedDict({ "estimator": estimator_name,
                                     "train time": train_time,
                                     "test time": test_time
                                  }))
print(summary)

speed_up = train_time[0] / train_time[1]
print("Frovedis Speed-up: %.2f" % (speed_up))
