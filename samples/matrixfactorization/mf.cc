#include <frovedis.hpp>
#include <frovedis/ml/recommendation/als.hpp>

#include <boost/program_options.hpp>
#include <stdlib.h>

using namespace boost;
using namespace frovedis;
using namespace std;

// to let dvector<T>::saveline know the operator...
namespace frovedis {
ostream& operator<<(ostream& os, const pair<size_t, double>& d) {
  return os << d.first << ", " << d.second;
}
}

void do_train(const string& input, const string& output, int num_iteration,
              int factor, double alpha, double regParam, int seed, 
              bool binary, double sf) {
  if(seed == -1) seed = abs(rand());
  crs_matrix<double> data;
  time_spent t(INFO);
  if(binary) data = make_crs_matrix_loadbinary<double>(input);
  else data = make_crs_matrix_load<double>(input);
  t.show("data load time: ");
  auto mfm = matrix_factorization_using_als::train(data, factor, num_iteration,
                                                   alpha, regParam, seed, sf);
  t.show("train time: ");
  if(binary) mfm.savebinary(output);
  else mfm.save(output);
  t.show("model save time: ");
}

void do_predict(const string& input, const string& output,
                int user_for_recommend, int product_for_recommend,
                int num_recommend, bool binary) {
  matrix_factorization_model<double> mfm;
  if(binary) mfm.loadbinary(input);
  else mfm.load(input);
  if(user_for_recommend >= 0 && product_for_recommend >= 0) {
    double r = mfm.predict(user_for_recommend, product_for_recommend);
    cout << r << endl;
  } else {
    if(user_for_recommend >= 0) {
      auto r = mfm.recommend_products(user_for_recommend, num_recommend);
      make_dvector_scatter(r).saveline(output);
    } else if(product_for_recommend >= 0) {
      auto r = mfm.recommend_users(product_for_recommend, num_recommend);
      make_dvector_scatter(r).saveline(output);
    } else {
      cerr << "either user or product is not specified for predict" << endl;
      exit(1);
    }
  }
}

int main(int argc, char* argv[]) {
  use_frovedis use(argc, argv);

  using namespace boost::program_options;

  options_description opt("option");
  opt.add_options()
    ("help,h", "print help")
    ("predict,p", "predict mode")
    ("input,i", value<string>(), "input matrix / model")
    ("output,o", value<string>(), "output model or predict result (if both product and user is specified for prediction, output to stdout)")
    ("factor,f", value<int>(), "number of factor")
    ("num-iteration,n", value<int>(), "number of iteration")
    ("alpha,a", value<double>(), "learning rate")
    ("regularization-parameter,e", value<double>(), "regularization parameter")
    ("seed,s", value<int>(), "random seed (default 0; if -1, random)")
    ("similarity-factor", value<double>(), "similarity factor (default 1.0)")
    ("recommend-products,r", value<int>(),
     "recommend products for specified user")
    ("recommend-users,u", value<int>(),
     "recommend users for specified product")
    ("num-recommend,k", value<int>(), "number of products/users for recommend")
    ("verbose", "set loglevel to DEBUG")
    ("verbose2", "set loglevel to TRACE")
    ("binary,b", "use binary input (output is stdout or text)");

  variables_map argmap;
  store(command_line_parser(argc,argv).options(opt).allow_unregistered().
        run(), argmap);
  notify(argmap);

  bool ispredict = false;
  string input, output;
  int num_iteration = 100;
  int factor = 100;
  double alpha = 0.01;
  double regParam = 0.01;
  int seed = 0;
  int product_for_recommend = -1;
  int user_for_recommend = -1;
  int num_recommend = 10;
  bool binary = false;
  double sf = 0.1;
  
  if(argmap.count("help")){
    cerr << opt << endl;
    exit(1);
  }

  if(argmap.count("predict")){
    ispredict = true;
  }

  if(argmap.count("input")){
    input = argmap["input"].as<string>();
  } else {
    cerr << "input is not specified" << endl;
    cerr << opt << endl;
    exit(1);
  }

  if(argmap.count("factor")){
    factor = argmap["factor"].as<int>();
  } else {
    if(!ispredict) {
      cerr << "factor is not specified" << endl;
      cerr << opt << endl;
      exit(1);
    }
  }

  if(argmap.count("num-iteration")){
    num_iteration = argmap["num-iteration"].as<int>();
  }

  if(argmap.count("alpha")){
    alpha = argmap["alpha"].as<double>();
  }

  if(argmap.count("regularization-parameter")){
    regParam = argmap["regularization-parameter"].as<double>();
  }

  if(argmap.count("similarity-factor")){
    sf = argmap["similarity-factor"].as<double>();
  }

  if(argmap.count("seed")){
    seed = argmap["seed"].as<int>();
  }

  // if both recommend-products and recommend-users are specified,
  // predict is called.
  // otherwise, predictProducts or predictUsers are called for prediction
  if(argmap.count("recommend-products")){
    user_for_recommend = argmap["recommend-products"].as<int>();
  }

  if(argmap.count("recommend-users")){
    product_for_recommend = argmap["recommend-users"].as<int>();
  }

  if(argmap.count("output")){
    output = argmap["output"].as<string>();
  } else {
    if(!(ispredict && user_for_recommend >= 0 && product_for_recommend >=0)) {
      cerr << "output is not specified" << endl;
      cerr << opt << endl;
      exit(1);
    }
  }

  if(argmap.count("num-recommend")){
    num_recommend = argmap["num-recommend"].as<int>();
  }

  if(argmap.count("binary")){
    binary = true;
  }

  if(argmap.count("verbose")){
    set_loglevel(DEBUG);
  }

  if(argmap.count("verbose2")){
    set_loglevel(TRACE);
  }

  if(ispredict) {
    do_predict(input, output, user_for_recommend, product_for_recommend,
               num_recommend, binary);
  } else {
    do_train(input, output, num_iteration, factor, alpha, 
             regParam, seed, binary, sf);
  }
}
