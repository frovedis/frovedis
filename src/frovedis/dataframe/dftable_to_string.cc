#include "dftable_to_string.hpp"

namespace frovedis {

using namespace std;

struct put_ith {
  put_ith(){}
  put_ith(size_t i, size_t num_col) : i(i), num_col(num_col) {}
  void operator()(vector<vector<string>>& vs, vector<string>& c) {
    if(i == 0) {
      vs.resize(c.size());
      for(size_t j = 0; j < vs.size(); j++) {
        vs[j].resize(num_col);
      }
    }
    for(size_t j = 0; j < vs.size(); j++) {
      swap(vs[j][i], c[j]);
    }
  }
  size_t i, num_col;
  SERIALIZE(i, num_col)
};

dvector<vector<string>> dftable_to_string(dftable_base& table) {
  auto ret = make_node_local_allocate<vector<vector<string>>>();
  auto cols = table.columns();
  auto num_col = table.num_col();
  for(size_t i = 0; i < cols.size(); i++) {
    use_dfcolumn use(table.raw_column(cols[i]));
    auto nl = table.column(cols[i])->as_string().moveto_node_local();
    ret.mapv(put_ith(i,num_col), nl);
  }
  return ret.moveto_dvector<vector<string>>();
}

}
