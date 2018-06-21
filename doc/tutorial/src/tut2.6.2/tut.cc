#include <frovedis.hpp>

int sum(std::string key, int a, int b) {return a + b;}

int optional_sum(std::string key, int a, boost::optional<int> b) {
  if(b) return a + *b;
  else return a;
}

int main(int argc, char* argv[]){
  frovedis::use_frovedis use(argc, argv);
  
  auto d1 = frovedis::make_dunordered_map_allocate<std::string,int>();
  auto d2 = d1;
  d1.put("apple", 10);
  d1.put("orange", 20);
  d2.put("apple", 50);
  d2.put("grape", 20);
  auto z = zip(d1,d2);
  auto d3 = z.map_values(sum);
  auto r = d3.as_dvector().gather();
  for(auto i: r) std::cout << i.first << ": " << i.second << std::endl;

  auto d4 = z.leftouter_map_values(optional_sum);
  r = d4.as_dvector().gather();
  for(auto i: r) std::cout << i.first << ": " << i.second << std::endl;
}
