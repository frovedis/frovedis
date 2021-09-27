'''
directed graph - ranking order confirm >> "Passed"
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
    
    frov_graph = fnx.read_edgelist(DATASET, nodetype=np.int32, delimiter=' ', \
                                       create_using=nx.DiGraph())
    frov_ranks = fnx.pagerank(frov_graph)
    df1 = pd.DataFrame({ "node": frov_ranks.keys(),
                         "rank": [round(e, 7) for e in frov_ranks.values()]
                       }) \
            .sort_values(by = ["rank", "node"]) \
            .head(10)
    FrovedisServer.shut_down()
except Exception as e:
    print ("status=Exception: " + str(e))
    sys.exit(1)

#NetworkX
try: 
    nx_graph = nx.read_edgelist(DATASET, nodetype=np.int32, delimiter=' ', \
                                create_using=nx.DiGraph())
    nx_ranks = nx.pagerank(nx_graph)
    df2 = pd.DataFrame({ "node": nx_ranks.keys(),
                         "rank": [round(e, 7) for e in nx_ranks.values()]
                       }) \
            .sort_values(by = ["rank", "node"]) \
            .head(10)
except Exception as e:
    print ("status=Exception: " + str(e))
    sys.exit(1)
print (df1) 
print (df2) 
if list(df1.node.values) == list(df2.node.values):
    print ("status=Passed")
else:
    print ("status=Failed")
