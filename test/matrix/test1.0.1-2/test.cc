#include <frovedis.hpp>
#include <frovedis/matrix/blockcyclic_matrix.hpp>

#define BOOST_TEST_MODULE FrovedisTest
#include <boost/test/unit_test.hpp>

using namespace frovedis;
using namespace std;

bool check(blockcyclic_matrix_local<float>& m) {
  return m.is_valid();
}

BOOST_AUTO_TEST_CASE( frovedis_test )
{
    int argc = 1;
    char** argv = NULL;
    use_frovedis use(argc, argv);

    std::vector<float> vec = {1, 2, 3, 4};

    // constructing a blockcyclic matrix from a vector
    blockcyclic_matrix<float> bm1 (vec);

    // converting to vector
    // even after the successful conversion, the matrix will remain valid 
    bm1.to_vector();
    BOOST_CHECK (bm1.data.map(check).get(0) == 1);

    // moving the contents while converting to vector
    // after the successful move operation, matrix will no longer valid 
    bm1.moveto_vector();
    BOOST_CHECK (bm1.data.map(check).get(0) == 0);
}

