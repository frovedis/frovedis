#include <frovedis.hpp>
#include <frovedis/dataframe.hpp>

int main(int argc, char* argv[]){
  frovedis::use_frovedis use(argc, argv);

  auto t = frovedis::make_dftable_loadtext_infertype("./t.csv", 
                                                     {"c1", "c2", "c3"});
  t.show();
  for(auto i: t.dtypes()) std::cout << i.first << ":" << i.second << " ";
  std::cout << std::endl;
}
