#include <frovedis.hpp>

#define BOOST_TEST_MODULE FrovedisTest
#include <boost/test/unit_test.hpp>

using namespace frovedis;
using namespace std;

int sum(int a, int b){return a + b;}

int convert(std::string item, std::unordered_map<std::string,int>& dic) {
  return dic[item];
}

BOOST_AUTO_TEST_CASE( frovedis_test )
{
  int argc = 1;
  char** argv = NULL;
  use_frovedis use(argc, argv);

  // filling sample input vector
  std::vector<std::string> v;
  v.push_back("apple");
  v.push_back("orange");
  v.push_back("grape");
  v.push_back("orange");
  v.push_back("grape");
  v.push_back("grape");
  v.push_back("orange");
  v.push_back("grape");

  auto d1 = frovedis::make_dvector_scatter(v);
  std::unordered_map<std::string, int> dic;
  dic["apple"] = 10;
  dic["orange"] = 20;
  dic["grape"] = 30;
  auto l1 = frovedis::make_node_local_broadcast(dic);
  int r = d1.map(convert, l1).reduce(sum);
  int ref = 10+20+30+20+30+30+20+30;
  //std::cout << r << std::endl;
  BOOST_CHECK(r == ref);
}
