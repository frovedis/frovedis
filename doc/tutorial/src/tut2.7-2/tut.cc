#include <frovedis.hpp>

std::vector<int> two_times(const std::vector<int>& v) {
  std::vector<int> r(v.size());
  for(size_t i = 0; i < v.size(); i++) r[i] = v[i] * 2;
  return r;
}

int main(int argc, char* argv[]){
  frovedis::use_frovedis use(argc, argv);

  std::vector<int> v;
  for(size_t i = 1; i <= 8; i++) v.push_back(i);
  auto d1 = frovedis::make_dvector_scatter(v);
  auto d2 = d1.as_node_local().map(two_times).moveto_dvector<int>();
  //auto d2 = d1.map_partitions(two_times);
  auto r = d2.gather();
  for(auto i: r) std::cout << i << std::endl;
}
