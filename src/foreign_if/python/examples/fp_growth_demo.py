#!/usr/bin/env python

import sys
import pandas as pd
from frovedis.exrpc.server import FrovedisServer
from frovedis.mllib.fpm import FPGrowth 

# initializing the Frovedis server
argvs = sys.argv
argc = len(argvs)
if (argc < 2):
    print ('Please give frovedis_server calling command as the first argument \n(e.g. "mpirun -np 2 -x /opt/nec/nosupport/frovedis/ve/bin/frovedis_server")')
    quit()
FrovedisServer.initialize(argvs[1])

'''
data = [[1,4],
        [1,3,2,5],
        [1],
        [1,2,5]]
'''

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

data = pd.read_csv("./input/retail.dat", sep = " ", 
       names = get_names("./input/retail.dat"), # variable no. of fields in each line, hence col-names are provided
       engine = 'python') # older pandas version has some parsing issue with c-engine

#data = pd.read_csv("./input/groceries.csv").drop(['Item(s)'], axis=1)

fpm = FPGrowth(min_support = 0.05, tree_depth=10, 
               compression_point=4, mem_opt_level=1)

# groceries data contains non-numeric (string) items
# encoding to numeric value is suggested by uncommenting below 
# for better performance, it is optional though...
#fpm.encode_string_input = True

model = fpm.fit(data)
print("frequent item count: %d" % (model.fis_count))
print("frequent item sets: ")
model.debug_print()

model.save("./out/FPModel") # saving the model
model.release() # releasing model from memory after saving into files

model.load("./out/FPModel") # loading the saved model for rule mining
print("frequent item count in loaded model: %d" % (model.fis_count))
model.generate_rules(0.05).debug_print()
model.generate_rules(0.2).debug_print()
model.generate_rules(0.9).debug_print()

# server clean-up
model.release()
FrovedisServer.shut_down()
