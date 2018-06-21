#include <frovedis.hpp>

#define BOOST_TEST_MODULE FrovedisTest
#include <boost/test/unit_test.hpp>

using namespace frovedis;
using namespace std;

void mpi_func(std::vector<int>& v) {
  int a;
  if(get_selfid() == 0) {a = 2;}
  // native mpi routine
  MPI_Bcast(&a, 1, MPI_INT, 0, MPI_COMM_WORLD);
  for(size_t i = 0; i < v.size(); i++) v[i] *= a;
}

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
  d1.mapv_partitions(mpi_func);
  auto r = d1.gather();
  //for(auto i: r) std::cout << i << std::endl;
  BOOST_CHECK(r == ref);
}
