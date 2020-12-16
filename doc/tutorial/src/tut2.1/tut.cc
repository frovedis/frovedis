#include <frovedis.hpp>

int two_times(int i) {return i*2;}

int main(int argc, char* argv[]){
  frovedis::use_frovedis use(argc, argv);
  
  std::vector<int> v = {1,2,3,4,5,6,7,8};
  auto d1 = frovedis::make_dvector_scatter(v);
  auto d2 = d1.map(two_times);
  auto r = d2.gather();
  for(auto i: r) std::cout << i << std::endl;
}
