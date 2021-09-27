'''
undirected graph - with 2 components  >> Passed
'''
from frovedis.exrpc.server import FrovedisServer 
import os
import sys
import time
import numpy as np
import pandas as pd
import networkx as nx
import frovedis.graph as fnx

DATASET = "input/data"

#FROVEDIS
try:
    argvs = sys.argv
    argc = len(argvs)
    if (argc < 2):
        print ('Please give frovedis_server calling command as the first argument \n(e.g. "mpirun -np 2 /opt/nec/frovedis/ve/bin/frovedis_server")')
        quit()
    FrovedisServer.initialize(argvs[1])
    
    frov_graph = fnx.read_edgelist(DATASET, nodetype=np.int32, delimiter=' ')
    fres = list(fnx.connected_components(frov_graph))
    
    FrovedisServer.shut_down()
except Exception as e:
    print ("status=Exception: " + str(e))
    sys.exit(1)

#NetworkX
try: 
    nx_graph = nx.read_edgelist(DATASET, nodetype=np.int32, delimiter=' ')
    nres = list(nx.connected_components(nx_graph))
except Exception as e:
    print ("status=Exception: " + str(e))
    sys.exit(1)
print (fres)
print (nres)
if fres == nres:
    print ("status=Passed")
else:
    print ("status=Failed")
