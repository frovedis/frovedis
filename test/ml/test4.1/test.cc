#include <frovedis.hpp>
#include <frovedis/ml/clustering/kmeans.hpp>

#define BOOST_TEST_MODULE FrovedisTest
#include <boost/test/unit_test.hpp>

using namespace frovedis;
using namespace std;

BOOST_AUTO_TEST_CASE( frovedis_test )
{
    int argc = 1;
    char** argv = NULL;
    use_frovedis use(argc, argv);

    // loading the input data matrix
    auto data = make_crs_matrix_load<double> ("./data");

    // setting the algorithm input parameters
    size_t num_iteration = 100;
    size_t k = 2;
    double eps = 0.01;

    // getting the trained model
    auto model = kmeans(data, k, num_iteration, eps); 

    // assigning
    auto mat = make_crs_matrix_local_load<double>("./data");   
    auto res = kmeans_assign_cluster(mat, model); 
    //for(auto &i: r) cout << i << " "; cout << endl;

    // confirming correctness of result
    std::vector<int> expected = {0, 1, 1, 0, 0, 1};
    BOOST_CHECK (res == expected);
}

