#include <frovedis.hpp>
#include <frovedis/dataframe.hpp>

int main(int argc, char* argv[]){
  frovedis::use_frovedis use(argc, argv);

  auto t = frovedis::make_dftable_loadtext("./t.csv", 
                                         {"int", "double", "dic_string"},
                                         {"c1", "c2", "c3"});

  auto t2 = t.sort("c2");
  t2.show();
  std::cout << std::endl;
  auto t3 = t2.sort("c1");
  t3.show();
  std::cout << std::endl;
  auto t4 = t.filter(frovedis::eq_im("c1",2)).sort("c2");
  t4.show();
}
