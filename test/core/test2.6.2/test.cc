#include <frovedis.hpp>

#define BOOST_TEST_MODULE FrovedisTest
#include <boost/test/unit_test.hpp>

using namespace frovedis;
using namespace std;

int sum(std::string key, int a, int b) {return a + b;}

int optional_sum(std::string key, int a, boost::optional<int> b) {
  if(b) return a + *b;
  else return a;
}

BOOST_AUTO_TEST_CASE( frovedis_test )
{
  int argc = 1;
  char** argv = NULL;
  use_frovedis use(argc, argv);

  // filling sample input imap  
  auto d1 = frovedis::make_dunordered_map_allocate<std::string,int>();
  auto d2 = d1;
  d1.put("apple", 10);
  d1.put("orange", 20);
  d2.put("apple", 50);
  d2.put("grape", 20);
  auto z = zip(d1,d2);
  auto d3 = z.map_values(sum);
  auto r = d3.as_dvector().sort().gather();
  //for(auto i: r) std::cout << i.first << ": " << i.second << std::endl;
  std::vector<std::pair<std::string,int>> ref1;
  ref1.push_back(std::make_pair("apple",60));
  BOOST_CHECK (r == ref1);

  auto d4 = z.leftouter_map_values(optional_sum);
  r = d4.as_dvector().sort().gather();
  //for(auto i: r) std::cout << i.first << ": " << i.second << std::endl;
  std::vector<std::pair<std::string,int>> ref2;
  ref2.push_back(std::make_pair("apple",60));
  ref2.push_back(std::make_pair("orange",20));
  BOOST_CHECK (r == ref2);
}
