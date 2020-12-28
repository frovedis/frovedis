#include <frovedis.hpp>
#include <frovedis/dataframe.hpp>

int main(int argc, char* argv[]){
  frovedis::use_frovedis use(argc, argv);

  auto t = frovedis::make_dftable_loadtext("./t.csv", 
                                           {"int", "datetime:%Y-%m-%d"},
                                           {"c1", "c2"});

  t.show();
  std::cout << std::endl;

  auto date = frovedis::makedatetime(2018,5,1);
  auto filtered = t.filter(frovedis::lt_im("c2", date));
  filtered.show();
  std::cout << std::endl;

  t.datetime_extract(frovedis::datetime_type::month, "c2", "month");
  t.show();
  std::cout << std::endl;

  auto dv = t.as_dvector<datetime_t>("c2");
  // each bytes are: |0|Y|Y|m|d|H|M|S|
  dv.mapv(+[](datetime_t& d){d += ((datetime_t) 1 << (5*8));});
  t.append_datetime_column("new_date", dv);
  t.show();
}
