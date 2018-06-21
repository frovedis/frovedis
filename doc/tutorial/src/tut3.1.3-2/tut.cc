#include <frovedis.hpp>
#include <frovedis/matrix/blockcyclic_matrix.hpp>
#include <frovedis/matrix/scalapack_wrapper.hpp>

int main(int argc, char* argv[]){
  frovedis::use_frovedis use(argc, argv);

  auto m = frovedis::make_blockcyclic_matrix_load<double>("./data");
  std::vector<double> v(3);
  for(size_t i = 0; i < 3; i++) v[i] = i+1;
  auto dv = frovedis::make_blockcyclic_matrix_scatter(v);

  frovedis::gesv<double>(m,dv);
  dv.save("./result");
}
