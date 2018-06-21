#include <frovedis.hpp>
#include <boost/lexical_cast.hpp>

#define BOOST_TEST_MODULE FrovedisTest
#include <boost/test/unit_test.hpp>

using namespace frovedis;
using namespace std;
using namespace boost;

int two_times(int i) {return i*2;}

BOOST_AUTO_TEST_CASE( frovedis_test )
{
  int argc = 1;
  char** argv = NULL;
  use_frovedis use(argc, argv);
  
  auto d1 = make_dvector_loadbinary<int>("./data_binary");
  auto d2 = d1.map(two_times);
  d2.savebinary("./result_binary");

  // testing for loadbinary (whether the vector loaded correctly)
  auto ivec = d1.gather();

  // testing for savebinary (whether the vector saved correctly)
  auto ovec = make_dvector_loadbinary<int>("./result_binary").gather();

  // creating reference output for cross verification
  std::vector<int> ref_out;
  for(auto e: ivec) ref_out.push_back(two_times(e));
  BOOST_CHECK(ref_out == ovec);
}
