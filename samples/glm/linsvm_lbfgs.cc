#include <frovedis.hpp>
#include <frovedis/ml/glm/svm_with_lbfgs.hpp>

#include <boost/program_options.hpp>

using namespace boost;
using namespace frovedis;
using namespace std;

double to_double(std::string& line) {
  return boost::lexical_cast<double>(line);
}

void do_train(const string& input, const string& label, const string& output,
              size_t num_iteration, double alpha, size_t hist_size,
              const string& regularizer, double regParam, bool intercept,
              double convTol, MatType mType, bool binary) {

  RegType rt;
  if(regularizer == "ZERO") {
    rt = ZERO;
  } else if (regularizer == "L1") {
    rt = L1;
  } else if (regularizer == "L2") {
    rt = L2;
  } else {
    cerr << "invalid regularization type: " << regularizer << endl;
    exit(1);
  }
  if(binary) {
    time_spent t(DEBUG);
    auto mat = make_crs_matrix_loadbinary<double>(input);
    t.show("load matrix: ");
    auto lb = make_dvector_loadbinary<double>(label);
    t.show("load label: ");
    auto lm = svm_with_lbfgs::train(mat, lb, num_iteration, alpha,
                                    hist_size, regParam,
                                    rt, intercept, convTol, mType);
    t.show("train time: ");
    lm.savebinary(output);
    t.show("save model time: ");
  } else {
    time_spent t(DEBUG);
    auto mat = make_crs_matrix_load<double>(input);
    t.show("load matrix: ");
    auto lb = make_dvector_loadline(label).map(to_double);
    t.show("load label: ");
    auto lm = svm_with_lbfgs::train(mat, lb, num_iteration, alpha,
                                    hist_size, regParam,
                                    rt, intercept, convTol, mType);
    t.show("train time: ");
    lm.save(output);
    t.show("save model time: ");
  }
}

void do_predict(const string& input, const string& model, const string& output,
                bool prob, bool binary) {
  svm_model<double> lm;
  if(binary) {
    lm.loadbinary(model);
    auto mat = make_crs_matrix_local_loadbinary<double>(input);
    if(prob) {
      auto r = lm.predict_probability(mat);
      make_dvector_scatter(r).savebinary(output);
    } else {
      auto r = lm.predict(mat);
      make_dvector_scatter(r).savebinary(output);
    }
  } else {
    lm.load(model);
    auto mat = make_crs_matrix_local_load<double>(input);
    if(prob) {
      auto r = lm.predict_probability(mat);
      make_dvector_scatter(r).saveline(output);
    } else {
      auto r = lm.predict(mat);
      make_dvector_scatter(r).saveline(output);
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
    ("predict-probability,y", "probability-predict mode")
    ("input,i", value<string>(), "input matrix")
    ("ell", "assume ell storage of input training data matrix")
    ("crs", "assume crs storage of input training data matrix (default for X86)")
    ("hybrid", "assume jds-crs hybrid storage of input training data matrix (default for SX)")
    ("label,l", value<string>(), "input label (for train)")
    ("model,m", value<string>(), "input model (for predict)")
    ("output,o", value<string>(), "output model or predict result")
    ("num-iteration,n", value<size_t>(), "number of iteration (default: 1000)")
    ("alpha,a", value<double>(), "learning rate (default: 0.01)")
    ("history-size,h", value<size_t>(), "size for lbfgs history vectors (default: 10)")
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
  size_t num_iteration = 1000, hist_size = 10;
  double alpha = 0.01;
  string regularizer = "ZERO";
  double regParam = 0.01;
  double convTol = 0.001;
  bool intercept = false;
  bool binary = false;
  MatType mType = CRS;
  
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

  if(argmap.count("history-size")){
    hist_size = argmap["history-size"].as<size_t>();
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

  if(ispredict) do_predict(input, model, output, predict_probability, binary);
  else do_train(input, label, output, num_iteration, alpha, hist_size,
                regularizer, regParam, intercept, convTol, mType, binary);
}
