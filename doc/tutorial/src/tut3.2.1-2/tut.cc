#include <frovedis.hpp>
#include <frovedis/matrix/crs_matrix.hpp>

int main(int argc, char* argv[]){
  frovedis::use_frovedis use(argc, argv);

  auto m = frovedis::make_crs_matrix_loadbinary<double>("./data_binary");
  m.transpose().save("./transposed");
}
