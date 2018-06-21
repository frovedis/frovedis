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

  // filling sample input vector
  std::vector<int> v, double_v;
  for(size_t i = 1; i <= 8; i++) v.push_back(i);
  for(size_t i = 1; i <= 8; i++) double_v.push_back(i*2);

  // filling sample output vector
  auto nproc = get_nodesize();
  std::vector<std::vector<int>> ref(nproc);
  for(size_t i = 0; i < nproc; i++) ref[i] = double_v;

  auto l1 = frovedis::make_node_local_broadcast(v);
  auto l2 = l1.map(two_times);
  auto r = l2.gather();
  /* 
  for(auto i: r){
    for(auto j: i) std::cout << j << " ";
    std::cout << std::endl;
  }*/

  BOOST_CHECK(r == ref);
}
