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

    // dot product of two std::vectors'
    std::vector<float> x = {1,2,3,4};
    std::vector<float> y = {1,2,3,4};
    auto d = dot<float> (x,y); // d = x.y
    BOOST_CHECK (d == 30);

    // creating a colmajor matrix local from file
    colmajor_matrix_local<float> cm (
           make_rowmajor_matrix_local_load<float>("./sample_4x4"));
  
    // checking dot() operation  
    auto row1 = make_row_vector<float> (cm,1);
    auto row2 = make_row_vector<float> (cm,2);
    
    // checking whether the dot operation successfully taken place 
    auto r = dot<float>(row1, row2); // r = row1 . row2
    BOOST_CHECK (r == 2);
}

