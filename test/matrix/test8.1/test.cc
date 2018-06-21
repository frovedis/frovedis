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

    // copying two distributed std::vectors'
    std::vector<float> x = {1,2,3,4}; 
    std::vector<float> y = {0,0,0,0};
    auto xbm = vec_to_bcm<float> (x); // distributed vector x
    auto ybm = vec_to_bcm<float> (y); // distrubuted vector y
    copy<float> (xbm,ybm); // ybm = xbm;
    BOOST_CHECK (xbm.moveto_vector() == ybm.moveto_vector());

    auto bm = make_blockcyclic_matrix_load<float> ("./sample_4x4");
  
    // checking copy() operation  
    auto row1 = make_row_vector<float> (bm,1);
    auto row2 = make_row_vector<float> (bm,2);
    copy<float>(row1,row2); // row2 = row1
    bm.save("./out_4x4");

    // checking whether the above operation successfully taken place 
    BOOST_CHECK (system("diff ./out_4x4 ./ref_4x4") == 0);
    system("rm -f ./out_4x4");
}

