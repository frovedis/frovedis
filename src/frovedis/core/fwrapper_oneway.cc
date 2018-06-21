#include <stdint.h>
#include <string>

namespace frovedis {

void fwrapper_oneway0(intptr_t function_addr, const std::string& input)
{
  typedef void(*Ftype)();
  Ftype f = reinterpret_cast<Ftype>(function_addr);
  f();
  return;
}

}
