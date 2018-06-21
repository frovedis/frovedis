#include <frovedis.hpp>

#define BOOST_TEST_MODULE FrovedisTest
#include <boost/test/unit_test.hpp>

using namespace frovedis;
using namespace std;

int sum(int a, int b) {return a + b;}

BOOST_AUTO_TEST_CASE( frovedis_test )
{
  int argc = 1;
  char** argv = NULL;
  use_frovedis use(argc, argv);

  // filling sample input vector and computing expected output
  std::vector<int> v;
  int ref_out = 0;
  for(size_t i = 1; i <= 8; i++) { 
    v.push_back(i);
    ref_out += i;
  }

  // testing on dvector::reduce
  auto d1 = frovedis::make_dvector_scatter(v);
  auto r = d1.reduce(sum);

  // confirming whether reduced result matched with expected output
  BOOST_CHECK(r == ref_out);
}
