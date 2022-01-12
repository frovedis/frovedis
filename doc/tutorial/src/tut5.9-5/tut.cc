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
  auto c3 = ~std::string("c3");
  t.fsort(c2 / c1).show();
  t.fgroup_by({(c2 / c1)->as("c3")}).
    fselect({c3,add_col(sum(c1),sum(c2))}).show();
}
