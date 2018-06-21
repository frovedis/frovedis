#include <frovedis.hpp>

int main(int argc, char* argv[]){
  frovedis::use_frovedis use(argc, argv);
  
  std::vector<int> v;
  v.push_back(3);
  v.push_back(8);
  v.push_back(7);
  v.push_back(2);
  v.push_back(6);
  v.push_back(4);
  v.push_back(5);
  v.push_back(1);
  auto d1 = frovedis::make_dvector_scatter(v);
  d1.sort();
  auto r = d1.gather();
  for(auto i: r) std::cout << i << std::endl;
}
