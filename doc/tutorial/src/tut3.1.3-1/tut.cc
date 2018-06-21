#include <frovedis.hpp>
#include <frovedis/matrix/blockcyclic_matrix.hpp>
#include <frovedis/matrix/pblas_wrapper.hpp>

int main(int argc, char* argv[]){
  frovedis::use_frovedis use(argc, argv);

  auto m = frovedis::make_blockcyclic_matrix_load<double>("./data");
  std::vector<double> v(m.num_col);
  for(size_t i = 0; i < v.size(); i++) v[i] = 1;
  auto dv = frovedis::make_blockcyclic_matrix_scatter(v);
  auto mv = m * dv;
  mv.save("./result");
}
