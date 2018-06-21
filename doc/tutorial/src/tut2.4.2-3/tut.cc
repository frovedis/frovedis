#include <frovedis.hpp>

void two_times(int& i) {i*=2;}

int main(int argc, char* argv[]){
  frovedis::use_frovedis use(argc, argv);
  
  std::vector<int> v;
  for(size_t i = 1; i <= 8; i++) v.push_back(i);
  auto d1 = frovedis::make_dvector_scatter(v);
  auto r = d1.mapv(two_times).gather();
  for(auto i: r) std::cout << i << std::endl;
}
