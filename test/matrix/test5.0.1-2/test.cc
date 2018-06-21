#include <frovedis.hpp>
#include <frovedis/matrix/sliced_vector.hpp>
#include <frovedis/matrix/pblas_wrapper.hpp>

#define BOOST_TEST_MODULE FrovedisTest
#include <boost/test/unit_test.hpp>

using namespace frovedis;
using namespace std;

BOOST_AUTO_TEST_CASE( frovedis_test )
{
    int argc = 1;
    char** argv = NULL;
    use_frovedis use(argc, argv);

    bool isError = false;
    auto bm = make_blockcyclic_matrix_load<float> ("./sample_4x4");

    // creating slice colmajor vector from entire "4x4 colmajor matrix"
    try {
      sliced_blockcyclic_vector<float> sv (bm); // error, since bm has 4 columns
    }
    catch (std::exception& e) {
      isError = true;
    }

    // confirming whether above expected exception occurs
    BOOST_CHECK (isError);
}

