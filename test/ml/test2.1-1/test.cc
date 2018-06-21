#include <frovedis.hpp>
#include <frovedis/ml/glm/logistic_regression_with_sgd.hpp>

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

    size_t num_iteration = 200;
    double alpha = 0.1;
    double minibatch_fraction = 1.0;
    bool intercept = true;
    RegType rt = L2;
    double regParam = 0.01;

    auto model = logistic_regression_with_sgd::train(std::move(data),label,
                                   num_iteration, alpha, minibatch_fraction,
                                   regParam, rt, intercept); 

    auto mat = make_crs_matrix_local_load<double>("./data");    
    auto out_p = model.predict(mat);
    auto out_pb = model.predict_probability(mat);
    //for(auto i: out_p) cout << i << " "; cout << endl;
    //for(auto i: out_pb) cout << i << " "; cout << endl;

    double tol = 0.01;
    std::vector<double> expected_out_p = {1.0, -1.0, 1.0, 1.0};
    std::vector<double> expected_out_pb = {0.952718, 0.437991, 0.958198, 0.955715};
    BOOST_CHECK (calc_rms_err<double> (out_p,expected_out_p) < tol);
    BOOST_CHECK (calc_rms_err<double> (out_pb,expected_out_pb) < tol);
}

