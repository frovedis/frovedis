#include <frovedis.hpp>

#define BOOST_TEST_MODULE FrovedisTest
#include <boost/test/unit_test.hpp>

using namespace frovedis;
using namespace std;

int sum(int a, int b){return a + b;}

BOOST_AUTO_TEST_CASE( frovedis_test )
{
  int argc = 1;
  char** argv = NULL;
  use_frovedis use(argc, argv);

  // filling sample input vector  
  std::vector<int> v, ref;
  for(size_t i = 1; i <= 8; i++) v.push_back(i);
  for(size_t i = 1; i <= 8; i++) ref.push_back(i*2);

  auto d1 = frovedis::make_dvector_scatter(v);
  auto d2 = d1;
  auto d3 = frovedis::zip(d1, d2).map(sum);
  auto r = d3.gather();
  //for(auto i: r) std::cout << i << std::endl;

  BOOST_CHECK(r == ref);
}
