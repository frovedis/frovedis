#include <frovedis.hpp>
#include <frovedis/matrix/rowmajor_matrix.hpp>

int main(int argc, char* argv[]){
  frovedis::use_frovedis use(argc, argv);

  auto m = frovedis::make_rowmajor_matrix_loadbinary<double>("./data_binary");
  m.transpose().save("./transposed");
}
