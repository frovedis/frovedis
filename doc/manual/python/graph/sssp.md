% single_source_shortest_path()

# NAME  

single_source_shortest_path() - finds the shortest path from source to all reachable nodes in 
graph 'G'. Here, graph 'G' maybe a directed or undirected graph.  

# SYNOPSIS  

    frovedis.graph.traversal.single_source_shortest_path(G, source, return_distance=False, 
                                                         verbose=0)  

# DESCRIPTION  
In graph theory, the shortest path problem is the problem of finding a path between two 
vertices (or nodes) in a graph such that the sum of the weights of its constituent edges is minimized.

This module provides a client-server implementation, where the client application 
is a normal python program. The frovedis public single_source_shortest_path method interface 
is almost same as NetworkX single_source_shortest_path public method interface, but it doesn’t 
have any dependency on NetworkX. It can be used simply even if the system doesn’t have 
NetworkX installed. Thus, in this implementation, a python client can interact with a frovedis 
server sending the required python data for training at frovedis side. Python data is converted 
into frovedis compatible data internally and the python ML call is linked with the respective frovedis 
ML call to get the job done at frovedis server.  

Python side calls for single_source_shortest_path() on the frovedis server. Once the shortest distances 
are computed for the given input graph data at the frovedis server, it returns a dictionary of 
lists containing shortest path from source to all other nodes to the client python program.  

## Detailed Description  

### 1. single_source_shortest_path()  

__Parameters__  
_**G**_: An instance of networkx graph or frovedis graph. The graph can be directed or undirected.  
_**source**_: A positive integer parameter that specifies the starting node for the path. It must be 
in the range **[1, G.num_vertices]**.  
_**return_distance**_: A boolean parameter if set to True, will return the shortest distances from 
source to all nodes along with traversal path. Otherwise, it will return only the traversal path. (Default: False)  
_**verbose**_: An integer parameter specifying the log level to use. Its value is 0 by 
default (INFO level). But it can be set to 1 (DEBUG level) or 2 (TRACE level) for getting 
training time logs from frovedis server.  

__Purpose__  
This method computes the shortest path between source and all other nodes reachable from source.  

For example,  

FILE: cit-Patents.txt  

1 2  
1 3  
1 4  
1 5  
1 6  
7 8  
7 9  
7 10  
7 11  
12 13  

Here, the above file contains a list of edges between the nodes of graph G.  

    # A directed graph loaded from edgelist file
    import numpy as np
    import networkx as nx
    import frovedis.graph as fnx
    frov_graph = fnx.read_edgelist('input/cit-Patents_10.txt', nodetype = np.int32, 
                          create_using = nx.DiGraph())
    # use verbose = 1 for detailed inforrmation                      
    path, dist = fnx.single_source_shortest_path(frov_graph, source = 1, return_distance=True
                                                 verbose = 1) 
    print("Frovedis sssp traversal path: ")
    print(list(path))
    print("Frovedis sssp traversal distance from source: ")
    print(dist)

Output
    
    sssp computation time: 0.001 sec.
    sssp res conversion time: 0.000 sec.
    Frovedis sssp traversal path: 
    [{1: [1]}, {2: [1, 2]}, {3: [1, 3]}, {4: [1, 4]}, {5: [1, 5]}, {6: [1, 6]}]
    Frovedis sssp traversal distance from source: 
    {1: 0.0, 2: 1.0, 3: 1.0, 4: 1.0, 5: 1.0, 6: 1.0}

**Incase we had started with node 2 i.e source = 2, then,**

    # A directed graph loaded from edgelist file
    import numpy as np
    import networkx as nx
    import frovedis.graph as fnx
    frov_graph = fnx.read_edgelist('input/cit-Patents_10.txt', nodetype = np.int32, 
                          create_using = nx.DiGraph())
    path, dist = fnx.single_source_shortest_path(frov_graph, source = 2, return_distance=True) 
    print("Frovedis sssp traversal path: ")
    print(list(path))
    print("Frovedis sssp traversal distance from source: ")
    print(dist)

Output

    Frovedis sssp traversal path:
    [{2: [2]}, {1: [2, 1]}, {3: [2, 1, 3]}, {4: [2, 1, 4]}, {5: [2, 1, 5]}, {6: [2, 1, 6]}]
    Frovedis sssp traversal distance from source:
    {2: 0.0, 1: 1.0, 3: 2.0, 4: 2.0, 5: 2.0, 6: 2.0}
    
**When source = 2 and return_distances = False,**  

    # A directed graph loaded from edgelist file
    import numpy as np
    import networkx as nx
    import frovedis.graph as fnx
    frov_graph = fnx.read_edgelist('input/cit-Patents_10.txt', nodetype = np.int32, 
                          create_using = nx.DiGraph())
    # return_distances = False, by default
    path= fnx.single_source_shortest_path(frov_graph, source = 2) 
    print("Frovedis sssp traversal path: ")
    print(list(path))

Output

    Frovedis sssp traversal path:
    [{2: [2]}, {1: [2, 1]}, {3: [2, 1, 3]}, {4: [2, 1, 4]}, {5: [2, 1, 5]}, {6: [2, 1, 6]}]

__Return Value__  
It returns a dictionary of lists containing shortest path from source to all other nodes.  

## SEE ALSO  
graph, bfs, connected_components, pagerank  