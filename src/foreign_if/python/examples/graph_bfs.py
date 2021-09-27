import sys
import numpy as np
from frovedis.exrpc.server import *
import networkx as nx
import frovedis.graph as fnx

# initializing the Frovedis server
argvs = sys.argv
argc = len(argvs)
if (argc < 2):
    print ('Please give frovedis_server calling command as the first argument \n(e.g. "mpirun -np 2 /opt/nec/frovedis/ve/bin/frovedis_server")')
    quit()

source = 1 
depth = 1
fname = 'input/cit-Patents_10.txt' 

FrovedisServer.initialize(argvs[1])
G_nx = nx.read_edgelist(fname, nodetype=np.int32, create_using=nx.DiGraph())
G = fnx.read_edgelist(fname, nodetype=np.int32, create_using=nx.DiGraph())
#G.debug_print()

print ("Frovedis BFS edges: ", list(fnx.bfs_edges(G, source, depth_limit=depth)))
print ("NetworkX BFS edges: ", list(nx.bfs_edges(G_nx, source, depth_limit=depth)))

print ("Edges in Frovedis bfs_tree: ", fnx.bfs_tree(G, source, depth_limit=depth).number_of_edges())
print ("Edges in NetworkX bfs_tree: ", nx.bfs_tree(G_nx, source, depth_limit=depth).number_of_edges())

print ("Frovedis bfs_predecessors: ", list(fnx.bfs_predecessors(G, source, depth_limit=depth)))
print ("NetworkX bfs_predecessors: ", list(nx.bfs_predecessors(G_nx, source, depth_limit=depth)))

print ("Frovedis bfs_successors: ", list(fnx.bfs_successors(G, source, depth_limit=depth)))
print ("NetworkX bfs_successors: ", list(nx.bfs_successors(G_nx, source, depth_limit=depth)))

print ("Frovedis descendants at distance:", fnx.descendants_at_distance(G, source, distance=depth))
print ("NetworkX descendants at distance:", nx.descendants_at_distance(G_nx, source, distance=depth))

# clean-up at the server side
G.release()
FrovedisServer.shut_down()
