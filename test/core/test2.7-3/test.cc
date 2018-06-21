#include <frovedis.hpp>

#define BOOST_TEST_MODULE FrovedisTest
#include <boost/test/unit_test.hpp>

using namespace frovedis;
using namespace std;

std::vector<int> vector_reduce1(const std::vector<int>& a,
                               const std::vector<int>& b) {
  std::vector<int> r(a.size());
  for(size_t i = 0; i < a.size(); i++) {
    r[i] = a[i] + b[i];
  }
  return r;
}

BOOST_AUTO_TEST_CASE( frovedis_test )
{
  int argc = 1;
  char** argv = NULL;
  use_frovedis use(argc, argv);

  // filling sample input/output vectors
  std::vector<int> v, ref;
  auto n = get_nodesize();
  for(size_t i = 1; i <= 8; i++) v.push_back(i);
  for(size_t i = 1; i <= 8; i++) ref.push_back(i*n);

  auto d1 = broadcast(v);
  auto r1 = d1.reduce(vector_reduce1);
  auto r2 = d1.vector_sum();
  /*for(auto i: r) std::cout << i << " "; std::cout << std::endl;*/
  BOOST_CHECK(r1 == ref && r2 == ref);
}
