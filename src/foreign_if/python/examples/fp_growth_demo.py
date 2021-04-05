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

fpm = FPGrowth(minSupport=0.001, minConfidence=0.05,
               compression_point=4, mem_opt_level=1) #memory optimization parameters

# groceries data contains non-numeric (string) items
# encoding to numeric value is suggested by uncommenting below 
# for better performance, it is optional though...
#fpm.encode_string_input = True

import time
t1 = time.time()
fpm.fit(data)
t2 = time.time()
print ("frequent itemsets generation time: %.4f sec" % (t2 - t1))

t1 = time.time()
rule = fpm.generate_rules() # can accept new minConfidence
t2 = time.time()
print ("rules generation time: %.4f sec" % (t2 - t1))

# for pyspark like output
print("frequent itemsets: ")
print(fpm.freqItemsets)
print("frequent itemsets count: %d" % (fpm.count))

print("association rules: ")
print(fpm.associationRules)
print("association rules count: %d" % (rule.count))

# for save/load/release/print 
# server-side in-memory frequent items and rules structures
fpm.save("./out/FPModel") # saving the model
fpm.release() # releasing model from memory after saving into files
fpm.load("./out/FPModel") # loading the saved model
fpm.debug_print() # for printing in memory frequent itemsets in server side

rule.save("./out/FPRule") # saving the rules
rule.release() # releasing rule from memory after saving into files
rule.load("./out/FPRule") #loading the saved rules
rule.debug_print() # for printing in memory rules in server side

# server clean-up
fpm.release()
rule.release()
FrovedisServer.shut_down()
