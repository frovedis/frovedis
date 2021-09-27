import sys
import numpy as np
import frovedis.graph as fnx
from frovedis.exrpc.server import FrovedisServer 

# initializing the Frovedis server
argvs = sys.argv
argc = len(argvs)
if (argc < 2):
    print ('Please give frovedis_server calling command as the first argument \n(e.g. "mpirun -np 2 /opt/nec/frovedis/ve/bin/frovedis_server")')
    quit()
FrovedisServer.initialize(argvs[1])

# graph loading from edgelist file
frov_graph = fnx.read_edgelist('input/cit-Patents_10.txt')

#graph operations
frov_graph.save("out/graph.dat")
frov_graph.release()
frov_graph.load_text("out/graph.dat")
frov_graph.debug_print()

# pagerank demo
ranks = fnx.pagerank(frov_graph)
print ("FROV PR: ", ranks)

frov_graph.release()
# Shutting down the Frovedis server
FrovedisServer.shut_down()
