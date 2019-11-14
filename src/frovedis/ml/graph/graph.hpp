
#ifndef GRAPH_HPP
#define GRAPH_HPP

#include "global.hpp"
#include "frovedis/ml/graph/set_union_multivec.hpp"
#include "frovedis/ml/graph/connected_components.hpp"
#include "frovedis/ml/graph/sssp.hpp"
#include "frovedis/ml/graph/pagerank.hpp"

namespace frovedis {
    

class graph {
public:
   
    void read_graph(std::string gp, bool if_binary);  //read graph in CRS matrix
    void read_graph_pagerank(std::string gp, bool if_binary);  //read graph in CRS matrix

    //Connected Components
#if 0  // NOT SUPPORTED DUE TO INTERNAL BUG. 
    void cc_bfs_spmv(std::string matformat); //Breath-first search for connected components.
#endif
    void cc_bfs_scatter();
    void cc_bfs_idxsort(size_t threshold, bool if_hyb);
    void cc_BFS_HybridMerge(size_t merge_th);
#if 0  // NOT SUPPORTED DUE TO INTERNAL BUG. 
    void cc_bfs_2dmerge_allgather(size_t merge_th); //update and check ifVisited locally, exchange idx_next among workers using allgather
#endif
    void cc_bfs_2dmerge_gather(size_t merge_th, bool if_scale_free, bool if_hyb); 
    
    
    //SSSP
    void sssp_bf_spmv(bool if_sssp_enq, size_t source_vertex);
    void sssp_bf_spmspv_2dmerge(bool if_sssp_enq, size_t source_vertex, size_t upperbound);
    void sssp_bf_spmspv_sort(bool if_sssp_enq, size_t source_vertex);   
    void sssp_query();
    
    //PageRank
    void pagerank_v1(std::string matformat, double dfactor, double epsilon, size_t iter_max);
    void pagerank_v1_shrink(std::string matformat, double dfactor, double epsilon, size_t iter_max);
    void pagerank_v2(std::string matformat, double dfactor, double epsilon, size_t iter_max);
    void pagerank_v2_shrink(std::string matformat, double dfactor, double epsilon, size_t iter_max);
        
    
    //generate matrix graph file from list file
    void prep_graph_crs(const std::string &filename_edge, bool if_direct); //Prepare CRS graph from original edge files. direct or undirect  
  void prep_graph_crs_pagerank(const std::string &filename_edge, const std::string &output_file);

    size_t insert_mapping(const std::string &key);
    bool add_arc(size_t from, size_t to);
    template <class Vector, class T> bool insert_into_vector(Vector& v, const T& t); 
    
    
    time_spent_graph t_all; //Exclude t_load
    time_spent_graph t_load;
    time_spent_graph t_compute_spmv;
    time_spent_graph t_compute_other;
    time_spent_graph t_comm;
    
    graph();
    graph(const graph& orig);
    void set_if_cout(bool if_cout){
        this->if_cout = if_cout;
    }
    void set_if_fout(bool if_fout){
        this->if_fout = if_fout;
    }    
    void reset();    

    virtual ~graph();
    
    void print_exectime();
    void print_summary_cc();
    void print_trace();  

    // load,save
    void loadbinary_pagerank(const std::string& path) { _load_pagerank(path, true); }
    void loadbinary_cc(const std::string& path) { _load_cc(path, true); }
    void loadbinary_sssp(const std::string& path) { _load_sssp(path, true); }
    void load_pagerank(const std::string& path) { _load_pagerank(path, false); }
    void load_cc(const std::string& path) { _load_cc(path, false); }
    void load_sssp(const std::string& path) { _load_sssp(path, false); }
  
    void savebinary_pagerank(const std::string& path) { _save_pagerank(path, true); }
    void savebinary_cc(const std::string& path) { _save_cc(path, true); }
    void savebinary_sssp(const std::string& path) { _save_sssp(path, true); }
    void save_pagerank(const std::string& path) { _save_pagerank(path, false); }
    void save_cc(const std::string& path) { _save_cc(path, false); }
    void save_sssp(const std::string& path) { _save_sssp(path, false); }
  
    size_t get_num_nodes() { return num_nodes; }
    size_t get_num_cc() { return num_cc; }
    size_t get_source_nodeID() { return source_nodeID; }
    std::vector<TYPE_MATRIX_PAGERANK> get_prank() { return prank; }
    std::vector<size_t>& get_num_nodes_in_each_cc() { return num_nodes_in_each_cc; }
    std::vector<size_t>& get_nodes_in_which_cc() { return nodes_in_which_cc; }
    std::vector<TYPE_MATRIX>& get_nodes_dist() { return nodes_dist; }
    std::vector<size_t>& get_nodes_pred() { return nodes_pred; }

    frovedis::crs_matrix<TYPE_MATRIX> A;             // generic matrix 
    frovedis::crs_matrix<TYPE_MATRIX_PAGERANK> A_pg; // pagerank matrix
    std::vector<size_t> num_outgoing; // number of outgoing links per column
    size_t num_nodes;    

private:

    std::string graph_path;    
    //frovedis::crs_matrix<TYPE_MATRIX> A;
    std::vector<TYPE_BITMAP> nodes_ifVisited; //bitmap for each node to accelerate searching
    std::vector<TYPE_BITMAP> nodes_ifNext;
    std::vector<TYPE_BITMAP> nodes_ifCurrent;
    std::vector<size_t> v_local_num_row;
    std::vector<size_t> mat_offset;
    
    sparse_vector<TYPE_BITMAP> nodes_Current_sp;
    sparse_vector<TYPE_BITMAP> nodes_Next_sp;
    
    std::vector<TYPE_IDXV> nodes_Next_idx;
    std::vector<TYPE_IDXV> nodes_Current_idx;
    //sparse_vector<type_BitMap> nodes_ifVisited_sp;
    
    size_t source_nodeID; //Note that the graph file we adopt is from 1. Node 0 is dummy and isolated
    std::vector<size_t> nodes_in_which_cc; //For each node, stores the lowest nodeID of the cc containg such node 
    //std::vector<long long> nodes_dist; //For each node, stores the distance from the source node (shortest path)
    
    size_t current_level;
    //size_t num_nodes;
    size_t num_cc;  //number of connected components
    std::vector<size_t> num_nodes_in_each_cc; //num of nodes in each cc;
    size_t hyb_threshold;
    size_t num_nodes_in_cc; //number of nodes in each cc;
    
    bool if_fout;
    bool if_cout;
    
    std::ofstream fout;
    std::ofstream fgraph_out; // generated graph file output path; 
    std::ofstream fout_exectime;
    
    //std::vector<size_t> num_outgoing; // number of outgoing links per column
    std::vector< std::vector<size_t> > rows; // the rows of the hyperlink matrix
    std::map<std::string, size_t> nodes_to_idx; // mapping from string node IDs to numeric
    std::map<size_t, std::string> idx_to_nodes; // mapping from numeric node IDs to string

    //sssp 
    std::vector<TYPE_MATRIX> nodes_dist;
    std::vector<size_t> nodes_pred;
    sparse_vector_tri<TYPE_MATRIX> nodes_dist_current;
    
    //pagerank
    std::vector<TYPE_MATRIX_PAGERANK> rel_prank; //relative rank vector
    std::vector<TYPE_MATRIX_PAGERANK> prank;  //rank vector
    //frovedis::crs_matrix<TYPE_MATRIX_PAGERANK> A_pg;
    TYPE_MATRIX_PAGERANK* prankp;    

    void _load_pagerank(const std::string&, bool binary);
    void _load_cc(const std::string&, bool binary);
    void _load_sssp(const std::string&, bool binary);
  
    void _save_pagerank(const std::string&, bool binary);
    void _save_cc(const std::string&, bool binary);
    void _save_sssp(const std::string&, bool binary);

};



}
#endif	/* GRAPH_HPP */

