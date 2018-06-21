#include <frovedis.hpp>

#define BOOST_TEST_MODULE FrovedisTest
#include <boost/test/unit_test.hpp>

using namespace frovedis;
using namespace std;

std::vector<int> duplicate(int i) {
  std::vector<int> v;
  v.push_back(i); v.push_back(i);
  return v;
}

BOOST_AUTO_TEST_CASE( frovedis_test )
{
  int argc = 1;
  char** argv = NULL;
  use_frovedis use(argc, argv);

  // filling sample input and output vectors  
  std::vector<int> v, ref_out;
  for(size_t i = 1; i <= 8; i++) v.push_back(i);
  for(size_t i = 1; i <= 8; i++) { 
    ref_out.push_back(i);
    ref_out.push_back(i);
  }

  // testing for dvector::flat_map
  auto d1 = frovedis::make_dvector_scatter(v);
  auto d2 = d1.flat_map(duplicate);
  auto r = d2.gather();

  // confirming flattened vector with expected output
  BOOST_CHECK(r == ref_out);
}
