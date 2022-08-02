#!/usr/bin/env python

import sys
import numpy as np

# initializing the Frovedis server
argvs = sys.argv
argc = len(argvs)
if (argc < 2):
    print ('Please give frovedis_server calling command as the first argument \n(e.g. "mpirun -np 2 /opt/nec/frovedis/ve/bin/frovedis_server")')
    quit()

from frovedis.exrpc.server import FrovedisServer
FrovedisServer.initialize(argvs[1])

data = np.loadtxt("./input/shampoo")

#from frovedis.matrix.dvector import FrovedisDvector
# data = FrovedisDvector(data)

# fitting the input time series with frovedis ARIMA
from frovedis.mllib.tsa.arima.model import ARIMA
arima = ARIMA(data, order=(2,1,2)).fit()

print('frovedis fitted data: ', arima.fittedvalues)

# In-sample prediction using frovedis ARIMA
print("arima predict(start=None,end=None): ", arima.predict(start=None, end=None))

# Out-sample prediction using frovedis ARIMA
print("arima predict(start=36,end=37): ", arima.predict(start=36, end=37))

# forecasting using frovedis ARIMA
print('arima forecast(): ', arima.forecast(steps=1))

#get_params/set_params
print("Using get_params(): \n", arima.get_params())
arima.set_params(order = (1,1,1), solver = 'scalapack').fit()
print("After using set_params(): \n", arima.get_params())

import pandas as pd
df = pd.read_csv("./input/daily_temperature.csv", index_col=0, header =None, names= ['Date','Temp'])

#fitting the input time series as pandas dataframe with frovedis ARIMA
arima.set_params(endog = df, order = (2,1,2), solver = 'lapack').fit()

print('frovedis fitted data: ', arima.fittedvalues)

# In-sample prediction using frovedis ARIMA with dates as strings
print("arima predict(start='1981-01-31',end='1981-01-31'): ", arima.predict(start='1981-01-31', end='1981-01-31'))

# Out-sample prediction using frovedis ARIMA
print("arima predict(start=36,end=37): ", arima.predict(start=36, end=37))

# forecasting using frovedis ARIMA
print('arima forecast(): ', arima.forecast(steps=2))

## For Referrence ##
# fitting the input time series with statsmodel ARIMA
# from statsmodels.tsa.arima.model import ARIMA as S_ARIMA
# s_arima = S_ARIMA(endog = data, order=(2,1,2)).fit()

# print('statsmodel fitted data: ', s_arima.fittedvalues)

# In-sample predictions using statsmodel ARIMA
# print("s_arima predict(start=None,end=None): ", s_arima.predict(start=None, end=None))

# Out-sample predictions using statsmodel ARIMA
# print("s_arima predict(start=36,end=37): ", s_arima.predict(start=36, end=37))

# forecasting using statsmodel ARIMA
# print('s_arima forecast(): ', s_arima.forecast(steps=1))

#fitting the input time series as pandas dataframe with statsmodel ARIMA
#s_arima = S_ARIMA(endog = df, order=(2,1,2)).fit()

#print('statsmodel fitted data: ', arima.fittedvalues)

# In-sample prediction using statsmodel ARIMA with dates as strings
#print("s_arima predict(start='1981-01-31',end='1981-01-31'): ", s_arima.predict(start='1981-01-31', end='1981-01-31'))

# Out-sample prediction using statsmodel ARIMA
#print("s_arima predict(start=36,end=37): ", s_arima.predict(start=36, end=37))

# forecasting using ststmodel ARIMA
#print('s_arima forecast(): ', s_arima.forecast(steps=2))

arima.release()
FrovedisServer.shut_down()
