#include <frovedis.hpp>

int sum(int a, int b){return a + b;}

int convert(std::string item, std::unordered_map<std::string,int>& dic) {
  return dic[item];
}

int main(int argc, char* argv[]){
  frovedis::use_frovedis use(argc, argv);

  std::vector<std::string> v;
  v.push_back("apple");
  v.push_back("orange");
  v.push_back("grape");
  v.push_back("orange");
  v.push_back("grape");
  v.push_back("grape");
  v.push_back("orange");
  v.push_back("grape");
  auto d1 = frovedis::make_dvector_scatter(v);
  std::unordered_map<std::string, int> dic;
  dic["apple"] = 10;
  dic["orange"] = 20;
  dic["grape"] = 30;
  auto l1 = frovedis::make_node_local_broadcast(dic);
  int r = d1.map(convert, l1).reduce(sum);
  std::cout << r << std::endl;
}
