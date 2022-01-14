% pagerank()

# NAME  

pagerank() - It computes a ranking of the nodes in the graph 'G' based on the structure 
of the incoming links. It was originally designed as an algorithm to rank web pages.  

# SYNOPSIS  

    frovedis.graph.Pagerank.pagerank(G, alpha=0.85, personalization=None, max_iter=100, tol=1.0e-6, 
                                     nstart=None, weight='weight', dangling=None, verbose=0)  

# DESCRIPTION  
The PageRank algorithm ranks the nodes in a graph by their relative importance or 
influence. PageRank determines each node's ranking by identifying the number of links to 
the node and the quality of the links. The quality of a link is determined by the 
importance (PageRank) of the node that presents the outbound link.  

The PageRank algorithm was designed at first to measure the importance of a webpage by 
analyzing the quantity and quality of the links that point to it.  

Mathematically, PageRank (PR) is defined as:  

    PR(A) = (1 - d) + d(PR(Ti)/C(Ti)+ ... + PR(Tn)/C(Tn))

where **Page A** has pages **T1 to Tn** which point to it.  
**d** is a damping factor. It corresponds to the probability that an imaginary web surfer 
will suddenly visit a random page on the web instead of following one of the outbound links 
prescribed by the web page that the surfer is currently visiting. This is useful for 
accounting for sinks (web pages that have inbound links but no outbound links).  
Also, the **(1 - d)** in the preceding formula dampens the contribution of the incoming PageRanks 
from the adjacent vertices. It is as if imaginary outbound edges are added from all sink 
vertices to every other vertex in the graph, and to keep things fair, this same thing is 
done to the non-sink vertices as well.  
**C(A)** is defined as the number of links going out of page A.  

This module provides a client-server implementation, where the client application 
is a normal python program. The frovedis public method interface is almost same as 
NetworkX pagerank public method interface, but it doesn’t have any dependency with 
NetworkX. It can be used simply even if the system doesn’t have NetworkX installed. Thus, 
in this implementation, a python client can interact with a frovedis server sending the 
required python data for training at frovedis side. Python data is converted into frovedis 
compatible data internally and the python ML call is linked with the respective frovedis 
ML call to get the job done at frovedis server.  

Python side calls for pagerank() on the frovedis server. Once the PageRanks are computed for 
the given input graph data at the frovedis server, it returns a dictionary of nodes with 
PageRank as value to the client python program.  

## Detailed Description  

### 1. pagerank()  

__Parameters__  
_**G**_: An instance of networkx graph or frovedis graph on which pagerank is to be computed. The 
graph can be directed or undirected.  
_**alpha**_: A positive integer parameter that is referred as **damping factor**. It must 
be in range 0 and 1. It represents a sort of minimum PageRank value. (Default: 0.85)  
_**personalization**_: An unused parameter. (Default: None)  
_**max\_iter**_: A positive integer parameter that specifies the maximum number of iterations 
to run this method. Convergence may occur before max_iter value, in that case the iterations 
will stop. (Default: 100)  
_**tol**_: A positive double (float64) parameter specifying the convergence 
tolerance. The PageRank iterations stop if the sum of the error values for all nodes is 
below this value. (Default: 1.0e-6)  
_**nstart**_: An unused parameter. (Default: None)  
_**weight**_: An unused parameter. (Default: 'weight')  
_**dangling**_: An unused parameter. (Default: None)  
_**verbose**_: An integer parameter specifying the log level to use. Its value is 0 by 
default (INFO level). But it can be set to 1 (DEBUG level) or 2 (TRACE level) for getting 
training time logs from frovedis server.   

__Purpose__  
This method ranks the nodes present in the graph with the given parameters.  

The parameters: "personalization", "nstart", "weight" and "dangling" are simply kept to 
make the method uniform to NetworkX pagerank method. They are not used anywhere 
within the frovedis implementation.  

For example,  

**When loading an undirected networkx.graph as input from an edgelist file,**

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

    # An undirected graph loaded from edgelist file
    import networkx as gnx
    networkx_graph = gnx.read_edgelist('input/cit-Patents.txt')
    ranks = gnx.pagerank(networkx_graph)
    print ("NX PR: ", ranks)

Output

    [rank 0] pagerank: converged in 8 iterations!
    NX PR:  {1: 0.11705684590144233, 2: 0.06889632312740385, 3: 0.06889632312740385, 
    4: 0.06889632312740385, 5: 0.06889632312740385, 6: 0.06889632312740385, 7: 0.10702340365685098, 
    8: 0.06939799523963341, 9: 0.06939799523963341, 10: 0.06939799523963341, 
    11: 0.06939799523963341, 12: 0.07692307692307693, 13: 0.07692307692307693}

**When loading an undirected frovedis graph as input from an edgelist file,**

FILE: cit-Patents.txt (same file)  

    # An undirected graph loaded from edgelist file
    import frovedis.graph as fnx
    frov_graph = fnx.read_edgelist('input/cit-Patents.txt')
    ranks = fnx.pagerank(frov_graph)
    print ("FROV PR: ", ranks)

Output

    [rank 0] pagerank: converged in 8 iterations!
    FROV PR:  {1: 0.11705684590144233, 2: 0.06889632312740385, 3: 0.06889632312740385, 
    4: 0.06889632312740385, 5: 0.06889632312740385, 6: 0.06889632312740385, 7: 0.10702340365685098, 
    8: 0.06939799523963341, 9: 0.06939799523963341, 10: 0.06939799523963341, 
    11: 0.06939799523963341, 12: 0.07692307692307693, 13: 0.07692307692307693}

**When loading an directed networkx.graph as input from an edgelist file,**

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
    import networkx as gnx
    networkx_graph = gnx.read_edgelist('input/cit-Patents.txt', create_using = gnx.DiGraph())
    ranks = gnx.pagerank(networkx_graph)
    print ("NX PR: ", ranks)

Output

    [rank 0] pagerank: converged in 3 iterations!
    NX PR:  {1: 0.06538461538461539, 2: 0.06734615384615385, 3: 0.06734615384615385, 
    4: 0.06734615384615385, 5: 0.06734615384615385, 6: 0.06734615384615385, 7: 0.06538461538461539, 
    8: 0.06783653846153846, 9: 0.06783653846153846, 10: 0.06783653846153846, 
    11: 0.06783653846153846, 12: 0.06538461538461539, 13: 0.0751923076923077}

**When loading an directed frovedis graph as input from an edgelist file,**

FILE: cit-Patents.txt (same file)  

    # A directed graph loaded from edgelist file
    import frovedis.graph as fnx
    import networkx
    frov_graph = fnx.read_edgelist('input/cit-Patents.txt', create_using = networkx.DiGraph())
    ranks = fnx.pagerank(frov_graph)
    print ("FROV PR: ", ranks)

Output

    [rank 0] pagerank: converged in 3 iterations!
    FROV PR:  {1: 0.06538461538461539, 2: 0.06734615384615385, 3: 0.06734615384615385, 
    4: 0.06734615384615385, 5: 0.06734615384615385, 6: 0.06734615384615385, 7: 0.06538461538461539, 
    8: 0.06783653846153846, 9: 0.06783653846153846, 10: 0.06783653846153846, 
    11: 0.06783653846153846, 12: 0.06538461538461539, 13: 0.0751923076923077}

__Return Value__    
It returns a dictionary of nodes with PageRank as value.  

## SEE ALSO  
graph, bfs, connected_components, single_source_shortest_path  