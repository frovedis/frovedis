#include <frovedis.hpp>
#include <frovedis/matrix/crs_matrix.hpp>
#include <frovedis/matrix/jds_crs_hybrid.hpp>
#include <frovedis/matrix/sparse_svd.hpp>

int main(int argc, char* argv[]){
  frovedis::use_frovedis use(argc, argv);

  auto crs = frovedis::make_crs_matrix_load<double>("./data");
  frovedis::colmajor_matrix<double> u;
  frovedis::colmajor_matrix<double> v;
  frovedis::diag_matrix_local<double> s;
  frovedis::sparse_svd(crs, u, s, v, 3);
  /*
  frovedis::sparse_svd<frovedis::jds_crs_hybrid<double>,
                     frovedis::jds_crs_hybrid_local<double>>
                     (crs, u, s, v, 3);
  */
  std::cout << u.to_rowmajor();
  std::cout << s;
  std::cout << v.to_rowmajor();
}
