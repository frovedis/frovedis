#include <frovedis.hpp>
#include <frovedis/ml/glm/ridge_regression_with_lbfgs.hpp>

#define BOOST_TEST_MODULE FrovedisTest
#include <boost/test/unit_test.hpp>
#include "../../rmse.hpp"

using namespace frovedis;
using namespace std;

double to_double(std::string& line) {
  return boost::lexical_cast<double>(line);
}

BOOST_AUTO_TEST_CASE( frovedis_test )
{
    int argc = 1;
    char** argv = NULL;
    use_frovedis use(argc, argv);

    auto data = make_crs_matrix_load<double> ("./data");
    auto label = make_dvector_loadline("./label").map(to_double);

    size_t num_iteration = 100;
    double alpha = 0.00001;
    size_t hist_size = 10;
    double regParam = 0.001;
    bool intercept = true;

    auto model = ridge_regression_with_lbfgs::train(std::move(data),label,
                                   num_iteration, alpha, hist_size,
                                   regParam, intercept);

    auto mat = make_crs_matrix_local_load<double>("./data");    
    auto out = model.predict(mat);
    //for(auto i: out) cout << i << " "; cout << endl;

    double tol= 0.01;
    std::vector<double> expected_out = {2.80754, 4.61481, 1.90378, 
                                        3.7113, 1.72307, 5.51807}; 
    BOOST_CHECK (calc_rms_err<double> (out, expected_out) < tol);
}

