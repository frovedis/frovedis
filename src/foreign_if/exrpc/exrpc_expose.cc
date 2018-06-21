#include "exrpc_expose.hpp"

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <strings.h>

namespace frovedis {

std::map<std::string, std::pair<intptr_t, intptr_t>> expose_table;

void flush_exposed_helper(std::map<std::string, std::pair<intptr_t, intptr_t>>& m) {
  expose_table = m;
}

void flush_exposed() {
  auto btable = make_node_local_broadcast(expose_table);
  btable.mapv(flush_exposed_helper);
}

void pfwrapper_oneway0(intptr_t function_addr, const std::string& input) {
  typedef void(*Ftype)();
  Ftype f = reinterpret_cast<Ftype>(function_addr);
  f();
  return;
}

void expose_function(const std::string& func_name, void(*f)()) {
  expose_table[func_name] =
    std::make_pair(reinterpret_cast<intptr_t>(pfwrapper_oneway0),
                   reinterpret_cast<intptr_t>(f));
}

}
