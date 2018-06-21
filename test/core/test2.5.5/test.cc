#include <frovedis.hpp>

#define BOOST_TEST_MODULE FrovedisTest
#include <boost/test/unit_test.hpp>

using namespace frovedis;
using namespace std;

BOOST_AUTO_TEST_CASE( frovedis_test )
{
  int argc = 1;
  char** argv = NULL;
  use_frovedis use(argc, argv);

  // filling sample input map 
  auto d1 = frovedis::make_dunordered_map_allocate<std::string,int>();

  d1.put("apple", 10);
  d1.put("orange", 20);

  auto x = d1.get("apple");
  d1.put("apple", 50);
  auto y = d1.get("apple");

  BOOST_CHECK(x == 10 && y == 50);
}
