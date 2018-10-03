#!/usr/bin/env python

import os
import sys
import traceback
from exrpc.server import FrovedisServer

def myexcepthook(type, value, trace):
  traceback.print_tb(trace)
  print(type.__name__ + ": " + str(value))
  FrovedisServer.shut_down()

# deprecated: as the same is now achieved through atexit
#sys.excepthook = myexcepthook

