#!/usr/bin/env python

import sys
from frovedis.exrpc.server import FrovedisServer
from frovedis.mllib.tsa.arima.model import ARIMA

desc = "Testing ARIMA object when endog is not proivded: "

# initializing the Frovedis server
argvs = sys.argv
argc = len(argvs)
if argc < 2:
    print ('Please give frovedis_server calling command as the first argument \n'
           '(e.g. "mpirun -np 2 /opt/nec/frovedis/ve/bin/frovedis_server")')
    quit()
FrovedisServer.initialize(argvs[1])

try:
    arima = ARIMA(order=(1, 1, 1))
    print(desc, "Passed")
except:
    print(desc, "Failed")
    
FrovedisServer.shut_down()
