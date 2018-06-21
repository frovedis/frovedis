#include <frovedis.hpp>
#include <frovedis/matrix/pblas_wrapper.hpp>
#include <frovedis/matrix/scalapack_wrapper.hpp>


#define BOOST_TEST_MODULE FrovedisTest
#include <boost/test/unit_test.hpp>

using namespace frovedis;
using namespace std;

BOOST_AUTO_TEST_CASE( frovedis_test )
{
    int argc = 1;
    char** argv = NULL;
    use_frovedis use(argc, argv);

    // creating blockcyclic matrix from file
    auto A = make_blockcyclic_matrix_load<float> ("./sample_2x2");
    auto B = make_blockcyclic_matrix_load<float> ("./sample_2x1");

    frovedis::lvec<int> ipiv;  // empty ipiv local-array
    getrf<float> (A,ipiv);   // A will be factorized and ipiv will contain pivoting info
    getrs<float> (A,B,ipiv); // solving AX=B, B will be overwritten with result matrix X

    // checking whether the above operations successfully taken place 
    B.save("./out_2x1");
    BOOST_CHECK (system("diff ./out_2x1 ./ref_2x1") == 0);
    system("rm -f ./out_2x1");
}

