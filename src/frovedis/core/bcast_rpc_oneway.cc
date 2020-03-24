#include "mpi_rpc.hpp"
#include <sstream>

namespace frovedis {

void fwrapper_oneway0(intptr_t function_addr, const std::string& input);

void bcast_rpc_oneway(void(*f)()) {
  my_ostream outss;
  my_oarchive outar(outss);
  OSTREAM_TO_STRING(outss, str);
  send_bcast_rpcreq_oneway
    (rpc_type::bcast_rpc_oneway_type,
     reinterpret_cast<intptr_t>(f),
     reinterpret_cast<intptr_t>(fwrapper_oneway0),
     str);
}

}
