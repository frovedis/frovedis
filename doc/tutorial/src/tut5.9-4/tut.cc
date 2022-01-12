#include <frovedis.hpp>
#include <frovedis/dataframe.hpp>

using namespace frovedis;

int main(int argc, char* argv[]){
  frovedis::use_frovedis use(argc, argv);

  auto t =
    frovedis::make_dftable_loadtext("./t.csv", 
                                    {"int", "double"},
                                    {"c1", "c2"});
  auto c1 = ~std::string("c1");
  auto c2 = ~std::string("c2");
  t.fselect({c1,when({(c1 >= 3) >> c2,(c1 >= 2) >> im(100)})}).show();
  t.fselect({c1,when({(c1 >= 3) >> c2,(c1 >= 2) >> im(100)}, im(-1))}).show();
}
