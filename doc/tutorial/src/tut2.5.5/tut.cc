#include <frovedis.hpp>

int main(int argc, char* argv[]){
  frovedis::use_frovedis use(argc, argv);
  
  auto d1 = frovedis::make_dunordered_map_allocate<std::string,int>();
  d1.put("apple", 10);
  d1.put("orange", 20);
  std::cout << d1.get("apple") << std::endl;
  d1.put("apple", 50);
  std::cout << d1.get("apple") << std::endl;
}
