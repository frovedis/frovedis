#include "python_client_headers.hpp"

using namespace frovedis;

extern "C" {
  // --- Frovedis server Initialization, Finalization, Query ---
  PyObject* initialize_server(const char *cmd) {
    if(!cmd) REPORT_ERROR(USER_ERROR,"Invalid server command!!");
    exrpc_node n;
    try {
      n = invoke_frovedis_server(cmd);
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
    auto host = n.hostname.c_str();
    auto port = n.rpcport; 
    return Py_BuildValue("{s:s, s:i}", "hostname", host, "rpcport", port); 
  }
  
  int get_worker_size(const char* host, int port) {
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host,port);
    int size = 0;
    try {
      size = exrpc_async0(fm_node, get_nodesize).get();
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
    return size;
  }

  std::vector<exrpc_node> get_worker_nodes(exrpc_node& fm_node) {  
    auto info = prepare_parallel_exrpc(fm_node);
    auto nodes = get_parallel_exrpc_nodes(fm_node, info);
    wait_parallel_exrpc(fm_node, info);
    return nodes;
  }

  std::vector<exrpc_node> 
  get_worker_nodes_for_multi_exrpc(exrpc_node& fm_node, 
                                   std::vector<size_t> blocksz) {
    auto info = prepare_parallel_exrpc(fm_node);
    auto nodes = get_parallel_exrpc_nodes(fm_node, info);
    wait_parallel_exrpc_multi(fm_node, info, blocksz);
    return nodes;
  }

  std::vector<exrpc_node> get_worker_nodes_for_vector_rawsend(exrpc_node& fm_node) {
    auto info = prepare_parallel_exrpc(fm_node);
    auto nodes = get_parallel_exrpc_nodes(fm_node, info);
    // for rawsend: 2 exrpc operations will be performed on each node
    //   - 1st exrpc: for memory allocation
    //   - 2nd exrpc: for raw-data transfer
    auto blocksz = vector_full<size_t>(nodes.size(), 2); 
    wait_parallel_exrpc_multi(fm_node, info, blocksz);
    return nodes;
  }

  void clean_server(const char* host, int port) {
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host,port);
    try {
      exrpc_oneway0(fm_node, cleanup_frovedis_server);
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
  }

  void finalize_server(const char* host, int port) {
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host,port);
    try {
      finalize_frovedis_server(fm_node);
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
  }
}
