#include "frovedis.hpp"
#include "exrpc.hpp"
#include "exrpc_async.hpp"
#include "exrpc_oneway.hpp"

#include "functions.hpp"

using namespace frovedis;
using namespace std;

int main(int argc, char* argv[]) {
  auto n = invoke_frovedis_server("mpirun -np 2 ./server");

  // demo of creating dvector at server side
  vector<int> v = {1,2,3,4};
  auto r = exrpc_async(n, make_dvector_sample, v).get();
  auto ep = exrpc_async(n, get_each_pointer, r).get();

  // demo of connecting to each workers
  auto info = prepare_parallel_exrpc(n);
  auto nodes = get_parallel_exrpc_nodes(n, info);
  wait_parallel_exrpc(n, info);
  for(size_t i = 0; i < nodes.size(); i++) {
    vector<int> to_add = {5,6};
    exrpc_oneway(nodes[i], add_each, ep[i], to_add);
  }
  
  auto vv = exrpc_async(n, gather_sample, r).get();
  for(auto i: vv) cout << i << endl;

  // demo of exception handling
  try {
    auto ep = exrpc_async(n, ex_sample, r).get();
    for(auto i: ep) cout << i << endl;
  } catch (exception& e) {
    cout << e.what() << endl;
  }

  // delete_exptr is in exrpc.hpp
  exrpc_oneway(n, delete_exptr<dvector<int>>, r); 

  finalize_frovedis_server(n);
}
