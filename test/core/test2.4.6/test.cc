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

  // filling sample input vector and expected get/put outputs
  std::vector<int> v;
  for(size_t i = 1; i <= 8; i++) v.push_back(i);
  int ro1 = 4, ro2 = 40;

  // testing on dvector get/put
  auto d1 = frovedis::make_dvector_scatter(v);
  auto o1 = d1.get(3);
  d1.put(3, 40);
  auto o2 = d1.get(3);

  // confirming obtained outputs with expected outputs
  BOOST_CHECK (o1 == ro1 && o2 == ro2);
}
