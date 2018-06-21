#include <frovedis.hpp>
#include <frovedis/ml/glm/lasso_with_sgd.hpp>

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
    double minibatch_fraction = 1.0;
    double regParam = 0.001;
    bool intercept = true;

    auto model = lasso_with_sgd::train(std::move(data),label,
                                   num_iteration, alpha, minibatch_fraction,
                                   regParam, intercept);

    auto mat = make_crs_matrix_local_load<double>("./data");    
    auto out = model.predict(mat);
    //for(auto i: out) cout << i << " "; cout << endl;

    double tol= 0.01;
    std::vector<double> expected_out = {2.73269, 4.46604, 1.86638, 
                                        3.599, 1.69297, 5.33379};
    BOOST_CHECK (calc_rms_err<double> (out, expected_out) < tol);
}

