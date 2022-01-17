#include "functions.hpp"

using namespace frovedis;
using namespace std;

exptr<dvector<int>> make_dvector_sample(vector<int>& v) {
  return exptr<dvector<int>>(new dvector<int>(make_dvector_scatter(v)));
}

vector<int> gather_sample(exptr<dvector<int>>& dv) {
  return dv.to_ptr()->gather();
}

vector<exptr<vector<int>>> get_each_pointer(exptr<dvector<int>>& dv) {
  // [](...){...} is lambda expression; "+" is used to make it function ptr
  return dv.to_ptr()->map_partitions(+[](vector<int>& d) {
      auto dp = make_exptr(&d);
      // mapPartitions need to return vector
      vector<exptr<vector<int>>> r(1);
      r[0] = dp;
      return r;
    }
  ).gather();
}

vector<exrpc_ptr_t> get_each_rawpointer(exptr<dvector<int>>& dv) {
  // [](...){...} is lambda expression; "+" is used to make it function ptr
  return dv.to_ptr()->map_partitions(+[](vector<int>& d) {
      auto dp = reinterpret_cast<exrpc_ptr_t>(d.data());
      // mapPartitions need to return vector
      vector<exrpc_ptr_t> r(1);
      r[0] = dp;
      return r;
    }
  ).gather();
}

void add_each(exptr<vector<int>>& p, vector<int>& v) {
  vector<int>& pv = *p.to_ptr();
  for(size_t i = 0; i < pv.size(); i++) {
    pv[i] += v[i];
  }
}

int ex(int& a) {
  if(get_selfid() == 1)
    throw std::runtime_error("exception test");
  else
    return a * 2;
}

vector<int> ex_sample(exptr<dvector<int>>& dv) {
  return dv.to_ptr()->map(ex).gather();
}
