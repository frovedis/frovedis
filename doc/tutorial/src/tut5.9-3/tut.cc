#include <frovedis.hpp>
#include <frovedis/dataframe.hpp>

using namespace frovedis;

int main(int argc, char* argv[]){
  frovedis::use_frovedis use(argc, argv);

  auto t =
    frovedis::make_dftable_loadtext("./t.csv", 
                                    {"dic_string"},
                                    {"c1"});
  auto c1 = ~std::string("c1");
  t.fselect({c1}).show();
  t.fselect({substr_posim_numim(c1,2,3)}).show();
}
