#include <frovedis.hpp>
#include <frovedis/matrix/rowmajor_matrix.hpp>

std::vector<double>
matvec(frovedis::rowmajor_matrix_local<double>& m, std::vector<double>& v) {
  std::vector<double> ret(m.local_num_row);
  for(size_t r = 0; r < m.local_num_row; r++) {
    for(size_t c = 0; c < m.local_num_col; c++) {
      ret[r] += m.val[m.local_num_col * r + c] * v[c];
    }
  }
  return ret;
}

int main(int argc, char* argv[]){
  frovedis::use_frovedis use(argc, argv);

  auto m = frovedis::make_rowmajor_matrix_load<double>("./data");
  std::vector<double> v(m.num_col);
  for(size_t i = 0; i < v.size(); i++) v[i] = 1;
  auto lv = frovedis::make_node_local_broadcast(v);
  auto mv = m.data.map(matvec, lv);
  auto g = mv.moveto_dvector<double>().gather();
  for(auto i: g) std::cout << i << std::endl;
}
