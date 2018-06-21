#include <frovedis.hpp>
#include <frovedis/ml/recommendation/als.hpp>

#define BOOST_TEST_MODULE FrovedisTest
#include <boost/test/unit_test.hpp>

using namespace frovedis;
using namespace std;

bool isError (std::vector<std::pair<size_t,double>>& v1,
              std::vector<std::pair<size_t,double>>& v2) {
  double tol = 0.01;
  double sum = 0.0;
  if (v1.size() != v2.size()) return false;
  for(size_t i=0; i<v1.size(); ++i) {
    if (v1[i].first != v2[i].first) return false;
    double diff = (v1[i].second - v2[i].second);
    sum += (diff * diff);
  }
  double r = sqrt(sum/v1.size());
  //cout << "RMSE: " << r << endl;
  return (r > tol);
}

BOOST_AUTO_TEST_CASE( frovedis_test )
{
    int argc = 1;
    char** argv = NULL;
    use_frovedis use(argc, argv);

    // loading the input rank matrix
    auto data = make_crs_matrix_load<double> ("./data");

    // setting the algorithm parameter
    size_t num_iteration = 100;
    size_t factor = 2;
    double alpha = 0.01;
    double regParam = 0.01;
    long seed = 0;

    // getting the trained model
    auto model = matrix_factorization_using_als::train(data, factor,
                                   num_iteration, alpha, 
                                   regParam, seed); 

    // predicting rating by a particular user for a particular product
    double r1 = model.predict(0,1);
    double r2 = model.predict(3,3);
    double e_r1 = 0.25;
    double e_r2 = 0.25;
    double tol = 0.01;
    BOOST_CHECK ((r1 - e_r1) < tol);
    BOOST_CHECK ((r2 - e_r2) < tol);

    // recommending top 3 users for product id 1
    auto ru = model.recommend_users(1,3);
    //for(auto &i: ru) cout << i.first << " " << i.second << endl; 
    std::vector<std::pair<size_t,double>> e_out;
    e_out.push_back(make_pair<size_t,double>(2,1.18));
    e_out.push_back(make_pair<size_t,double>(1,0.97));
    e_out.push_back(make_pair<size_t,double>(3,0.56));
    BOOST_CHECK (isError(e_out,ru) == false);

    // recommending top 3 products for user id 1
    auto rp = model.recommend_products(1,3);
    //for(auto &i: rp) cout << i.first << " " << i.second << endl; 
    e_out.clear();
    e_out.push_back(make_pair<size_t,double>(1,0.97));
    e_out.push_back(make_pair<size_t,double>(3,0.93));
    e_out.push_back(make_pair<size_t,double>(0,0.25));
    BOOST_CHECK (isError(e_out,rp) == false);
}
