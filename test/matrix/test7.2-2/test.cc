#include <frovedis.hpp>
#include <frovedis/matrix/blas_wrapper.hpp>
#include <frovedis/matrix/lapack_wrapper.hpp>


#define BOOST_TEST_MODULE FrovedisTest
#include <boost/test/unit_test.hpp>

using namespace frovedis;
using namespace std;

BOOST_AUTO_TEST_CASE( frovedis_test )
{
    int argc = 1;
    char** argv = NULL;
    use_frovedis use(argc, argv);

    // creating a colmajor matrix local from file
    colmajor_matrix_local<float> A (
           make_rowmajor_matrix_local_load<float>("./sample_3x3"));

    colmajor_matrix_local<float> B (
           make_rowmajor_matrix_local_load<float>("./sample_2x1"));

    // creating sliced matrix
    auto sm = make_sliced_colmajor_matrix_local<float> (A,1,1,2,2);

    std::vector<int> ipiv;   // empty ipiv array
    getrf<float> (sm,ipiv);   // A will be factorized and ipiv will contain pivoting info
    getrs<float> (sm,B,ipiv); // solving AX=B, B will be overwritten with result matrix X

    // checking whether the above operations successfully taken place 
    B.to_rowmajor().save("./out_2x1");
    BOOST_CHECK (system("diff ./out_2x1 ./ref_2x1") == 0);
    system("rm -f ./out_2x1");
}

