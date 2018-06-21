#include <frovedis.hpp>
#include <boost/algorithm/string.hpp>

#define BOOST_TEST_MODULE FrovedisTest
#include <boost/test/unit_test.hpp>

using namespace frovedis;
using namespace std;

std::vector<std::string> splitter(const std::string& line) {
  std::vector<std::string> wordvec;
  boost::split(wordvec, line, boost::is_space());
  return wordvec;
}

std::pair<std::string, int> counter(const std::string& v) {
  return std::make_pair(v, 1);
}

int sum(int a, int b){return a + b;}

BOOST_AUTO_TEST_CASE( frovedis_test )
{
  int argc = 1;
  char** argv = NULL;
  use_frovedis use(argc, argv);

  // testing for string counter
  auto d1 = frovedis::make_dvector_loadline("./sample.txt");
  auto d2 = d1.flat_map(splitter).map(counter).reduce_by_key<std::string,int>(sum);
  auto r = d2.as_dvector().sort().gather();

  // filling expected output vector
  std::vector<std::pair<std::string,int>> ref_out;
  ref_out.push_back(std::make_pair("Is", 2));
  ref_out.push_back(std::make_pair("Yes", 2));
  ref_out.push_back(std::make_pair("a", 3));
  ref_out.push_back(std::make_pair("is", 1));
  ref_out.push_back(std::make_pair("is!", 1));
  ref_out.push_back(std::make_pair("it", 1));
  ref_out.push_back(std::make_pair("really", 1));
  ref_out.push_back(std::make_pair("test.", 1));
  ref_out.push_back(std::make_pair("test?", 2));
  ref_out.push_back(std::make_pair("this", 3));

  BOOST_CHECK(r == ref_out);
  //for(auto& i: r) std::cout << i.first << ": " << i.second << std::endl;
}
