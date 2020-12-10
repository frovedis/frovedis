#!/usr/bin/env python

import sys
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

mat =([[1,2,3,4],
       [1,3,2,5],
       [2,3,4,5],
       [1,2,4,5]])

model = FPGrowth(minSupport = 0.2).fit(mat)

model.generate_rules(0.2).debug_print()
model.generate_rules(0.9).debug_print()

model.release()

FrovedisServer.shut_down()





