#!/usr/bin/env python

import sys
import numpy as np
import pandas as pd

from frovedis.exrpc.server import FrovedisServer
from frovedis.matrix.dvector import FrovedisDvector
from frovedis.mllib.fpm import FPGrowth 

# initializing the Frovedis server
argvs = sys.argv
argc = len(argvs)
if (argc < 2):
    print ('Please give frovedis_server calling command as the first argument \n(e.g. "mpirun -np 2 -x /opt/nec/nosupport/frovedis/ve/bin/frovedis_server")')
    quit()
FrovedisServer.initialize(argvs[1])

def encode_data(data):
    import itertools
    unq = np.unique(list(itertools.chain.from_iterable(data)))
    id = np.arange(1, len(unq) + 1, 1)
    transmap = dict(zip(unq, id))
    ret = []
    for e in data:
        enc = [int(transmap[i]) for i in e]
        ret.append(enc)
    return ret

def preprocess_data(fname):
    df = pd.read_csv(fname)
    df = df.drop(['Item(s)'], axis=1) #.sample(n=100)
    item_list = []
    for ilist in df.values.tolist():
        item = [itm for itm in ilist if str(itm) != 'nan']
        item_list.append(item)
    item_list = encode_data(item_list)
    return item_list

'''
mat =([[1,2,3,4],
       [1,3,2,5],
       [2,3,4,5],
       [1,2,4,5]])
'''

mat = preprocess_data("./input/groceries.csv")
#print(mat)
model = FPGrowth(minSupport = 0.05).fit(mat)

model.save("./out/FPModel") #saving the model
model.release()

model.load("./out/FPModel") #loading the saved model
model.debug_print()


model.generate_rules(0.05).debug_print()
model.generate_rules(0.2).debug_print()
model.generate_rules(0.9).debug_print()

model.release()
FrovedisServer.shut_down()
