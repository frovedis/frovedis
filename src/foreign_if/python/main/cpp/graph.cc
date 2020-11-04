#include "python_client_headers.hpp"
#include "exrpc_graphx.hpp"
#include "short_hand_sparse_type.hpp"

extern "C" {
  long set_graph_data(const char* host, int port, long proxy){
    ASSERT_PTR(host);
    exrpc_node fm_node(host, port);
    auto f_dptr = static_cast<exrpc_ptr_t> (proxy);
    //int vb = 0; //no log (default=0)
    exrpc_ptr_t result = -1;
    try{
      result = exrpc_async(fm_node,(set_graph_data<S_MAT1,DT1>),f_dptr).get();
    }
    catch(std::exception& e) { set_status(true,e.what()); }
    return (long)result;
  }

  PyObject* get_graph_data(const char* host, int port, long proxy) {
    exrpc_node fm_node(host, port);
    auto f_dptr = static_cast<exrpc_ptr_t> (proxy);
    dummy_matrix adj_mat;
    try {
      adj_mat = exrpc_async(fm_node, get_graph_data_dummy_matrix<graph<DT1>>, f_dptr).get();
    }
    catch(std::exception& e) { set_status(true,e.what()); }
    return to_py_dummy_matrix(adj_mat);
  }

  PyObject* call_frovedis_pagerank(const char* host, int port, long proxy,
                                  double epsilon, double dfactor,
                                  int max_iter){
    ASSERT_PTR(host);
    exrpc_node fm_node(host, port);
    auto f_dptr = static_cast<exrpc_ptr_t> (proxy);
    int vb = 0; //no log (default=0)
    std::vector<double> result;
    try{
      result = exrpc_async(fm_node, frovedis_pagerank<graph<DT1>>, f_dptr,
                       epsilon, dfactor, max_iter, vb).get();
    }
    catch(std::exception& e) { set_status(true,e.what()); }
    return to_python_double_list(result);
  }

  void call_frovedis_sssp(const char* host, int port, long proxy,
                          double* dist, long* pred, 
                          long num_vertices, long source){
    ASSERT_PTR(host);
    exrpc_node fm_node(host, port);
    auto f_dptr = static_cast<exrpc_ptr_t> (proxy);
    int vb = 0; //no log (default=0)
    sssp_result<DT1,DT5> result;
    try{
      result = exrpc_async(fm_node, (frovedis_sssp<graph<DT1>,DT1,DT5>), f_dptr,
                       source, vb).get();
    }
    catch(std::exception& e) { set_status(true,e.what()); }
    checkAssumption(result.distances.size() == num_vertices);
    checkAssumption(result.predecessors.size() == num_vertices);
    auto resdistp = result.distances.data();
    auto respredp = result.predecessors.data();
    for(size_t i = 0; i < num_vertices; ++i) {
      dist[i] = resdistp[i];
      pred[i] = (long) respredp[i];
    }
  }

  void call_frovedis_bfs(const char* host, int port, long proxy,
                         long* dist, long* pred,
                         long num_vertices, long source,
                         int opt_level, double hyb_threshold) {
    ASSERT_PTR(host);
    exrpc_node fm_node(host, port);
    auto f_dptr = static_cast<exrpc_ptr_t> (proxy);
    int vb = 0; //no log (default=0)
    bfs_result<DT5> result;
    try{
      result = exrpc_async(fm_node, (frovedis_bfs<graph<DT1>,DT5>), f_dptr,
                       source, opt_level, hyb_threshold, vb).get();
    }
    catch(std::exception& e) { set_status(true,e.what()); }
    checkAssumption(result.distances.size() == num_vertices);
    checkAssumption(result.predecessors.size() == num_vertices);
    auto resdistp = result.distances.data();
    auto respredp = result.predecessors.data();
    auto lmax = std::numeric_limits<long>::max();
    auto uimax = std::numeric_limits<size_t>::max();
    for(size_t i = 0; i < num_vertices; ++i) {
      dist[i] = (resdistp[i] == uimax) ? lmax : (long) resdistp[i];
      pred[i] = (long) respredp[i];
    }
  }

  PyObject* call_frovedis_cc(const char* host, int port, long proxy,
                             long* nodes_in_which_cc, 
                             long* nodes_dist, long num_vertices,
                             int opt_level, double hyb_threshold) {
    ASSERT_PTR(host);
    exrpc_node fm_node(host, port);
    auto f_dptr = static_cast<exrpc_ptr_t> (proxy);
    int vb = 0; //no log (default=0)
    cc_result<DT5> result;
    try{
      result = exrpc_async(fm_node, (frovedis_cc<graph<DT1>,DT5>), 
                           f_dptr, opt_level, hyb_threshold, vb).get();
    }
    catch(std::exception& e) { set_status(true,e.what());}
    checkAssumption(result.distances.size() == num_vertices);
    checkAssumption(result.num_nodes_in_which_cc.size() == num_vertices);
    auto resdistp = result.distances.data();
    auto reswhichp = result.num_nodes_in_which_cc.data();
    auto lmax = std::numeric_limits<long>::max();
    auto uimax = std::numeric_limits<size_t>::max();
    for(size_t i = 0; i < num_vertices; ++i){
      nodes_dist[i] = (resdistp[i] == uimax) ? lmax : (long) resdistp[i];
      nodes_in_which_cc[i] = (reswhichp[i] == uimax) ? lmax : (long) reswhichp[i];
    }
    std::vector<long> ret(2 * result.num_cc);
    for(size_t i = 0; i < result.num_cc; ++i) {
      ret[2 * i] = result.root_in_each_cc[i];
      ret[2 * i + 1] = result.num_nodes_in_each_cc[i];
    }
    return to_python_long_list(ret);
  }

  long copy_graph_py(const char* host, int port, long proxy) {
    ASSERT_PTR(host);
    exrpc_node fm_node(host, port);
    auto f_dptr = static_cast<exrpc_ptr_t> (proxy);
    exrpc_ptr_t ret = -1;
    try {
      ret = exrpc_async(fm_node, copy_graph<graph<DT1>>, f_dptr).get();
    }
    catch(std::exception& e) { set_status(true,e.what()); }
    return (long)ret;
  }

  void show_graph_py(const char* host, int port, long proxy){
    ASSERT_PTR(host);
    exrpc_node fm_node(host, port);
    auto f_dptr = static_cast<exrpc_ptr_t> (proxy);
    try{
      exrpc_oneway(fm_node, show_graph<graph<DT1>>, f_dptr);
    }
    catch(std::exception& e) {set_status(true, e.what());}
  }

  void release_graph_py(const char* host, int port, long proxy){
    ASSERT_PTR(host);
    exrpc_node fm_node(host, port);
    auto f_dptr = static_cast<exrpc_ptr_t> (proxy);
    try{
      exrpc_oneway(fm_node, release_graph<graph<DT1>>, f_dptr);
    }
    catch(std::exception& e) {set_status(true, e.what());}
  }

  void save_graph_py(const char* host, int port, long proxy,
                     const char* fname){
    ASSERT_PTR(host);
    exrpc_node fm_node(host, port);
    std::string f_name(fname);
    auto f_dptr = static_cast<exrpc_ptr_t> (proxy);
    try{
      exrpc_oneway(fm_node, save_graph<graph<DT1>>, f_dptr, f_name);
    }
    catch(std::exception& e) {set_status(true, e.what());}
  }

  PyObject* load_graph_from_text_file(const char* host, int port,
            const char* fname){
    ASSERT_PTR(host);
    exrpc_node fm_node(host, port);
    std::string f_name(fname);
    dummy_graph ret;
    try{
      ret = exrpc_async(fm_node, (load_graph<graph<DT1>,DT1>), f_name).get();
    }
    catch(std::exception& e) {set_status(true, e.what());}
    return to_py_dummy_graph(ret);
  }
}
