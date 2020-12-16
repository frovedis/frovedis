#include <frovedis.hpp>

std::vector<int> two_times(const std::vector<int>& v) {
  std::vector<int> r(v.size());
  for(size_t i = 0; i < v.size(); i++) r[i] = v[i] * 2;
  return r;
}

int main(int argc, char* argv[]){
  frovedis::use_frovedis use(argc, argv);

  std::vector<int> v = {1,2,3,4,5,6,7,8};
  auto l1 = frovedis::make_node_local_broadcast(v);
  auto l2 = l1.map(two_times);
  auto r = l2.gather();
  for(auto i: r){
    for(auto j: i) std::cout << j << " ";
    std::cout << std::endl;
  }
}
