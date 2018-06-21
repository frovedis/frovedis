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

    // dot product of two distributed std::vectors'
    std::vector<float> x = {1,2,3,4}; 
    std::vector<float> y = {1,2,3,4};
    auto xbm = vec_to_bcm<float> (x); // distributed vector x
    auto ybm = vec_to_bcm<float> (y); // distrubuted vector y
    auto d = dot<float> (xbm,ybm); // d = xbm.ybm
    BOOST_CHECK (d == 30);

    auto bm = make_blockcyclic_matrix_load<float> ("./sample_4x4");
  
    // checking dot() operation  
    auto row1 = make_row_vector<float> (bm,1);
    auto row2 = make_row_vector<float> (bm,2);
    
    // checking whether the dot operation successfully taken place 
    auto r = dot<float>(row1, row2); // r = row1 . row2
    BOOST_CHECK (r == 2);
}

