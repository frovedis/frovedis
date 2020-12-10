import sys
import numpy as np
from frovedis.exrpc.server import *
import frovedis.graph as fnx
import networkx as nx

# initializing the Frovedis server
argvs = sys.argv
argc = len(argvs)
if (argc < 2):
    print ('Please give frovedis_server calling command as the first argument \n(e.g. "mpirun -np 2 -x /opt/nec/nosupport/frovedis/ve/bin/frovedis_server")')
    quit()

source = 1

fname = 'input/cit-Patents_10.txt' 

FrovedisServer.initialize(argvs[1])
#frov_graph = fnx.read_edgelist(fname, nodetype=np.int32)
frov_graph = fnx.read_edgelist(fname, nodetype=np.int32, create_using=nx.DiGraph())
frov_graph.debug_print()

#returns dict of lists containing shortest path from source to all other nodes
ret = fnx.bfs(frov_graph, source) 
print("Frovedis bfs traversal path: ")
print(ret)

frov_graph.release()
# Shutting down the Frovedis server
FrovedisServer.shut_down()
