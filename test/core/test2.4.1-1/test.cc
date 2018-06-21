#include <frovedis.hpp>
#include <boost/lexical_cast.hpp>

#define BOOST_TEST_MODULE FrovedisTest
#include <boost/test/unit_test.hpp>

using namespace frovedis;
using namespace std;
using namespace boost;

int parse_data(const string& s) {
  return lexical_cast<int>(s);
}

int two_times(int i) {return i*2;}

BOOST_AUTO_TEST_CASE( frovedis_test )
{
  int argc = 1;
  char** argv = NULL;
  use_frovedis use(argc, argv);

  // loading data from file to create dvector and performing map operation on it
  auto d1 = frovedis::make_dvector_loadline("./data");
  auto d2 = d1.map(parse_data).map(two_times);

  // saving the resultant dvector
  d2.saveline("./result");

  // checking the saved file content with expected result
  BOOST_CHECK (system("diff ./result ./ref_out") == 0);
}
