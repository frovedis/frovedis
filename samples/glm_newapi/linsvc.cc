#include <frovedis.hpp>
#include <frovedis/ml/glm/linear_svc.hpp>
#include <boost/program_options.hpp>

using namespace boost;
using namespace frovedis;
using namespace std;

template <class T>
void do_train(const string& input, 
              const string& label, 
              const string& output,
              const string& solver,
              size_t num_iteration, 
              double alpha, 
              double minibatch_fraction,
              size_t hist_size,
              const string& regularizer, 
              double regParam, bool intercept,
              double convTol, MatType mType, bool binary) {

  time_spent t(DEBUG);
  crs_matrix<T> mat;
  dvector<T> lbl;

  if(binary) {
    mat = make_crs_matrix_loadbinary<T>(input);
    t.show("load matrix: ");
    lbl = make_dvector_loadbinary<T>(label);
    t.show("load label: ");
  } else {
    mat = make_crs_matrix_load<T>(input);
    t.show("load matrix: ");
    lbl = make_dvector_loadline<T>(label);
    t.show("load label: ");
  }

  auto lm = linear_svm_classifier<T>().set_solver(solver)
                                      .set_max_iter(num_iteration)
                                      .set_alpha(alpha)
                                      .set_mini_batch_fraction(minibatch_fraction)
                                      .set_hist_size(hist_size)
                                      .set_reg_param(regParam)
                                      .set_reg_type(regularizer)
                                      .set_intercept(intercept)
                                      .set_tol(convTol);
  lm.fit(mat, lbl);
  t.show("train time: "); 

  if(binary) lm.model.savebinary(output);
  else       lm.model.save(output);
  t.show("save model time: ");
}

template <class T>
void do_predict(const string& input,
                const string& model,
                const string& output,
                bool prob, bool binary) {
  time_spent t(DEBUG);
  crs_matrix_local<T> mat;
  svm_model<T> lm;
  if(binary) {
    lm.loadbinary(model);
    t.show("load model: ");
    mat = make_crs_matrix_local_loadbinary<T>(input);
    t.show("load matrix: ");
  } else {
    lm.load(model);
    t.show("load model: ");
    mat = make_crs_matrix_local_load<T>(input);
    t.show("load matrix: ");
  }
  auto r = prob ? lm.predict_probability(mat) : lm.predict(mat);
  t.show("prediction time: ");
  if(binary) make_dvector_scatter(r).savebinary(output);
  else       make_dvector_scatter(r).saveline(output);
  t.show("save predicted result: ");
}

int main(int argc, char* argv[]) {
  use_frovedis use(argc, argv);

  using namespace boost::program_options;

  options_description opt("option");
  opt.add_options()
    ("help,h", "print help")
    ("predict,p", "predict mode")
    ("predict-probability,y", "probability-predict mode")
    ("input,i", value<string>(), "input sparse data matrix")
    ("ell", "assume ell storage of input training data matrix")
    ("crs", "assume crs storage of input training data matrix (default for X86)")
    ("hybrid", "assume jds-crs hybrid storage of input training data matrix (default for SX)")
    ("label,l", value<string>(), "input label (for train)")
    ("model,m", value<string>(), "input model (for predict)")
    ("output,o", value<string>(), "output model or predict result")
    ("dtype", value<string>(), "target data type for input (double or float) (default: double)")
    ("solver,s", value<string>(), "linear svm regression solver: sgd (only)")
    ("num-iteration,n", value<size_t>(), "number of iteration (default: 1000)")
    ("alpha,a", value<double>(), "learning rate (default: 0.01)")
    ("minibatch-fraction,f", value<double>(), "fraction rate for minibatch (default: 1.0)")
    ("hist-size,h", value<size_t>(), "size of history vectors - applicable only for lbfgs solver (default: 10)")
    ("regularizer,r", value<string>(),
     "regularizer (ZERO (default), L1, or L2)")
    ("regularization-parameter,e", value<double>(), "regularization parameter (default: 0.01)")
    ("convergence-tolerance,c", value<double>(), "a tolerance value to determine convergence (default: 0.001)")
    ("intercept,t", "use bias or not")
    ("verbose", "set loglevel to DEBUG")
    ("verbose2", "set loglevel to TRACE")
    ("binary,b", "use binary input/output");

  variables_map argmap;
  store(command_line_parser(argc,argv).options(opt).allow_unregistered().
        run(), argmap);
  notify(argmap);

  bool ispredict = false;
  bool predict_probability = false;
  string input, label, model, output;
  size_t num_iteration = 1000;
  double alpha = 0.01;
  double minibatch_fraction = 1.0;
  size_t hist_size = 10;
  string regularizer = "ZERO";
  double regParam = 0.01;
  double convTol = 0.001;
  bool intercept = false;
  bool binary = false;
#if defined(_SX) || defined(__ve__)
  MatType mType = HYBRID;
#else
  MatType mType = CRS;
#endif
  string solver = "sgd";
  string dtype = "double";
  
  if(argmap.count("help")){
    cerr << opt << endl;
    exit(1);
  }

  if(argmap.count("predict")){
    ispredict = true;
  }

  if(argmap.count("predict-probability")){
    ispredict = true;
    predict_probability = true;
  }

  if(argmap.count("input")){
    input = argmap["input"].as<string>();
  } else {
    cerr << "input is not specified" << endl;
    cerr << opt << endl;
    exit(1);
  }

  if(!ispredict) {
    if(argmap.count("label")){
      label = argmap["label"].as<string>();
    } else {
      cerr << "label file is not specified" << endl;
      cerr << opt << endl;
      exit(1);
    }

    if(argmap.count("ell")) {
      mType = ELL;
    } else if(argmap.count("crs")) {
      mType = CRS;
    } else if(argmap.count("hybrid")) {
      mType = HYBRID;
    }
  }

  if(ispredict) {
    if(argmap.count("model")){
      model = argmap["model"].as<string>();
    } else {
      cerr << "model file is not specified" << endl;
      cerr << opt << endl;
      exit(1);
    }
  }

  if(argmap.count("output")){
    output = argmap["output"].as<string>();
  } else {
    cerr << "output is not specified" << endl;
    cerr << opt << endl;
    exit(1);
  }

  if(argmap.count("num-iteration")){
    num_iteration = argmap["num-iteration"].as<size_t>();
  }

  if(argmap.count("alpha")){
    alpha = argmap["alpha"].as<double>();
  }

  if(argmap.count("minibatch-fraction")){
    minibatch_fraction = argmap["minibatch-fraction"].as<double>();
  }

  if(argmap.count("regularizer")){
    regularizer = argmap["regularizer"].as<string>();
  }

  if(argmap.count("convergence-tolerance")){
    convTol = argmap["convergence-tolerance"].as<double>();
  }

  if(argmap.count("regularization-parameter")){
    regParam = argmap["regularization-parameter"].as<double>();
  }

  if(argmap.count("intercept")){
    intercept = true;
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

  if(argmap.count("solver")){
    solver = argmap["solver"].as<string>();
  } 

  if(argmap.count("dtype")){
    dtype = argmap["dtype"].as<string>();
  } 

  if(argmap.count("hist-size")){
    hist_size = argmap["hist-size"].as<size_t>();
  }

  if (dtype == "float") {
    if(ispredict) do_predict<float>(input, model, output, 
                                    predict_probability, binary);
    else do_train<float>(input, label, output, solver, 
                         num_iteration, alpha, minibatch_fraction, hist_size, 
                         regularizer, regParam, intercept, convTol, 
                         mType, binary);
  }
  else if (dtype == "double") {
    if(ispredict) do_predict<double>(input, model, output, 
                                     predict_probability, binary);
    else do_train<double>(input, label, output, solver, 
                          num_iteration, alpha, minibatch_fraction, hist_size,
                          regularizer, regParam, intercept, convTol, 
                          mType, binary);
  }
  else REPORT_ERROR(USER_ERROR, "supported dtype is either float or double!\n");
}
