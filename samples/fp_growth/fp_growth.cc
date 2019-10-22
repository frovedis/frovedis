#include <frovedis.hpp>
#include <frovedis/dataframe.hpp>
#include <frovedis/ml/fpm/fp_growth.hpp>
#include <frovedis/ml/fpm/fp_growth_model.hpp>

using namespace frovedis;

void display (const std::vector<std::string>& data) {
  for(auto i: data) std::cout << i << " ";
  std::cout << std::endl;
}

int main(int argc, char** argv) {
  use_frovedis use(argc,argv);

  set_loglevel(INFO);
  auto t = make_dftable_loadtext("./data.csv", 
                                 {"int", "int"}, 
                                 {"trans_id", "item"});
  std::cout << "transaction data:" << std::endl;
  t.show();

  time_spent grow(INFO), tree(INFO);
  double conf = 0.5;
  double support = 0.2;
  grow.lap_start();
  auto model = grow_fp_tree(t,support);
  model.debug_print();
  grow.lap_stop();
  grow.show_lap("grow_fp_tree: ");

  tree.lap_start();
  auto rule = model.generate_rules(conf);
  rule.debug_print();
  tree.lap_stop();
  tree.show_lap("generate_rules: ");

  return 0;
}

