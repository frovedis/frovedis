import sys
import numpy as np
from frovedis.exrpc.server import *
import frovedis.graph as fnx
import networkx as nx

# initializing the Frovedis server
argvs = sys.argv
argc = len(argvs)
if argc < 2:
    print ('Please give frovedis_server calling command as the first argument \n(e.g. "mpirun -np 2 /opt/nec/frovedis/ve/bin/frovedis_server")')
    quit()

fname = 'input/cit-Patents_10.txt'
FrovedisServer.initialize(argvs[1])
frov_graph = fnx.read_edgelist(fname, nodetype=np.int64)
frov_graph.debug_print()

ret = fnx.connected_components(frov_graph, print_summary=True, verbose=1)
#ret = fnx.connected_components(frov_graph)
for i in ret:
  print(i)
frov_graph.release()

# Shutting down the Frovedis server
FrovedisServer.shut_down()
