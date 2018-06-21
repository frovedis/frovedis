#include <frovedis.hpp>

int sum_vector(int key, std::vector<int>& v) {
  int sum = 0;
  for(size_t i = 0; i < v.size(); i++) sum += v[i];
  return sum;
}

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
  auto d2 = d1.group_by_key<int,int>().map_values(sum_vector);
  auto r = d2.as_dvector().gather();
  for(auto i: r) std::cout << i.first << ": " << i.second << std::endl;
}
