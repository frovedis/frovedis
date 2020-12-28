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
path, dist = fnx.single_source_shortest_path(frov_graph, source, return_distance=True, verbose=1) 
print("Frovedis sssp traversal path: ")
print(list(path))
print("Frovedis sssp traversal distance from source: ")
print(dist)

#nx_graph = nx.read_edgelist(fname, nodetype=np.int32)
nx_graph = nx.read_edgelist(fname, nodetype=np.int32, create_using=nx.DiGraph())
ret = nx.single_source_shortest_path(nx_graph, source) 
print("Networkx sssp traversal path: ")
print(ret)

frov_graph.release()
# Shutting down the Frovedis server
FrovedisServer.shut_down()
