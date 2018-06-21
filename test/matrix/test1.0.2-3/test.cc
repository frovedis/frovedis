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

    // constructing blockcylic matrix from colmajor matrix
    colmajor_matrix<float> cm (
             make_rowmajor_matrix_load<float>("./sample_4x4"));
    blockcyclic_matrix<float> bm1 (cm); // grid-2

    // checking the validity of the created blockcyclic matrix
    BOOST_CHECK (bm1.data.map(check).get(0));

    // converting back to colmajor matrix
    bool isError = false;
    try {
      bm1.to_colmajor(); // okay
      bm1.moveto_colmajor(); // error, since bm1 is a grid-2 matrix
    }
    catch (std::exception& e) {
      isError = true;
    }

    // checking whether the expected error occurs
    BOOST_CHECK (isError);
}

