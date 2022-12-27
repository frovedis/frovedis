#!/usr/bin/env python

import sys
import numpy as np
from frovedis.exrpc.server import FrovedisServer
from frovedis.mllib.tsa.arima.model import ARIMA

desc = "Testing forecast() when exog is provided: "

# initializing the Frovedis server
argvs = sys.argv
argc = len(argvs)
if argc < 2:
    print ('Please give frovedis_server calling command as the first argument \n'
           '(e.g. "mpirun -np 2 /opt/nec/frovedis/ve/bin/frovedis_server")')
    quit()
FrovedisServer.initialize(argvs[1])

# sample numpy array of shape (36,)
data = np.asarray([266, 145.9, 183.1, 119.3, 180.3, 
                  168.5, 231.8, 224.5, 192.8, 122.9,
                  336.5, 185.9, 194.3, 149.5, 210.1,
                  273.3, 191.4, 287, 226, 303.6, 289.9, 
                  421.6, 264.5, 342.3, 339.7, 440.4, 
                  315.9, 439.3, 401.3, 437.4, 575.5, 
                  407.6, 682, 475.3, 581.3, 646.9])

arima = ARIMA(endog=data, order=(2, 1, 2)).fit()

try:
    arima.forecast(exog=np.array([1,2,3,4,5,6,
                                  7,8,9,10,11,
                                  12,13,14,15,16,
                                  17,18,19,20,21,
                                  22,23,24,25,26,
                                  27,28,29,30,31,
                                  32,33,34,35]))
    print(desc, "Passed")
except:
    print(desc, "Failed")
    
FrovedisServer.shut_down()
