#include <frovedis.hpp>
#include <frovedis/dataframe.hpp>

using namespace frovedis;

int main(int argc, char* argv[]){
  frovedis::use_frovedis use(argc, argv);

  auto t =
    frovedis::make_dftable_loadtext("./t.csv", 
                                    {"dic_string","dic_string","dic_string"},
                                    {"c1","c2","c3"});
  auto c1 = ~std::string("c1");
  auto c2 = ~std::string("c2");
  auto c3 = ~std::string("c3");
  t.fselect({c1,c2,c3}).show();
  t.fselect({substr_posim_numim(c1,2,3)}).show();
  t.fselect({substr_posim_numim(c2,-3,2)}).show();
  t.fselect({char_length_col(c1)}).show();
  t.fselect({length_col(c2), char_length_col(c2)}).show();
  t.fselect({locate_im(c3,"com")}).show();
  t.fselect({locate_im(c2,"パイ")}).show();
  t.fselect({ltrim_im(c3,"w.")}).show();
  t.fselect({rtrim_im(c2,"パイ")}).show();
  t.fselect({replace_im(c1,"pie","cake")}).show();
  t.fselect({replace_im(c2,"パイ","ケーキ")}).show();
  t.fselect({reverse_col(c1)}).show();
  t.fselect({reverse_col(c2)}).show();
  t.fselect({substring_index_im(c3,".",2)}).show();
  t.fselect({substring_index_im(c2,"パ",1)}).show();
  t.fselect({concat_col(c1,c2)}).show();
  t.fselect({concat_ws("_",c1,c2)}).show();
  t.fselect({concat_multi_col({c1,c2,c3})}).show();
  t.fselect({concat_multi_ws("_",{c1,c2,c3})}).show();
  t.fselect({upper_col(c1)}).show();
  t.fselect({repeat_im(c1,3)}).show();
  t.fselect({lpad_im(c1,10)}).show();
  t.fselect({rpad_im(c2,10,"＊")}).show();
  t.fselect({ascii_col(c1)}).show();
}
