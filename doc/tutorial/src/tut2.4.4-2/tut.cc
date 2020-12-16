#include <frovedis.hpp>

bool is_even(int i) {return i % 2 == 0;}

int main(int argc, char* argv[]){
  frovedis::use_frovedis use(argc, argv);
  
  std::vector<int> v = {1,2,3,4,5,6,7,8};
  auto d1 = frovedis::make_dvector_scatter(v);
  d1.inplace_filter(is_even);
  auto r = d1.gather();
  for(auto i: r) std::cout << i << std::endl;
}
