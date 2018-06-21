#include <frovedis.hpp>

int sum(int a, int b){return a + b;}

int main(int argc, char* argv[]){
  frovedis::use_frovedis use(argc, argv);

  std::vector<std::pair<int,int>> v;
  v.push_back(std::make_pair(1,10));
  v.push_back(std::make_pair(4,10));
  v.push_back(std::make_pair(3,10));
  v.push_back(std::make_pair(2,10));
  v.push_back(std::make_pair(3,20));
  v.push_back(std::make_pair(2,20));
  v.push_back(std::make_pair(4,20));
  v.push_back(std::make_pair(1,20));
  auto d1 = frovedis::make_dvector_scatter(v);
  auto d2 = d1.reduce_by_key<int,int>(sum);
  auto r = d2.as_dvector().gather();
  for(auto i: r) std::cout << i.first << ": " << i.second << std::endl;
}
