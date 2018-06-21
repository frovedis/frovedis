#include <frovedis.hpp>
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

    // norm of a distributed std::vector
    std::vector<float> x = {0,0,3,4}; 
    auto xbm = vec_to_bcm<float> (x); // distributed vector x
    auto d = nrm2<float> (xbm); 
    BOOST_CHECK (d == 5);

    auto bm = make_blockcyclic_matrix_load<float> ("./sample_4x4");
  
    // checking norm operation  
    auto row1 = make_row_vector<float> (bm,1);
    
    // checking whether the norm operation successfully taken place
    float expected = 6; 
    float res = nrm2<float>(row1);
    BOOST_CHECK (res == expected);
}

