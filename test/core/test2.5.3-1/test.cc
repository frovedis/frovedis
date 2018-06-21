#include <frovedis.hpp>

#define BOOST_TEST_MODULE FrovedisTest
#include <boost/test/unit_test.hpp>

using namespace frovedis;
using namespace std;

int sum_vector(int key, std::vector<int>& v) {
  int sum = 0;
  for(size_t i = 0; i < v.size(); i++) sum += v[i];
  return sum;
}

BOOST_AUTO_TEST_CASE( frovedis_test )
{
  int argc = 1;
  char** argv = NULL;
  use_frovedis use(argc, argv);

  // filling sample input vector
  std::vector<std::pair<int,int>> v;
  v.push_back(std::make_pair(1,10));
  v.push_back(std::make_pair(4,10));
  v.push_back(std::make_pair(3,10));
  v.push_back(std::make_pair(2,10));
  v.push_back(std::make_pair(3,20));
  v.push_back(std::make_pair(2,20));
  v.push_back(std::make_pair(4,20));
  v.push_back(std::make_pair(1,20));

  // filling sample output vector
  std::vector<std::pair<int,int>> ref_out;
  ref_out.push_back(std::make_pair(1,30));
  ref_out.push_back(std::make_pair(2,30));
  ref_out.push_back(std::make_pair(3,30));
  ref_out.push_back(std::make_pair(4,30));

  auto d1 = frovedis::make_dvector_scatter(v);
  auto d2 = d1.group_by_key<int,int>().map_values(sum_vector);
  auto r = d2.as_dvector().sort().gather();
  //for(auto i: r) std::cout << i.first << ": " << i.second << std::endl;

  BOOST_CHECK(r == ref_out);
}
