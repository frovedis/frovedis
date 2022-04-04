#include <frovedis.hpp>
#include <frovedis/dataframe.hpp>

using namespace frovedis;

int main(int argc, char* argv[]){
  frovedis::use_frovedis use(argc, argv);

  std::vector<int> v(1 << 30); // 1G * 4byte = 4GB
  auto dv = make_dvector_scatter(v);
  
  dftable t;
  try {
    for(size_t i = 0; i < 11; i++) {
      t.append_column(std::to_string(i), dv);
    }
    t.show();
  } catch (std::exception& e) {
    std::cerr << e.what() << std::endl;
  }
}
