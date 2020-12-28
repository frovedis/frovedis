#include <frovedis.hpp>
#include <frovedis/dataframe.hpp>

int main(int argc, char* argv[]){
  frovedis::use_frovedis use(argc, argv);

  auto t = frovedis::make_dftable_loadtext("./t.csv", 
                                         {"int", "double", "dic_string"},
                                         {"c1", "c2", "c3"});
  t.show();
  t.savetext("./saved.csv");
}
