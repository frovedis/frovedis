'''
exceptional case (Passed if exception)
directed graph - src with many reachable nodes     >> "Passed"
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
src = 22
exc1 = False
exc2 = False

#FROVEDIS
try:
    argvs = sys.argv
    argc = len(argvs)
    if (argc < 2):
        print ('Please give frovedis_server calling command as the first argument \n(e.g. "mpirun -np 2 -x /opt/nec/nosupport/frovedis/ve/bin/frovedis_server")')
        quit()
    FrovedisServer.initialize(argvs[1])
    
    frov_graph = fnx.read_edgelist(DATASET, nodetype=np.int32, delimiter=' ', \
                                       create_using=nx.DiGraph())
    fpath, fdist = fnx.single_source_shortest_path(frov_graph, src, \
                                                   return_distance=True)
    
    FrovedisServer.shut_down()
except Exception as e:
    sub_str = "sssp: source "+str(src)+" not found in input graph!"
    if sub_str in str(e):
        exc1 = True
    else:
        print ("status=Exception: " + str(e))
        sys.exit(1)

#NetworkX
try: 
    nx_graph = nx.read_edgelist(DATASET, nodetype=np.int32, delimiter=' ', \
                                create_using=nx.DiGraph())
    npath = nx.single_source_shortest_path(nx_graph, src)
    ndist = {k: float(len(v)-1) for k, v in npath.items()}
except Exception as e:
    sub_str = "Source "+str(src)+" not in G"
    if sub_str in str(e):
        exc2 = True
    else:
        print ("status=Exception: " + str(e))
        sys.exit(1)
if exc1 and exc2:
    print ("status=Passed")
else:
    print ("status=Failed")
