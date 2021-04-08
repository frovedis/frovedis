"""
Demo for graph algorithms
"""
import os
import sys
import time
import numpy as np
import pandas as pd
import networkx as nx
import frovedis.graph as fnx
from collections import OrderedDict

# Load dataset
input_graph_files = {
    'preferentialAttachment' : './datasets/preferentialAttachment.mtx',
    'caidaRouterLevel'       : './datasets/caidaRouterLevel.mtx',
    'coAuthorsDBLP'          : './datasets/coAuthorsDBLP.mtx',
    'dblp'                   : './datasets/dblp-2010.mtx',
    #'citationCiteseer'       : './datasets/citationCiteseer.mtx',
    #'coPapersDBLP'           : './datasets/coPapersDBLP.mtx',
    #'coPapersCiteseer'       : './datasets/coPapersCiteseer.mtx',
    #'as-Skitter'             : './datasets/as-Skitter.mtx',
    #'kk_u.txt'               : './datasets/kk_u.txt'
}

from frovedis.exrpc.server import FrovedisServer
FrovedisServer.initialize("mpirun -ve 1 -np 8 " + os.environ["FROVEDIS_SERVER"])

# -------- Graph loading demo --------
frov_data_loading_time = []
nx_data_loading_time = []
frov_graph = []
frov_graph_dir = []
nx_graph = []
nx_graph_dir = []
nnodes = []
nedges = []

for dataset, path in input_graph_files.items():
    dl_start_time = time.time()
    fgraph = fnx.read_edgelist(path, nodetype=np.int32, delimiter=' ')
    # for pagerank etc. we will use directed graph for better ranking
    fgraph_dir = fnx.read_edgelist(path, nodetype=np.int32, delimiter=' ', \
                                   create_using=nx.DiGraph())
    frov_data_loading_time.append(round(time.time() - dl_start_time, 4))
    frov_graph.append(fgraph)
    frov_graph_dir.append(fgraph_dir)
    
    dl_start_time = time.time()
    ngraph = nx.read_edgelist(path, nodetype=np.int32, delimiter=' ')
    # for pagerank etc. we will use directed graph for better ranking
    ngraph_dir = nx.read_edgelist(path, nodetype=np.int32, delimiter=' ', \
                                  create_using=nx.DiGraph())
    nx_data_loading_time.append(round(time.time() - dl_start_time, 4))
    nx_graph.append(ngraph)
    nx_graph_dir.append(ngraph_dir)

    nnodes.append(ngraph.number_of_nodes())
    nedges.append(ngraph.number_of_edges())

data_details = pd.DataFrame(OrderedDict({ "dataset": list(input_graph_files.keys()),
                                "num_nodes": nnodes,
                                "num_edges": nedges,
                                "frov_loading_time": frov_data_loading_time,
                                "nx_loading_time": nx_data_loading_time
                             }))
data_details["frov_speed_up"] = data_details["nx_loading_time"] / data_details["frov_loading_time"]
print(data_details)


# -------- SSSP demo --------
src = 5
frov_sssp_time = []
nx_sssp_time = []
dist_matched = []

for i in range(len(frov_graph)):
    start_time = time.time()
    fpath, fdist = fnx.single_source_shortest_path(frov_graph[i], \
                   src, return_distance=True)
    frov_sssp_time.append(round(time.time() - start_time, 4))

    #print("frovedis: distance for {} dataset".format(dataset))
    #print(fdist)

    start_time = time.time()
    npath = nx.single_source_shortest_path(nx_graph[i], src)
    nx_sssp_time.append(round(time.time() - start_time, 4))

    ndist = {k: float(len(v)-1) for k, v in npath.items()}
    #print("networkx: distance for {} dataset".format(dataset))
    #print(ndist)

    dist_matched.append("Yes" if fdist == ndist else "No")

sssp_df = pd.DataFrame(OrderedDict ({ "dataset": list(input_graph_files.keys()),
                                      "frov_sssp_time": frov_sssp_time,
                                      "nx_sssp_time": nx_sssp_time,
                                      "result_matched": dist_matched
                                   }))
sssp_df["frov_speed_up"] = sssp_df["nx_sssp_time"] / sssp_df["frov_sssp_time"]
print(sssp_df)

# -------- pagerank demo --------
frov_pr_time = []
nx_pr_time = []
order_matched = []

#pagerank uses directed graph for generating better numerical ranks
for i in range(len(frov_graph_dir)):
    start_time = time.time()
    frov_ranks = fnx.pagerank(frov_graph_dir[i])
    frov_pr_time.append(round(time.time() - start_time, 4))
    df1 = pd.DataFrame({ "node": list(frov_ranks.keys()),
                         "rank": list(frov_ranks.values())
                       }) \
            .sort_values(by = ["rank", "node"]) \
            .head(10)
    #print(df1)

    start_time = time.time()
    nx_ranks = nx.pagerank(nx_graph_dir[i])
    nx_pr_time.append(round(time.time() - start_time, 4))
    df2 = pd.DataFrame({ "node": list(nx_ranks.keys()),
                         "rank": list(nx_ranks.values())
                       }) \
            .sort_values(by = ["rank", "node"]) \
            .head(10)
    #print(df2)

    order_matched.append("Yes" if list(df1.node.values) == list(df2.node.values) else "No")

pagerank_df = pd.DataFrame(OrderedDict ({ "dataset": list(input_graph_files.keys()),
                             "frov_pr_time": frov_pr_time,
                             "nx_pr_time": nx_pr_time,
                             "ranking_order_matched": order_matched
                          }))
pagerank_df["frov_speed_up"] = pagerank_df["nx_pr_time"] / pagerank_df["frov_pr_time"]
print(pagerank_df)


# -------- bfs demo --------
source = 1
distance = 4 
frov_bfs_time = []
nx_bfs_time = []
result_matched = []

for i in range(len(frov_graph)):
    start_time = time.time()
    frov_res = fnx.descendants_at_distance(frov_graph[i], source, distance)
    frov_bfs_time.append(round(time.time() - start_time, 4))
    #print(list(frov_res)[:5])

    start_time = time.time()
    nx_res = nx.descendants_at_distance(nx_graph[i], source, distance)
    nx_bfs_time.append(round(time.time() - start_time, 4))
    #print(list(nx_res)[:5])

    result_matched.append("Yes" if len(frov_res - nx_res) == 0 else "No")

bfs_df = pd.DataFrame(OrderedDict ({ "dataset": list(input_graph_files.keys()),
                        "frov_bfs_time": frov_bfs_time,
                        "nx_bfs_time": nx_bfs_time,
                        "result_matched": result_matched
                     }))
bfs_df["frov_speed_up"] = bfs_df["nx_bfs_time"] / bfs_df["frov_bfs_time"]
print(bfs_df)

# -------- connected components demo --------
frov_cc_time = []
nx_cc_time = []
frov_ncomponents = []
nx_ncomponents = []
result_matched = []

for i in range(len(frov_graph)):
    start_time = time.time()
    tmp = fnx.connected_components(frov_graph[i])
    set_cc_frov = set()
    for elem in tmp:
        set_cc_frov.add(frozenset(elem))
    frov_cc_time.append(round(time.time() - start_time, 4))
    frov_ncomponents.append(len(set_cc_frov))

    start_time = time.time()
    tmp = nx.connected_components(nx_graph[i])
    set_cc_nx = set()
    for elem in tmp:
        set_cc_nx.add(frozenset(elem))
    nx_cc_time.append(round(time.time() - start_time, 4))
    nx_ncomponents.append(len(set_cc_nx))

    result_matched.append("Yes" if set_cc_frov == set_cc_nx else "No")

cc_df = pd.DataFrame(OrderedDict ({ "dataset": list(input_graph_files.keys()),
                        "frov_cc_time": frov_cc_time,
                        "nx_cc_time": nx_cc_time,
                        "frov_ncomponents": frov_ncomponents,
                        "nx_ncomponents": nx_ncomponents,
                        "result_matched": result_matched
                     }))
cc_df["frov_speed_up"] = cc_df["nx_cc_time"] / cc_df["frov_cc_time"]
print(cc_df)

FrovedisServer.shut_down()
