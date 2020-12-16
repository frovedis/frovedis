#include <frovedis.hpp>

int sum(int a, int b){return a + b;}

int main(int argc, char* argv[]){
  frovedis::use_frovedis use(argc, argv);
  
  std::vector<int> v = {1,2,3,4,5,6,7,8};
  auto d1 = frovedis::make_dvector_scatter(v);
  auto d2 = d1;
  auto d3 = frovedis::zip(d1, d2).map(sum);
  auto r = d3.gather();
  for(auto i: r) std::cout << i << std::endl;
}
