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
  t.fselect({c1, c2}).show();
  t.fselect({c1+c2}).show();
  t.fselect({(c1*c1+c2)->as("newcol")}).show();
  t.fselect({idiv_col(c2,c1), idiv_im_as(c2,2.0,"newcol")}).show();
  auto t2 = t.fselect({cast_col(c1,"dic_string")->as("c3")});
  t2.show();
  auto c3 = ~std::string("c3");
  t2.fselect({cast_col(c3,"double")}).show();
}
