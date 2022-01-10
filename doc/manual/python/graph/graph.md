% Graph

# NAME  

Graph - This class implements a directed or undirected frovedis graph.  

# SYNOPSIS  

    frovedis.graph.graph.Graph(nx_graph=None)  

## Public Member Functions  
load(nx_graph)  
load_csr(smat)  
debug_print()  
release()  
clear()  
number_of_edges()  
number_of_nodes()  
save(fname)  
load_text(fname)  
to_networkx_graph()  

# DESCRIPTION  


## Detailed Description  

### 1. Graph()  

__Parameters__  
**_nx\_graph_**: Data to initialize graph. Here data provided is a scipy sparse csr_matrix or 
a networkx.Graph instance. It loads such data to create a frovedis graph. (Default: None)  
When it is None, an empty frovedis graph is created.  

__Purpose__  
It initializes a Frovedis Graph object having nodes and edges. It can create both directed or 
undirected graph.  

For example,   

**a) When loading an undirected networkx.Graph as input,**  

    import frovedis.graph as fnx
    import networkx as gnx
    net_graph = gnx.read_edgelist('input/cit-Patents_10.txt')
    frov_graph = fnx.Graph(net_graph)

**b) When loading a directed networkx.Graph as input,**  

    import frovedis.graph as fnx
    import networkx as gnx
    net_graph = gnx.read_edgelist('input/cit-Patents_10.txt', create_using = gnx.DiGraph())
    frov_graph = fnx.Graph(net_graph)

**c) When loading a scipy sparse csr_matrix as input to create undirected frovedis graph,**  

    #Here it creates an undirected graph
    import frovedis.graph as fnx
    from scipy.sparse import csr_matrix
    data = [1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1]
    row = [0,0,0,0,0,1,2,3,4,5,6,6,6,6,7,8,9,10,11,12]
    col = [1,2,3,4,5,0,0,0,0,0,7,8,9,10,6,6,6,6,12,11]
    csr_mat = csr_matrix((data, (row, col)), shape = (13, 13))
    frov_graph = fnx.Graph(csr_mat)

**d) Again, when loading a scipy sparse csr_matrix as input to create directed frovedis graph,**  

    #Here it creates a directed graph
    import frovedis.graph as fnx
    from scipy.sparse import csr_matrix
    data = [1,1,1,1,1,1,1,1,1,1]
    row = [0,0,0,0,0,6,6,6,6,11]
    col = [1,2,3,4,5,7,8,9,10,12]
    csr_mat = csr_matrix((data, (row, col)), shape = (13, 13))
    frov_graph = fnx.Graph(csr_mat)

__Return Value__  
It simply returns "self" reference.  

### 2. load(nx_graph)  

__Parameters__  
**_nx\_graph_**: Data to initialize graph. Here graph data provided is a networkx.Graph 
instance.  

__Purpose__  
It loads a networkx graph to create a frovedis graph. Here, the loaded graph can be directed or undirected.  

For example,  

    import frovedis.graph as fnx
    import networkx as gnx
    net_graph = gnx.read_edgelist('input/cit-Patents_10.txt')
    frov_graph = fnx.Graph().load(nx_graph = net_graph)

__Return Value__  
It simply returns "self" reference.  

### 3. load_csr(smat)  

__Parameters__  
**_smat_**: Data to initialize graph. Here graph data provided is a scipy sparse csr_matrix.  

__Purpose__  
It loads frovedis graph from a scipy csr_matrix. Here, depending on the scipy csr_matrix data, 
the final graph maybe directed or undirected.  

For example,  
    
    #Loading a scipy csr_matrix to create an undirected frovedis graph
    import frovedis.graph as fnx
    from scipy.sparse import csr_matrix
    data = [1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1]
    row = [0,0,0,0,0,1,2,3,4,5,6,6,6,6,7,8,9,10,11,12]
    col = [1,2,3,4,5,0,0,0,0,0,7,8,9,10,6,6,6,6,12,11]
    csr_mat = csr_matrix((data, (row, col)), shape = (13, 13))
    frov_graph = fnx.Graph().load_csr(smat = csr_mat)

    #Loading a scipy csr_matrix to create a directed frovedis graph
    import frovedis.graph as fnx
    from scipy.sparse import csr_matrix
    data = [1,1,1,1,1,1,1,1,1,1]
    row = [0,0,0,0,0,6,6,6,6,11]
    col = [1,2,3,4,5,7,8,9,10,12]
    csr_mat = csr_matrix((data, (row, col)), shape = (13, 13))
    frov_graph = fnx.Graph().load_csr(smat = csr_mat)

__Return Value__  
It simply returns "self" reference.  

### 4. debug_print()  

__Purpose__  
It shows graph information on the client side and server side user terminal. It is mainly used for 
debugging purpose.  

For example,  

    frov_graph.debug_print()

Output on client side  

    Num of edges:  20
    Num of vertices:  13

It displays information such as number of edges and vertices.

Output on server side

    Num of edges:  20
    Num of vertices:  13
    is directed: 0
    is weighted: 0
    in-degree:
    5 1 1 1 1 1 4 1 1 1 1 1 1
    out-degree:
    5 1 1 1 1 1 4 1 1 1 1 1 1
    adjacency matrix:
    0 1 1 1 1 1 0 0 0 0 0 0 0
    1 0 0 0 0 0 0 0 0 0 0 0 0
    1 0 0 0 0 0 0 0 0 0 0 0 0
    1 0 0 0 0 0 0 0 0 0 0 0 0
    1 0 0 0 0 0 0 0 0 0 0 0 0
    1 0 0 0 0 0 0 0 0 0 0 0 0
    0 0 0 0 0 0 0 1 1 1 1 0 0
    0 0 0 0 0 0 1 0 0 0 0 0 0
    0 0 0 0 0 0 1 0 0 0 0 0 0
    0 0 0 0 0 0 1 0 0 0 0 0 0
    0 0 0 0 0 0 1 0 0 0 0 0 0
    0 0 0 0 0 0 0 0 0 0 0 0 1
    0 0 0 0 0 0 0 0 0 0 0 1 0

It displays information such as number of edges and vertices, directed ('0' for No, '1' for Yes) , incoming 
links, outgoing links and an adjacency matrix. Currently, it shows information about an undirected 
frovedis graph.  

__Return Value__  
It returns nothing.  

### 5. release()  

__Purpose__  
It can be used to release the in-memory model at frovedis server.  

For example,  

    frov_graph.release()  

This will remove the graph model, model-id present on server, along with releasing server side memory.  

__Return Value__  
It returns nothing.  

### 6. clear()  

__Purpose__  
It can be used to release the in-memory model at frovedis server. This method is an alias to release().  

For example,  

    frov_graph.clear()  

This will remove the graph model, model-id present on server, along with releasing server side memory.  

__Return Value__  
It returns nothing.  

### 7. number_of_edges()  

__Purpose__  
It is used to fetch the number of edges present in the directed or undirected frovedis graph.  

For example,  

    edge_count = frov_graph.number_of_edges()
    print('Number of edges: ', edge_count)
    
Output

    Number of edges: 20

__Return Value__  
It returns a long (int64) type value specifying the number of edges.  

### 8. number_of_nodes()  

__Purpose__  
It is used to fetch the number of nodes/vertices present in the directed or undirected frovedis graph.  

For example,  

    nodes_count = frov_graph.number_of_nodes()
    print('Number of nodes: ', nodes_count)
    
Output

    Number of nodes: 13

__Return Value__  
It returns a long (int64) type value specifying the number of node/vertices.  

### 9. save(fname)  

__Parameters__  
**_fname_**: A string object containing the name of the file on which the adjacency matrix is to be saved.  

__Purpose__  
On success, it writes the adjacency matrix information in the specified file. Otherwise, it throws an exception.  

For example,  

    # To save the frovedis graph  
    frov_graph.save("./out/FrovGraph")  
    
This will save the frovedis grpah on the path '/out/FrovGraph'. It would raise exception if the directory 
already exists with same name.  

__Return Value__  
It returns nothing.  

### 10. load_text(fname)  

__Parameters__  
**_fname_**: A string object containing the name of the file having adjacency matrix information 
to be loaded.  

__Purpose__  
It loads a frovedis graph from the specified file path (having adjacency matrix data).  

For example,  

    # To load the frovedis graph  
    frov_graph.load_text("./out/FrovGraph")  

__Return Value__  
It simply returns "self" reference.

### 11. to_networkx_graph()  

__Purpose__  
It is used to convert a frovedis graph into a networkx graph.  

For example,  

    nx_graph = frov_graph.to_networkx_graph()
    print(nx_graph.adj)

Output

    {0: {1: {'weight': 1.0}, 2: {'weight': 1.0}, 3: {'weight': 1.0}, 4: {'weight': 1.0}, 
    5: {'weight': 1.0}}, 1: {0: {'weight': 1.0}}, 2: {0: {'weight': 1.0}}, 
    3: {0: {'weight': 1.0}}, 4: {0: {'weight': 1.0}}, 5: {0: {'weight': 1.0}}, 
    6: {7: {'weight': 1.0}, 8: {'weight': 1.0}, 9: {'weight': 1.0}, 10: {'weight': 1.0}}, 
    7: {6: {'weight': 1.0}}, 8: {6: {'weight': 1.0}}, 9: {6: {'weight': 1.0}}, 
    10: {6: {'weight': 1.0}}, 11: {12: {'weight': 1.0}}, 12: {11: {'weight': 1.0}}}

__Return Value__  
It returns a networkx.Graph instance.  

## SEE ALSO  
pagerank, bfs, connected_componenets, single_source_shortest_path