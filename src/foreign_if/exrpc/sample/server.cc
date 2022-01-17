#include "frovedis.hpp"
#include "exrpc_expose.hpp"

#include "functions.hpp"

using namespace frovedis;
using namespace std;

int main(int argc, char* argv[]) {
  frovedis::use_frovedis use(argc, argv);
  expose(make_dvector_sample);
  expose(gather_sample);
  expose(delete_exptr<dvector<int>>);
  expose(get_each_pointer);
  expose(get_each_rawpointer);
  expose(add_each);
  expose(ex_sample);
  frovedis::init_frovedis_server(argc, argv);
}
