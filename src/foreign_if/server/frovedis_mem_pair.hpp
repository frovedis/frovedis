#ifndef _FROVEDIS_MEM_PAIR_HPP_
#define _FROVEDIS_MEM_PAIR_HPP_

namespace frovedis {
struct frovedis_mem_pair {
  frovedis_mem_pair() {}
  frovedis_mem_pair(exrpc_ptr_t x, exrpc_ptr_t y) : mx(x), my(y) {}
  exrpc_ptr_t first() { return mx; } 
  exrpc_ptr_t second() { return my; }
  void display() { std::cout << "[" << mx << "," << my << "]\n"; }

  exrpc_ptr_t mx, my;
  SERIALIZE(mx, my)
};
}
#endif
