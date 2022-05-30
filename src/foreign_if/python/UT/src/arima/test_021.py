#!/usr/bin/env python

import sys
import numpy as np
from frovedis.exrpc.server import FrovedisServer
from frovedis.mllib.tsa.arima.model import ARIMA
from frovedis.matrix.dvector import FrovedisDvector

desc = "Testing fitted_values_ attribute by setting it explicitly: "

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

data=FrovedisDvector(data)
arima = ARIMA(endog=data, order=(2, 1, 2)).fit()

try:
    arima.fittedvalues = [0.99, 1.99, 2.99, 3.99, 4.99,
                          5.99, 6.99, 7.99, 8.99, 9.99,
                          10.99, 11.99, 12.99, 13.99, 14.99,
                          14.99, 15.99, 16.99, 17.99, 18.99,
                          19.99, 20.99, 21.99, 22.99, 23.99,
                          24.99, 25.99, 26.99, 27.99, 28.99,
                          29.99, 30.99, 31.99, 32.99, 33.99,
                          34.99, 35.99]
    arima.fittedvalues
    print(desc, "Passed")
except:
    print(desc, "Failed")
    
FrovedisServer.shut_down()
