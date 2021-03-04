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
data = pd.read_csv("./input/groceries.csv").drop(['Item(s)'], axis=1)
#print(data)

# groceries data contains non-numeric (string) items
# encoding to numeric value is suggested for 
# better performance, it is optional though...
fpm = FPGrowth(minSupport = 0.05)#, encode_string_input = True)

model = fpm.fit(data)
print("frequent item sets: ")
model.debug_print()

model.save("./out/FPModel") # saving the model
model.release() # releasing model from memory after saving into files

model.load("./out/FPModel") # loading the saved model for rule mining
model.generate_rules(0.05).debug_print()
model.generate_rules(0.2).debug_print()
model.generate_rules(0.9).debug_print()

# server clean-up
model.release()
FrovedisServer.shut_down()
