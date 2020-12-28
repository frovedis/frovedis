#include <frovedis.hpp>
#include <frovedis/dataframe.hpp>

int main(int argc, char* argv[]){
  frovedis::use_frovedis use(argc, argv);

  auto t = frovedis::make_dftable_loadtext("./t.csv", 
                                         {"int", "double", "dic_string"},
                                         {"c1", "c2", "c3"});
  auto t2 = t.select({"c2", "c3"});
  t2.show();
  std::cout << std::endl;

  t2.drop("c2");
  t2.show();
  std::cout << std::endl;

  t2.rename("c3","cx");
  t2.show();
}
