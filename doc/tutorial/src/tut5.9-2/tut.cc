#include <frovedis.hpp>
#include <frovedis/dataframe.hpp>

using namespace frovedis;

int main(int argc, char* argv[]){
  frovedis::use_frovedis use(argc, argv);

  auto t =
    frovedis::make_dftable_loadtext("./t.csv", 
                                    {"datetime:%Y-%m-%d", "datetime:%Y-%m-%d"},
                                    {"c1", "c2"});
  auto c1 = ~std::string("c1");
  auto c2 = ~std::string("c2");
  t.fselect({c1, c2}).show();
  t.fselect({datetime_extract_col(c1, frovedis::datetime_type::month)}).show();
  t.fselect({datetime_diff_col(c1, c2, frovedis::datetime_type::day)}).show();
  t.fselect({datetime_add_im(c1, 10, frovedis::datetime_type::day)}).show();
  t.fselect({datetime_truncate_col(c1, frovedis::datetime_type::month)}).show();
  t.fselect({datetime_months_between_col(c1, c2)}).show();
  t.fselect({datetime_next_day_im(c1, 1)}).show();
  auto t2 = t.fselect({cast_col(c1, "dic_string")->as("c3")});
  t2.show();
  auto c3 = ~std::string("c3");
  t2.fselect({cast_col(c3, "datetime")}).show();
  auto t3 =
    t.fselect({datetime_format_im(c1, "%Y/%m/%d", "dic_string")->as("c3")});
  t3.show();
  t3.fselect({cast_col(c3, "datetime:%Y/%m/%d")}).show();
}
