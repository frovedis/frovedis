#!/usr/bin/env python

import sys
import numpy as np
from frovedis.exrpc.server import FrovedisServer
from frovedis.mllib.tsa.arima.model import ARIMA

desc = "Testing ARIMA with large data and using auto_arima: "

# initializing the Frovedis server
argvs = sys.argv
argc = len(argvs)
if argc < 2:
    print ('Please give frovedis_server calling command as the first argument \n'
           '(e.g. "mpirun -np 2 /opt/nec/frovedis/ve/bin/frovedis_server")')
    quit()
FrovedisServer.initialize(argvs[1])

# sample numpy array of shape (36,)
data = np.loadtxt("./input/shampoo")

try:
    arima = ARIMA(endog=data, order=(10, 1, 10), auto_arima=True).fit()
    print(desc, "Passed")
except:
    print(desc, "Failed")
    
FrovedisServer.shut_down()
