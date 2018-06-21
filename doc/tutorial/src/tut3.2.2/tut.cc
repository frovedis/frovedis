#include <frovedis.hpp>
#include <frovedis/matrix/ccs_matrix.hpp>
#include <frovedis/matrix/ell_matrix.hpp>
#include <frovedis/matrix/jds_matrix.hpp>
#include <frovedis/matrix/jds_crs_hybrid.hpp>

std::vector<double>
matvec(
  frovedis::ccs_matrix_local<double>& m,
  //frovedis::ell_matrix_local<double>& m,
  //frovedis::jds_matrix_local<double>& m,
  //frovedis::jds_crs_hybrid_local<double>& m,
  std::vector<double>& v) {
  return m * v;
}

int main(int argc, char* argv[]){
  frovedis::use_frovedis use(argc, argv);

  auto crs = frovedis::make_crs_matrix_load<double>("./data");
  frovedis::ccs_matrix<double> m(crs);
  //frovedis::ell_matrix<double> m(crs);
  //frovedis::jds_matrix<double> m(crs);
  //frovedis::jds_crs_hybrid<double> m(crs);
  std::vector<double> v(m.num_col);
  for(size_t i = 0; i < v.size(); i++) v[i] = 1;
  auto lv = frovedis::make_node_local_broadcast(v);
  auto mv = m.data.map(matvec, lv);
  auto g = mv.moveto_dvector<double>().gather();
  for(auto i: g) std::cout << i << std::endl;
}
