#include <frovedis.hpp>

#define BOOST_TEST_MODULE FrovedisTest
#include <boost/test/unit_test.hpp>

using namespace frovedis;
using namespace std;

struct n_times {
  n_times(){}
  n_times(int n_) : n(n_) {}
  int operator()(int i){return i*n;}
  int n;
  SERIALIZE(n)
};

BOOST_AUTO_TEST_CASE( frovedis_test )
{
  int argc = 1;
  char** argv = NULL;
  use_frovedis use(argc, argv);

  // filling sample input and output vectors  
  std::vector<int> v, ref_out;
  for(size_t i = 1; i <= 8; i++) v.push_back(i);
  for(size_t i = 1; i <= 8; i++) ref_out.push_back(i*3);

  // testing dvector::map on functor
  auto d1 = frovedis::make_dvector_scatter(v);
  auto d2 = d1.map<int>(n_times(3));
  auto r = d2.gather();

  // confirming resultant vector with expected output
  BOOST_CHECK(r == ref_out);
}
