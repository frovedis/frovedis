#include <frovedis.hpp>
#include <frovedis/matrix/colmajor_matrix.hpp>
#include <frovedis/matrix/blas_wrapper.hpp>

std::vector<double>
matvec(frovedis::colmajor_matrix_local<double>& m, std::vector<double>& v) {
  std::vector<double> ret(m.local_num_row);
  frovedis::gemv<double>(m, v, ret);
  return ret;
}

int main(int argc, char* argv[]){
  frovedis::use_frovedis use(argc, argv);

  auto rm = frovedis::make_rowmajor_matrix_load<double>("./data");
  frovedis::colmajor_matrix<double> m(rm);
  std::vector<double> v(m.num_col);
  for(size_t i = 0; i < v.size(); i++) v[i] = 1;
  auto lv = frovedis::make_node_local_broadcast(v);
  auto mv = m.data.map(matvec, lv);
  auto g = mv.moveto_dvector<double>().gather();
  for(auto i: g) std::cout << i << std::endl;
}
