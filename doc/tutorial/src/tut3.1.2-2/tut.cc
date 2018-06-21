#include <frovedis.hpp>
#include <frovedis/matrix/colmajor_matrix.hpp>
#include <frovedis/matrix/blas_wrapper.hpp>

int main(int argc, char* argv[]){
  frovedis::use_frovedis use(argc, argv);

  auto rm1 = frovedis::make_rowmajor_matrix_local_load<double>("./data1");
  frovedis::colmajor_matrix_local<double> m1(rm1);
  auto rm2 = frovedis::make_rowmajor_matrix_local_load<double>("./data2");
  frovedis::colmajor_matrix_local<double> m2(rm2);

  auto sm1 = frovedis::make_sliced_colmajor_matrix_local<double>(m1, 0, 0, 3, 3);
  auto mm = sm1 * m2;
  std::cout << mm.to_rowmajor();
}
