#include <frovedis.hpp>
#include <frovedis/dataframe.hpp>

int main(int argc, char* argv[]){
  frovedis::use_frovedis use(argc, argv);

  auto t =
    frovedis::make_dftable_loadtext("./t.csv", 
                                    {"int","double","dic_string","double"},
                                    {"c1","c2","c3","c4"});

  std::cout << "c1 == c4" << std::endl;
  auto teq = t.filter(frovedis::eq("c1","c4"));
  teq.show();
  std::cout << std::endl;
  std::cout << "c1 != c4" << std::endl;
  auto tneq = t.filter(frovedis::neq("c1","c4"));
  tneq.show();
  std::cout << std::endl;
  std::cout << "c1 < c4" << std::endl;
  auto tlt = t.filter(frovedis::lt("c1","c4"));
  tlt.show();
  std::cout << std::endl;
  std::cout << "c1 <= c4" << std::endl;
  auto tle = t.filter(frovedis::le("c1","c4"));
  tle.show();
  std::cout << std::endl;
  std::cout << "c1 > c4" << std::endl;
  auto tgt = t.filter(frovedis::gt("c1","c4"));
  tgt.show();
  std::cout << std::endl;
  std::cout << "c1 >= c4" << std::endl;
  auto tge = t.filter(frovedis::ge("c1","c4"));
  tge.show();
  std::cout << std::endl;

  std::cout << "c2 == 30.0" << std::endl;
  auto teqim = t.filter(frovedis::eq_im("c2",30.0));
  teqim.show();
  std::cout << std::endl;
  std::cout << "c2 != 30.0" << std::endl;
  auto tneqim = t.filter(frovedis::neq_im("c2",30.0));
  tneqim.show();
  std::cout << std::endl;
  std::cout << "c2 < 30.0" << std::endl;
  auto tltim = t.filter(frovedis::lt_im("c2",30.0));
  tltim.show();
  std::cout << std::endl;
  std::cout << "c2 <= 30.0" << std::endl;
  auto tleim = t.filter(frovedis::le_im("c2",30.0));
  tleim.show();
  std::cout << std::endl;
  std::cout << "c2 > 30.0" << std::endl;
  auto tgtim = t.filter(frovedis::gt_im("c2",30.0));
  tgtim.show();
  std::cout << std::endl;
  std::cout << "c2 >= 30.0" << std::endl;
  auto tgeim = t.filter(frovedis::ge_im("c2",30.0));
  tgeim.show();
  std::cout << std::endl;

  std::cout << "c3 contains c" << std::endl;
  auto tislike = t.filter(frovedis::is_like("c3","%c"));
  tislike.show();
  std::cout << std::endl;
  std::cout << "c3 does not contain c" << std::endl;
  auto tisnotlike = t.filter(frovedis::is_not_like("c3","%c"));
  tisnotlike.show();

  std::cout << std::endl;
  std::cout << "(c1 == c4 || c2 <= 30.0) && c3 does not contain c" 
            << std::endl;  
  auto tandor = t.filter(frovedis::and_op
                         (frovedis::or_op
                          (frovedis::eq("c1","c4"),
                           frovedis::le_im("c2",30.0)),
                          frovedis::is_not_like("c3","%c")));
  tandor.show();
}
