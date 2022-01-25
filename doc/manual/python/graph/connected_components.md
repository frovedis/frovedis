% connected_components()

# NAME  

connected_components() - generates the connected components of an undirected graph 'G'.  

# SYNOPSIS  

    frovedis.graph.ConnectedComponents.connected_components(G, opt_level=2, hyb_threshold=0.4,
                                                            verbose=0, print_summary=False,
                                                            print_limit=5)  

# DESCRIPTION  
In graph theory, a component of an undirected graph is a connected subgraph that is not part 
of any larger connected subgraph. The components of any graph partition its vertices into 
disjoint sets, and are the induced subgraphs of those sets. A graph that is itself connected 
has exactly one component, consisting of the whole graph. Components are also sometimes 
called connected components.  

This module provides a client-server implementation, where the client application 
is a normal python program. The frovedis public connected_components method interface 
is almost same as NetworkX connected_components public method interface, but it doesn’t 
have any dependency on NetworkX. It can be used simply even if the system doesn’t have 
NetworkX installed. Thus, in this implementation, a python client can interact with a frovedis 
server sending the required python data for training at frovedis side. Python data is converted 
into frovedis compatible data internally and the python ML call is linked with the respective frovedis 
ML call to get the job done at frovedis server.  

Python side calls for connected_components() on the frovedis server. Once the connected components  
are computed for the given input graph data at the frovedis server, it returns a dictionary 
with keys as root-nodeid for each component and values as list of pairs of nodeid with its 
distance from root of the component to which the node belongs.  

## Detailed Description  

### 1. connected_components()  

__Parameters__  
_**G**_: An instance of networkx graph or frovedis graph. The graph must be undirected.  
_**opt\_level**_: Zero or a positive integer parameter that must be in range 0 to 2. It is an optimization 
parameter that is used for reducing computation time while generating the connected components. (Default: 1)  
- **When opt_level = 0**: this should only be used where systems have memory constraints. It is slowest.  
- **When opt_level = 1**: this is fastest. It uses comparatively large amount of memory.  
- **When opt_level = 2**: this is much better than **'opt_level = 0'**, but slightly slower than 
**opt_level = 1**. It optimizes the memory usage over **'opt_level = 1'**.  

_**hyb\_threshold**_: A double (float64) parameter that specifies a threshold value which performs optimization 
in generating connected components when the number of remaining nodes to be visited becomes less then 
this value. It optimizes the execution time. This parameter works only with **'opt_level = 2'**. It must be 
within the range 0 to 1. (Default: 0.4)  
For example, if it is 0.5, then optimization starts when number of remaining nodes to be visited is less 
than 50%.  
_**verbose**_: An integer parameter specifying the log level to use. Its value is 0 by 
default (INFO level). But it can be set to 1 (DEBUG level) or 2 (TRACE level) for getting 
training time logs from frovedis server.  
_**print_summary**_: A boolean parameter that specifies whether to print summary of connected 
components. (Default: False)  
_**print_limit**_: An integer parameter that specifies the maximum number of nodes info to be 
printed. (Default: 5)  

__Purpose__  
This method computes connected components of an undirected graph.  

For example,  
    
    # An undirected graph loaded from edgelist file
    import numpy as np
    import networkx as nx
    import frovedis.graph as fnx
    frov_graph = fnx.read_edgelist('input/cit-Patents_10.txt', nodetype = np.int64)
    ret = fnx.connected_components(frov_graph)
    for i in ret:
        print(i)
    
Output

    {1, 2, 3, 4, 5, 6}
    {7, 8, 9, 10, 11}
    {12, 13}

**When print_summary = True and print_limit = 5 (by default), then,**

    # An undirected graph loaded from edgelist file
    import numpy as np
    import networkx as nx
    import frovedis.graph as fnx
    frov_graph = fnx.read_edgelist('input/cit-Patents_10.txt', nodetype = np.int64)
    ret = fnx.connected_components(frov_graph, print_summary = True)
    for i in ret:
        print(i)

Output

    Number of connected components: 3
    Root with its count of nodes in each connected component: (root_id:count)
    1:6
    7:5
    12:2
    Nodes in which cc: (node_id:root_id)
    1:1
    2:1
    3:1
    4:1
    5:1
    ...
    Nodes dist: (node:level_from_root)
    1:0
    2:1
    3:1
    4:1
    5:1
    ...
    {1, 2, 3, 4, 5, 6}
    {7, 8, 9, 10, 11}
    {12, 13}
    
**When print_limit = 10, then,**  

    # An undirected graph loaded from edgelist file
    import numpy as np
    import networkx as nx
    import frovedis.graph as fnx
    frov_graph = fnx.read_edgelist('input/cit-Patents_10.txt', nodetype = np.int64)
    ret = fnx.connected_components(frov_graph, print_summary = True, print_limit = 10)
    for i in ret:
        print(i)

Output

    Number of connected components: 3
    Root with its count of nodes in each connected component: (root_id:count)
    1:6
    7:5
    12:2
    Nodes in which cc: (node_id:root_id)
    1:1
    2:1
    3:1
    4:1
    5:1
    6:1
    7:7
    8:7
    9:7
    10:7
    ...
    Nodes dist: (node:level_from_root)
    1:0
    2:1
    3:1
    4:1
    5:1
    6:1
    7:0
    8:1
    9:1
    10:1
    ...
    {1, 2, 3, 4, 5, 6}
    {7, 8, 9, 10, 11}
    {12, 13}

__Return Value__  
It yields an output as Sets of nodes for each connected components.  

## SEE ALSO  
graph, bfs, sssp, pagerank  
