#include "exrpc_ml.hpp"

using namespace frovedis;

void expose_frovedis_pattern_mining_functions() {
  // (1) frequent pattern mining using fp-growth
  expose(frovedis_fp_growth<dftable>);
  expose(frovedis_fpr<fp_growth_model>);
  //expose(get_frovedis_fpm<std::string>);
  expose(get_frovedis_fpm<int>);
}
