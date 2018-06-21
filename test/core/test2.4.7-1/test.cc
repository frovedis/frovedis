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

  // filling sample input vector  
  std::vector<int> v;
  for(size_t i = 1; i <= 8; i++) v.push_back(i);
  auto d1 = frovedis::make_dvector_scatter(v);

  // testing on dvector::size() and dvector::clear()
  auto s1 = d1.size();
  d1.clear();
  auto s2 = d1.size();
  
  // confirming ontained result with expected output
  BOOST_CHECK(s1 == 8 && s2 == 0);
}
