#include <frovedis.hpp>

#define BOOST_TEST_MODULE FrovedisTest
#include <boost/test/unit_test.hpp>

using namespace frovedis;
using namespace std;

std::vector<int> two_times(const std::vector<int>& v) {
  std::vector<int> r(v.size());
  for(size_t i = 0; i < v.size(); i++) r[i] = v[i] * 2;
  return r;
}

BOOST_AUTO_TEST_CASE( frovedis_test )
{
  int argc = 1;
  char** argv = NULL;
  use_frovedis use(argc, argv);

  // filling sample input/output vectors
  std::vector<int> v, ref;
  for(size_t i = 1; i <= 8; i++) v.push_back(i);
  for(size_t i = 1; i <= 8; i++) ref.push_back(i*2);

  auto d1 = frovedis::make_dvector_scatter(v);
  auto d2 = d1.as_node_local().map(two_times).moveto_dvector<int>();
  auto d3 = d1.map_partitions(two_times);

  auto r1 = d2.gather();
  auto r2 = d3.gather();
  BOOST_CHECK(r1 == ref && r2 == ref);
}
