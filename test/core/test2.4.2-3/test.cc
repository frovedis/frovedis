#include <frovedis.hpp>

#define BOOST_TEST_MODULE FrovedisTest
#include <boost/test/unit_test.hpp>

using namespace frovedis;
using namespace std;

void two_times(int& i) {i*=2;}

BOOST_AUTO_TEST_CASE( frovedis_test )
{
  int argc = 1;
  char** argv = NULL;
  use_frovedis use(argc, argv);

  // filling sample input and output vectors  
  std::vector<int> v, ref_out;
  for(size_t i = 1; i <= 8; i++) v.push_back(i);
  for(size_t i = 1; i <= 8; i++) ref_out.push_back(i*2);

  // testing on dvector::mapv to perform in-place update
  auto d1 = frovedis::make_dvector_scatter(v);
  auto r = d1.mapv(two_times).gather();

  // confirming updated vector with expected output
  BOOST_CHECK(r == ref_out);
}
