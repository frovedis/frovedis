'''
undirected graph - src at begining (min id of vertext)   >> "Passed"
bfs_edges
depth = None
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
src = 10
depth = None
#FROVEDIS
try:
    argvs = sys.argv
    argc = len(argvs)
    if (argc < 2):
        print ('Please give frovedis_server calling command as the first argument \n(e.g. "mpirun -np 2 -x /opt/nec/nosupport/frovedis/ve/bin/frovedis_server")')
        quit()
    FrovedisServer.initialize(argvs[1])
    
    frov_graph = fnx.read_edgelist(DATASET, nodetype=np.int32, delimiter=' ')
    fres = set(fnx.bfs_edges(frov_graph, src, depth_limit=depth))
    
    FrovedisServer.shut_down()
except Exception as e:
    print ("status=Exception: " + str(e))
    sys.exit(1)

#NetworkX
try: 
    nx_graph = nx.read_edgelist(DATASET, nodetype=np.int32, delimiter=' ')
    nres = set(nx.bfs_edges(nx_graph, src, depth_limit=depth))
except Exception as e:
    print ("status=Exception: " + str(e))
    sys.exit(1)
print (fres)
print (nres)
if len(fres - nres) == 0:
    print ("status=Passed")
else:
    print ("status=Failed")
