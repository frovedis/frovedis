#include <frovedis.hpp>

#define BOOST_TEST_MODULE FrovedisTest
#include <boost/test/unit_test.hpp>

using namespace frovedis;
using namespace std;

int two_times(int i) {return i*2;}

BOOST_AUTO_TEST_CASE( frovedis_test )
{
  int argc = 1;
  char** argv = NULL;
  use_frovedis use(argc, argv);

  // sample input and expected output vectors
  std::vector<int> v, ref;
  for(size_t i = 1; i <= 8; i++) v.push_back(i);
  for(size_t i = 1; i <= 8; i++) ref.push_back(i*2);

  // performing map on a dvector
  auto d1 = frovedis::make_dvector_scatter(v);
  auto d2 = d1.map(two_times);

  // gathering distributed output
  auto r = d2.gather();

  // checking gathered output with expected output
  BOOST_CHECK (ref == r);
}
