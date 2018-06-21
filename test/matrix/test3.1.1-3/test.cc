#include <frovedis.hpp>
#include <frovedis/matrix/sliced_vector.hpp>
#include <frovedis/matrix/sliced_matrix.hpp>
#include <frovedis/matrix/blas_wrapper.hpp>

#define BOOST_TEST_MODULE FrovedisTest
#include <boost/test/unit_test.hpp>

using namespace frovedis;
using namespace std;

BOOST_AUTO_TEST_CASE( frovedis_test )
{
    int argc = 1;
    char** argv = NULL;
    use_frovedis use(argc, argv);

    colmajor_matrix_local<float> cm (
             make_rowmajor_matrix_local_load<float>("./sample_6x6"));

    // creating slice matrix from colmajor matrix (error, invalid range)
    bool error_in_slice_matrix_creation = false;
    sliced_colmajor_matrix_local<float> sm;
    try {
      sm = make_sliced_colmajor_matrix_local<float>(cm,1,1,6,6);
    }
    catch (std::exception& e) {
      error_in_slice_matrix_creation = true;
    }

    // creating slice matrix from colmajor matrix (OK)
    sm = make_sliced_colmajor_matrix_local<float>(cm,1,1,4,4);

    // creating slice vectors from slice matrix
    bool error_in_row_vector_creation = false;
    sliced_colmajor_vector_local<float> sv;
    try {
      sv = make_row_vector(sm,4); // error, invalid row-id
    }
    catch (std::exception& e) {
      error_in_row_vector_creation = true;
    }

    // confirming if above expected exceptions occur
    BOOST_CHECK (error_in_slice_matrix_creation);
    BOOST_CHECK (error_in_row_vector_creation);
}

