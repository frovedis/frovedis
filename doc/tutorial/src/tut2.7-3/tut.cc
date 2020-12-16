#include <frovedis.hpp>

std::vector<int> vector_reduce(const std::vector<int>& a,
                               const std::vector<int>& b) {
  std::vector<int> r(a.size());
  for(size_t i = 0; i < a.size(); i++) {
    r[i] = a[i] + b[i];
  }
  return r;
}

int main(int argc, char* argv[]){
  frovedis::use_frovedis use(argc, argv);

  std::vector<int> v = {1,2,3,4,5,6,7,8};
  auto d1 = frovedis::make_dvector_scatter(v);
  auto l1 = d1.as_node_local();
  auto r = l1.reduce(vector_reduce);
  for(auto i: r) std::cout << i << " ";
  std::cout << std::endl;
}
