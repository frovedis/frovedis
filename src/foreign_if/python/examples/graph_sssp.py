import sys
import numpy as np
from frovedis.exrpc.server import *
import frovedis.networkx as fnx

# initializing the Frovedis server
argvs = sys.argv
argc = len(argvs)
if (argc < 2):
    print ('Please give frovedis_server calling command as the first argument \n(e.g. "mpirun -np 2 -x /opt/nec/nosupport/frovedis/ve/bin/frovedis_server")')
    quit()

FrovedisServer.initialize(argvs[1])
frov_graph = fnx.read_edgelist('input/cit-Patents_10.txt')
frov_graph.debug_print()

#returns dict of lists containing shortest path from source to all other nodes
source = 4
ret = fnx.single_source_shortest_path(frov_graph, source) 
print(ret)

frov_graph.release()
# Shutting down the Frovedis server
FrovedisServer.shut_down()
