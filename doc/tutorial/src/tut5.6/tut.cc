#include <frovedis.hpp>
#include <frovedis/dataframe.hpp>

int main(int argc, char* argv[]){
  frovedis::use_frovedis use(argc, argv);

  auto t = frovedis::make_dftable_loadtext("./t.csv", 
                                           {"int", "int", "double"},
                                           {"c1", "c2", "c3"});

  auto grouped = t.group_by({"c1"});
  std::cout << "sum" << std::endl;
  auto grouped_sum = grouped.select({"c1"}, {frovedis::sum("c3")});
  grouped_sum.show();
  std::cout << std::endl;

  std::cout << "other aggregations" << std::endl;
  auto aggregated = grouped.select({"c1"}, 
                                   {frovedis::sum_as("c3","sum"),
                                    frovedis::count_as("c3","count"),
                                    frovedis::avg_as("c3","avg"),
                                    frovedis::max_as("c3","max"),
                                    frovedis::min_as("c3","min")});
  aggregated.show();
  std::cout << std::endl;

  std::cout << "group by multiple columns" << std::endl;
  auto multi_grouped = t.group_by({std::string("c1"), std::string("c2")});
  auto multi_aggregated = multi_grouped.select({std::string("c1"), 
                                                std::string("c2")}, 
                                               {frovedis::sum_as("c3","sum"),
                                                frovedis::count_as("c3","count"),
                                                frovedis::avg_as("c3","avg"),
                                                frovedis::max_as("c3","max"),
                                                frovedis::min_as("c3","min")});
  multi_aggregated.show();
}
