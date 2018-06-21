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

  // filling sample input vector to be sorted in parallel
  std::vector<int> v;
  v.push_back(3);
  v.push_back(8);
  v.push_back(7);
  v.push_back(2);
  v.push_back(6);
  v.push_back(4);
  v.push_back(5);
  v.push_back(1);

  // filling expected output vector
  std::vector<int> ref_out;
  for(int i=1; i<=8; ++i) ref_out.push_back(i);

  // testing dvector::sort()
  auto d1 = frovedis::make_dvector_scatter(v);
  d1.sort();
  auto r = d1.gather();

  // confirming obtained result with expected output
  BOOST_CHECK(r == ref_out);
}
