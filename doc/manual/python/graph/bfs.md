% Breadth First Search  

# NAME  

Breadth First Search - Basic algorithms for breadth-first searching the nodes of a graph.  

# SYNOPSIS  
    
    frovedis.graph.traversal.bfs(G, source, depth_limit=None, opt_level=1, hyb_threshold=0.4, 
                                 verbose=0)
    
    frovedis.graph.traversal.bfs_edges(G, source, reverse=False, depth_limit=None, 
                                       sort_neighbors=None, opt_level=1, hyb_threshold=0.4, 
                                       verbose=0)
    
    frovedis.graph.traversal.bfs_tree(G, source, reverse=False, depth_limit=None, 
                                      sort_neighbors=None, opt_level=1, hyb_threshold=0.4, 
                                      verbose=0)
    
    frovedis.graph.traversal.bfs_predecessors(G, source, depth_limit=None, sort_neighbors=None,
                                              opt_level=1, hyb_threshold=0.4, verbose=0)
    
    frovedis.graph.traversal.bfs_successors(G, source, depth_limit=None, sort_neighbors=None,
                                            opt_level=1, hyb_threshold=0.4, verbose=0)
    
    frovedis.graph.traversal.descendants_at_distance(G, source, distance, opt_level=1, 
                                                     hyb_threshold=0.4, verbose=0)

# DESCRIPTION  
Breadth-first search is an algorithm for searching a tree data structure for a node that satisfies 
a given property. It starts at the tree root and explores all nodes at the present depth prior to 
moving on to the nodes at the next depth level.  

This module provides a client-server implementation, where the client application 
is a normal python program. The frovedis breadth first search public method interfaces are 
almost same as NetworkX breadth first serach public method interface, but it doesn’t have any 
dependency on NetworkX. It can be used simply even if the system doesn’t have NetworkX installed. 
Thus, in this implementation, a python client can interact with a frovedis server sending the 
required python data for training at frovedis side. Python data is converted into frovedis 
compatible data internally and the python ML call is linked with the respective frovedis 
ML call to get the job done at frovedis server.  

## Detailed Description     

### 1. bfs()  

__Parameters__  
_**G**_: An instance of networkx graph or frovedis graph. The graph can be directed or undirected.  
_**source**_: A positive integer parameter that specifies the starting node for breadth-first search. 
This method iterates over only those edges in the component, reachable from this node. It must be in 
range **[1, G.num_vertices]**.  
_**depth\_limit**_: A positive integer parameter that specifies the maximum search depth. (Default: None)  
When it is None (not specified explicitly), it will be set as maximum value for int64 datatype.  
_**opt\_level**_: Zero or a positive integer parameter that must be 0, 1 or 2. It is an optimization 
parameter that can be used for fast computation, reduce memory constraints during breadth first search. (Default: 1)  
- **When opt_level = 0**: this should only be used where systems have memory constraints. It is slowest.  
- **When opt_level = 1**: this is fastest. It uses comparatively large amount of memory.  
- **When opt_level = 2**: this is much better than **'opt_level = 0'** but slightly slower than **opt_level = 1**. 
It optimizes the memory usage over **'opt_level = 1'**.  

_**hyb\_threshold**_: A double (float64) parameter that specifies a threshold value which performs optimization 
during breadth first search when the number of remaining nodes to be visited becomes less then 
this value. It optimizes the execution time. This parameter works only with **'opt_level = 2'**. It must be 
within the range 0 to 1. (Default: 0.4)  
For example, if it is 0.5, then optimization starts when number of remaining nodes to be visited is less 
than 50%.  
_**verbose**_: An integer parameter specifying the log level to use. Its value is 0 by 
default (INFO level). But it can be set to 1 (DEBUG level) or 2 (TRACE level) for getting 
training time logs from frovedis server.  

__Purpose__  
This method computes bfs traversal path of a graph.  

**This method is not present in Networkx. It is only provided in frovedis.**  

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
    G = fnx.read_edgelist('input/cit-Patents_10.txt', nodetype = np.int32, 
                          create_using = nx.DiGraph())
    print ("Frovedis BFS : ", list(fnx.bfs(G, source = 1, depth_limit = 1)))

Output

    Frovedis BFS :  [{1: [1]}, {2: [1, 2]}, {3: [1, 3]}, {4: [1, 4]}, {5: [1, 5]}, {6: [1, 6]}]

**In case, we started from node 7 i.e source = 7 in the directed graph 'G'**  

    print ("Frovedis BFS : ", list(fnx.bfs(G, source = 7, depth_limit = 1)))

Output

    Frovedis BFS :  [{7: [7]}, {8: [7, 8]}, {9: [7, 9]}, {10: [7, 10]}, {11: [7, 11]}]

__Return Value__  
It returns a dictionary with keys as destination node-id and values as corresponding 
traversal path from the source. In case any node in input graph not reachable from the source, 
it would not be included in the resultant dictionary.  

### 2. bfs_edges()  

__Parameters__  
_**G**_: An instance of networkx graph or frovedis graph. The graph can be directed or undirected.  
_**source**_: A positive integer parameter that specifies the starting node for breadth-first search. 
This method iterates over only those edges in the component, reachable from this node. It must be in 
range **[1, G.num_vertices]**.  
_**reverse**_: A boolean parameter, in general, specifies the direction of traversal (forward or backward) 
if **'G'** is a directed graph. Currently, it can only traverse in forward direction **(reverse = True 
not supported yet)**. (Default: False)  
_**depth\_limit**_: A positive integer parameter that specifies the maximum search depth. (Default: None)  
When it is None (not specified explicitly), it will be set as maximum value for int64 datatype.  
_**sort\_neighbors**_: An unused parameter. (Default: None)  
_**opt\_level**_: Zero or a positive integer parameter that must be 0, 1 or 2. It is an optimization 
parameter that can be used for fast computation, reduce memory constraints during breadth first search. (Default: 1)  
- **When opt_level = 0**: this should only be used where systems have memory constraints. It is slowest.  
- **When opt_level = 1**: this is fastest. It uses comparatively large amount of memory.  
- **When opt_level = 2**: this is much better than **'opt_level = 0'** but slightly slower than **opt_level = 1**. 
It optimizes the memory usage over **'opt_level = 1'**.  

_**hyb\_threshold**_: A double (float64) parameter that specifies a threshold value which performs optimization 
during breadth first search when the number of remaining nodes to be visited becomes less then 
this value. It optimizes the execution time. This parameter works only with **'opt_level = 2'**. It must be 
within the range 0 to 1. (Default: 0.4)  
For example, if it is 0.5, then optimization starts when number of remaining nodes to be visited is less 
than 50%.  
_**verbose**_: An integer parameter specifying the log level to use. Its value is 0 by 
default (INFO level). But it can be set to 1 (DEBUG level) or 2 (TRACE level) for getting 
training time logs from frovedis server.  

__Purpose__  
This method iterates over edges in a breadth-first-search starting at source.  

The parameter: "sort_neighbors" is simply kept to make the method uniform to NetworkX bfs_edges method. 
It is not used anywhere within the frovedis implementation.  

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
    G = fnx.read_edgelist('input/cit-Patents_10.txt', nodetype = np.int32, 
                          create_using = nx.DiGraph())
    print("Frovedis BFS edges: ", list(fnx.bfs_edges(G, source = 1, depth_limit = 1)))

Output

    Frovedis BFS edges:  [(1, 2), (1, 3), (1, 4), (1, 5), (1, 6)]

**In case, we started from node 7 i.e source = 7 in the directed graph 'G'**  

    print("Frovedis BFS edges: ", list(fnx.bfs_edges(G, source = 7, depth_limit = 1)))

Output

    Frovedis BFS edges:  [(7, 8), (7, 9), (7, 10), (7, 11)]

__Return Value__  
It yields edges resulting from the breadth-first search.  

### 3. bfs_tree()  

__Parameters__  
_**G**_: An instance of networkx graph or frovedis graph. The graph can be directed or undirected.  
_**source**_: A positive integer parameter that specifies the starting node for breadth-first search. 
This method iterates over only those edges in the component, reachable from this node. It must be in 
range **[1, G.num_vertices]**.  
_**reverse**_: A boolean parameter, in general, specifies the direction of traversal (forward or backward) 
if **'G'** is a directed graph. Currently, it can only traverse in forward direction **(reverse = True 
not supported yet)**. (Default: False)  
_**depth\_limit**_: A positive integer parameter that specifies the maximum search depth. (Default: None)  
When it is None (not specified explicitly), it will be set as maximum value for int64 datatype.  
_**sort\_neighbors**_: An unused parameter. (Default: None)  
_**opt\_level**_: Zero or a positive integer parameter that must be 0, 1 or 2. It is an optimization 
parameter that can be used for fast computation, reduce memory constraints during breadth first search. (Default: 1)  
- **When opt_level = 0**: this should only be used where systems have memory constraints. It is slowest.  
- **When opt_level = 1**: this is fastest. It uses comparatively large amount of memory.  
- **When opt_level = 2**: this is much better than **'opt_level = 0'** but slightly slower than **opt_level = 1**. 
It optimizes the memory usage over **'opt_level = 1'**.  

_**hyb\_threshold**_: A double (float64) parameter that specifies a threshold value which performs optimization 
during breadth first search when the number of remaining nodes to be visited becomes less then 
this value. It optimizes the execution time. This parameter works only with **'opt_level = 2'**. It must be 
within the range 0 to 1. (Default: 0.4)  
For example, if it is 0.5, then optimization starts when number of remaining nodes to be visited is less 
than 50%.  
_**verbose**_: An integer parameter specifying the log level to use. Its value is 0 by 
default (INFO level). But it can be set to 1 (DEBUG level) or 2 (TRACE level) for getting 
training time logs from frovedis server.  

__Purpose__  
This method constructs a directed tree from of a breadth-first-search starting at source.  

The parameter: "sort_neighbors" is simply kept to make the method uniform to NetworkX bfs_tree method. 
It is not used anywhere within the frovedis implementation.  

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
    G = fnx.read_edgelist('input/cit-Patents_10.txt', nodetype = np.int32, 
                          create_using = nx.DiGraph())
    fnx.bfs_tree(G, source = 1, depth_limit = 1)
    
    # To check edges in newly created networkx directed graph
    print("Edges in Frovedis bfs_tree: ")
    print(fnx.bfs_tree(G, source = 1, depth_limit = 1).number_of_edges())

Output
    
    Edges in Frovedis bfs_tree:  
    5

**In case, we started from node 7 i.e source = 7 in the directed graph 'G'**  

    fnx.bfs_tree(G, source = 7, depth_limit = 1)
    
    # To check edges in newly created networkx directed graph
    print("Edges in Frovedis bfs_tree: ")
    print(fnx.bfs_tree(G, source = 7, depth_limit = 1).number_of_edges())

Output

    Edges in Frovedis bfs_tree:  
    4

__Return Value__  
It returns a new Networkx.DiGraph instance.  

### 4. bfs_predecessors()

__Parameters__  
_**G**_: An instance of networkx graph or frovedis graph. The graph can be directed or undirected.  
_**source**_: A positive integer parameter that specifies the starting node for breadth-first search. 
This method iterates over only those edges in the component, reachable from this node. It must be in 
range **[1, G.num_vertices]**.  
_**depth\_limit**_: A positive integer parameter that specifies the maximum search depth. (Default: None)  
When it is None (not specified explicitly), it will be set as maximum value for int64 datatype.  
_**sort\_neighbors**_: An unused parameter. (Default: None)  
_**opt\_level**_: Zero or a positive integer parameter that must be 0, 1 or 2. It is an optimization 
parameter that can be used for fast computation, reduce memory constraints during breadth first search. (Default: 1)  
- **When opt_level = 0**: this should only be used where systems have memory constraints. It is slowest.  
- **When opt_level = 1**: this is fastest. It uses comparatively large amount of memory.  
- **When opt_level = 2**: this is much better than **'opt_level = 0'** but slightly slower than **opt_level = 1**. 
It optimizes the memory usage over **'opt_level = 1'**.  

_**hyb\_threshold**_: A double (float64) parameter that specifies a threshold value which performs optimization 
during breadth first search when the number of remaining nodes to be visited becomes less then 
this value. It optimizes the execution time. This parameter works only with **'opt_level = 2'**. It must be 
within the range 0 to 1. (Default: 0.4)  
For example, if it is 0.5, then optimization starts when number of remaining nodes to be visited is less 
than 50%.  
_**verbose**_: An integer parameter specifying the log level to use. Its value is 0 by 
default (INFO level). But it can be set to 1 (DEBUG level) or 2 (TRACE level) for getting 
training time logs from frovedis server.  

__Purpose__  
This method provides predecessors for each node in breadth-first-search from source.

The parameter: "sort_neighbors" is simply kept to make the method uniform to NetworkX bfs_tree method. 
It is not used anywhere within the frovedis implementation.  

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
    G = fnx.read_edgelist('input/cit-Patents_10.txt', nodetype = np.int32, 
                          create_using = nx.DiGraph())
    print("Frovedis bfs_predecessors: ")
    print(list(fnx.bfs_predecessors(G, source = 1, depth_limit = 1)))

Output
    
    Frovedis bfs_predecessors:  
    [(2, 1), (3, 1), (4, 1), (5, 1), (6, 1)]

Here, node 2, node 3, node 4, node 5 and node 6 were having source = 1 as thier predecessor.  

**In case, we started from node 7 i.e source = 7 in the directed graph 'G'**  

    print("Frovedis bfs_predecessors: ")
    print(list(fnx.bfs_predecessors(G, source = 7, depth_limit = 1)))

Output

    Frovedis bfs_predecessors:  
    [(8, 7), (9, 7), (10, 7), (11, 7)]

Here, node 8, node 9, node 10 and node 11 were having source = 7 as thier predecessor.  

__Return Value__  
It yields predecessors for each node in form of **(node, predecessor)** iterator.  

### 5. bfs_successors()

__Parameters__  
_**G**_: An instance of networkx graph or frovedis graph. The graph can be directed or undirected.  
_**source**_: A positive integer parameter that specifies the starting node for breadth-first search. 
This method iterates over only those edges in the component, reachable from this node. It must be in 
range **[1, G.num_vertices]**.  
_**depth\_limit**_: A positive integer parameter that specifies the maximum search depth. (Default: None)  
When it is None (not specified explicitly), it will be set as maximum value for int64 datatype.  
_**sort\_neighbors**_: An unused parameter. (Default: None)  
_**opt\_level**_: Zero or a positive integer parameter that must be 0, 1 or 2. It is an optimization 
parameter that can be used for fast computation, reduce memory constraints during breadth first search. (Default: 1)  
- **When opt_level = 0**: this should only be used where systems have memory constraints. It is slowest.  
- **When opt_level = 1**: this is fastest. It uses comparatively large amount of memory.  
- **When opt_level = 2**: this is much better than **'opt_level = 0'** but slightly slower than **opt_level = 1**. 
It optimizes the memory usage over **'opt_level = 1'**.  

_**hyb\_threshold**_: A double (float64) parameter that specifies a threshold value which performs optimization 
during breadth first search when the number of remaining nodes to be visited becomes less then 
this value. It optimizes the execution time. This parameter works only with **'opt_level = 2'**. It must be 
within the range 0 to 1. (Default: 0.4)  
For example, if it is 0.5, then optimization starts when number of remaining nodes to be visited is less 
than 50%.  
_**verbose**_: An integer parameter specifying the log level to use. Its value is 0 by 
default (INFO level). But it can be set to 1 (DEBUG level) or 2 (TRACE level) for getting 
training time logs from frovedis server.  

__Purpose__  
This method provides successors for each node in breadth-first-search from source.

The parameter: "sort_neighbors" is simply kept to make the method uniform to NetworkX bfs_tree method. 
It is not used anywhere within the frovedis implementation.  

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
    G = fnx.read_edgelist('input/cit-Patents_10.txt', nodetype = np.int32, 
                          create_using = nx.DiGraph())
    print("Frovedis bfs_successors: ", list(fnx.bfs_successors(G, source = 1, depth_limit = 1)))

Output
    
    Frovedis bfs_successors:  [(1, [2, 3, 4, 5, 6])]

Here, for source = 1, node 2, node 3, node 4, node 5 and node 6 were its successor.  

**In case, we started from node 7 i.e source = 7 in the directed graph 'G'**  

    print("Frovedis bfs_successors: ", list(fnx.bfs_successors(G, source = 7, depth_limit = 1)))

Output

    Frovedis bfs_successors:  [(7, [8, 9, 10, 11])]

Here, for source = 7, node 8, node 9, node 10 and node 11 were its successor.  

__Return Value__  
It yields successors for each node in form of **(node, successor)** iterator.  

### 6. descendants_at_distance()

__Parameters__  
_**G**_: An instance of networkx graph or frovedis graph. The graph can be directed or undirected.  
_**source**_: A positive integer parameter that specifies the starting node for breadth-first search. 
This method iterates over only those edges in the component, reachable from this node. It must be in 
range **[1, G.num_vertices]**.  
_**distance**_: A positive integer parameter that specifies the distance of the wanted nodes from source.  
_**opt\_level**_: Zero or a positive integer parameter that must be 0, 1 or 2. It is an optimization 
parameter that can be used for fast computation, reduce memory constraints during breadth first search. (Default: 1)  
- **When opt_level = 0**: this should only be used where systems have memory constraints. It is slowest.  
- **When opt_level = 1**: this is fastest. It uses comparatively large amount of memory.  
- **When opt_level = 2**: this is much better than **'opt_level = 0'** but slightly slower than **opt_level = 1**. 
It optimizes the memory usage over **'opt_level = 1'**.  

_**hyb\_threshold**_: A double (float64) parameter that specifies a threshold value which performs optimization 
during breadth first search when the number of remaining nodes to be visited becomes less then 
this value. It optimizes the execution time. This parameter works only with **'opt_level = 2'**. It must be 
within the range 0 to 1. (Default: 0.4)  
For example, if it is 0.5, then optimization starts when number of remaining nodes to be visited is less 
than 50%.  
_**verbose**_: An integer parameter specifying the log level to use. Its value is 0 by 
default (INFO level). But it can be set to 1 (DEBUG level) or 2 (TRACE level) for getting 
training time logs from frovedis server.  

__Purpose__  
This method returns all nodes at a fixed distance from source in G.  

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
    G = fnx.read_edgelist('input/cit-Patents_10.txt', nodetype = np.int32, 
                          create_using = nx.DiGraph())
    print("Frovedis descendants at distance:")
    print(fnx.descendants_at_distance(G, source = 1, distance = 1))

Output
    
    Frovedis descendants at distance: 
    {2, 3, 4, 5, 6}

**In case, we started from node 7 i.e source = 7 in the directed graph 'G'**  

    print("Frovedis descendants at distance:")
    print(fnx.descendants_at_distance(G, source = 7, distance = 1))

Output
    
    Frovedis descendants at distance: 
    {8, 9, 10, 11}

__Return Value__  
It returns an instance of Set having the descendants of source in graph 'G' at the given distance from source.   

## SEE ALSO  
graph, pagerank, connected_components, single_source_shortest_path  