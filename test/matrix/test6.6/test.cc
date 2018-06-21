#include <frovedis.hpp>
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

    // norm of a std::vector
    std::vector<float> x = {0,0,3,4};
    auto d = nrm2<float> (x);
    BOOST_CHECK (d == 5);

    // creating a colmajor matrix local from file
    colmajor_matrix_local<float> cm (
           make_rowmajor_matrix_local_load<float>("./sample_4x4"));
  
    // checking norm() operation  
    auto row1 = make_row_vector<float> (cm,1);
    
    // checking whether the norm operation successfully taken place
    float expected = 6; 
    float res = nrm2<float>(row1);
    BOOST_CHECK (res == expected);
}

