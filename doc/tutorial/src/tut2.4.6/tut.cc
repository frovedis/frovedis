#include <frovedis.hpp>

int main(int argc, char* argv[]){
  frovedis::use_frovedis use(argc, argv);

  std::vector<int> v = {1,2,3,4,5,6,7,8};
  auto d1 = frovedis::make_dvector_scatter(v);
  std::cout << d1.get(3) << std::endl;
  d1.put(3, 40);
  std::cout << d1.get(3) << std::endl;
}
