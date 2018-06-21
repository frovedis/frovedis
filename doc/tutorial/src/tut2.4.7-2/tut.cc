#include <frovedis.hpp>

struct less_than {
  less_than(){}
  less_than(int n_) : n(n_) {}
  bool operator()(int i){return i < n;}
  int n;
  SERIALIZE(n)
};

int main(int argc, char* argv[]){
  frovedis::use_frovedis use(argc, argv);
  
  std::vector<int> v;
  for(size_t i = 1; i <= 8; i++) v.push_back(i);
  auto d1 = frovedis::make_dvector_scatter(v);
  for(auto i: d1.sizes()) std::cout << i << " ";
  std::cout << std::endl;
  auto d2 = d1.filter(less_than(5));
  auto d3 = d2;
  for(auto i: d2.sizes()) std::cout << i << " ";
  std::cout << std::endl;
  d2.align_block();
  for(auto i: d2.sizes()) std::cout << i << " ";
  std::cout << std::endl;
  d2.align_to(d3);
  for(auto i: d2.sizes()) std::cout << i << " ";
  std::cout << std::endl;
  std::vector<size_t> sizes;
  sizes.push_back(1);
  sizes.push_back(1);
  sizes.push_back(2);
  sizes.push_back(0);
  d2.align_as(sizes);
  for(auto i: d2.sizes()) std::cout << i << " ";
  std::cout << std::endl;
}
