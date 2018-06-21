#include "mpi_rpc.hpp"
#include <sstream>

namespace frovedis {

void fwrapper_oneway0(intptr_t function_addr, const std::string& input);

void rpc_oneway(NID n, void(*f)()) {
  std::ostringstream outss;
  my_oarchive outar(outss);
  send_rpcreq(rpc_type::rpc_oneway_type, n,
              reinterpret_cast<intptr_t>(f),
              reinterpret_cast<intptr_t>(fwrapper_oneway0),
              outss.str());
}

}
