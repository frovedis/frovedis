#include <frovedis.hpp>

#define BOOST_TEST_MODULE FrovedisTest
#include <boost/test/unit_test.hpp>

using namespace frovedis;
using namespace std;

bool is_even(int i, std::vector<int>& v) {return i % 2 == 0;}

BOOST_AUTO_TEST_CASE( frovedis_test )
{
  int argc = 1;
  char** argv = NULL;
  use_frovedis use(argc, argv);

  // filling sample input vector
  std::vector<std::pair<int,int>> v;
  v.push_back(std::make_pair(1,10));
  v.push_back(std::make_pair(4,10));
  v.push_back(std::make_pair(3,10));
  v.push_back(std::make_pair(2,10));
  v.push_back(std::make_pair(3,20));
  v.push_back(std::make_pair(2,20));
  v.push_back(std::make_pair(4,20));
  v.push_back(std::make_pair(1,20));

  // filling sample output vector
  std::vector<std::pair<int,std::vector<int>>> ref_out;
  std::vector<int> temp = {10,20};
  ref_out.push_back(std::make_pair(2,temp));
  ref_out.push_back(std::make_pair(4,temp));

  auto d1 = frovedis::make_dvector_scatter(v);
  auto d2 = d1.group_by_key<int,int>();
  d2.inplace_filter(is_even);
  auto r = d2.as_dvector().sort().gather();
  /*
  for(auto i: r) {
    std::cout << i.first << ": ";
    for(auto j: i.second) std::cout << j << " ";
    std::cout << std::endl;
  }*/
  
  BOOST_CHECK(r == ref_out);
}
