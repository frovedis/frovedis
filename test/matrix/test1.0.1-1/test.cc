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

    // checking the validity of the created blockcyclic matrix
    BOOST_CHECK (bm1.data.map(check).get(0));

    // copy constructor
    blockcyclic_matrix<float> bm2 (bm1);

    // move constructor
    blockcyclic_matrix<float> bm3 (std::move(bm1));

    // checking whether the above constructors worked correctly
    // converting back to the vector form and comparing values
    BOOST_CHECK (bm2.to_vector() == bm3.to_vector());
}

