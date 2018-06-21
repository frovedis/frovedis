#include <frovedis.hpp>
#include <frovedis/matrix/crs_matrix.hpp>

std::vector<double>
matvec(frovedis::crs_matrix_local<double>& m, std::vector<double>& v) {
  return m * v;
}

int main(int argc, char* argv[]){
  frovedis::use_frovedis use(argc, argv);

  auto m = frovedis::make_crs_matrix_load<double>("./data");
  std::vector<double> v(m.num_col);
  for(size_t i = 0; i < v.size(); i++) v[i] = 1;
  auto lv = frovedis::make_node_local_broadcast(v);
  auto mv = m.data.map(matvec, lv);
  auto g = mv.moveto_dvector<double>().gather();
  for(auto i: g) std::cout << i << std::endl;
}
