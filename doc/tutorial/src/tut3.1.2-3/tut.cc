#include <frovedis.hpp>
#include <frovedis/matrix/colmajor_matrix.hpp>
#include <frovedis/matrix/lapack_wrapper.hpp>

int main(int argc, char* argv[]){
  frovedis::use_frovedis use(argc, argv);

  auto rm = frovedis::make_rowmajor_matrix_local_load<double>("./data");
  frovedis::colmajor_matrix_local<double> m(rm);

  std::vector<double> v(3);
  for(size_t i = 0; i < 3; i++) v[i] = i+1;
  frovedis::gesv<double>(m,v);
  for(auto i: v) std::cout << i << std::endl;
}
