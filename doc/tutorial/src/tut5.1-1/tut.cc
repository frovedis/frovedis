#include <frovedis.hpp>
#include <frovedis/dataframe.hpp>

int main(int argc, char* argv[]){
  frovedis::use_frovedis use(argc, argv);

  std::vector<int> v1 = {1,2,3,4,5,6,7,8};
  auto d1 = frovedis::make_dvector_scatter(v1);
  std::vector<double> v2 = {10,20,30,40,50,60,70,80};
  auto d2 = frovedis::make_dvector_scatter(v2);
  std::vector<std::string> v3 = {"a","b","c","d","e","f","g","h"};
  auto d3 = frovedis::make_dvector_scatter(v3);

  frovedis::dftable t;
  t.append_column("c1", d1);
  t.append_column("c2", d2);
  t.append_column("c3", d3);
  
  t.save("./t");

  auto t2 = frovedis::make_dftable_load("./t");
  t2.show();

  std::cout << std::endl;
  auto dc1 = t.as_dvector<int>("c1");
  for(auto i: dc1.gather()) std::cout << i << " ";
  std::cout << std::endl;
}
