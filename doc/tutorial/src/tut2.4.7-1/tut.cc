#include <frovedis.hpp>

int main(int argc, char* argv[]){
  frovedis::use_frovedis use(argc, argv);
  
  std::vector<int> v;
  for(size_t i = 1; i <= 8; i++) v.push_back(i);
  auto d1 = frovedis::make_dvector_scatter(v);
  std::cout << d1.size() << std::endl;
  d1.clear();
  std::cout << d1.size() << std::endl;
}
