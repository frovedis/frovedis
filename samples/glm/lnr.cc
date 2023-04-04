#include <frovedis.hpp>
#include <frovedis/ml/glm/linear_regression_with_sgd.hpp>
#include <frovedis/ml/glm/linear_regression_with_lbfgs.hpp>
#include <frovedis/ml/glm/simple_linear_regression.hpp>
#include <boost/program_options.hpp>

using namespace boost;
using namespace frovedis;
using namespace std;

template <class MATRIX, class T>
linear_regression_model<T>
do_train_impl(MATRIX& mat, dvector<T>& lbl,
              const string& solver,
              size_t num_iteration, 
              double alpha, 
              double minibatch_fraction,
              size_t hist_size,
              bool intercept,
              double convTol, MatType mType) {
  linear_regression_model<T> lm;
  if(solver == "sgd") {
    lm = linear_regression_with_sgd::train(std::move(mat), lbl, num_iteration, 
                                           alpha, minibatch_fraction,
                                           intercept, convTol, mType);
  }
  else if (solver == "lbfgs") {
    lm = linear_regression_with_lbfgs::train(std::move(mat), lbl, num_iteration, 
                                             alpha, hist_size,
                                             intercept, convTol, mType);
  }
  else if (solver == "lapack") {
    int rank = 0;
    std::vector<T> sval, sample_weight;
    lm = linear_regression_with_lapack(mat, lbl, rank, sval,
                                       sample_weight,
                                       intercept);
  }
  else if (solver == "scalapack") {
    std::vector<T> sample_weight;
    lm = linear_regression_with_scalapack(mat, lbl,
                                          sample_weight,
                                          intercept);
  }
  else if (solver == "sparse_lsqr") {
    size_t n_iter;
    std::vector<T> sample_weight;
    lm = linear_regression_with_lsqr_impl(mat, lbl,
                                          sample_weight, num_iteration,
                                          intercept, n_iter);
  }
  else REPORT_ERROR(USER_ERROR, "supported solver is either sgd or lbfgs!\n");
  return lm;
}

template <class T>
void do_train(const string& input, 
              const string& label, 
              const string& output,
              const string& solver,
              size_t num_iteration, 
              double alpha, 
              double minibatch_fraction,
              size_t hist_size,
              bool intercept,
              double convTol, MatType mType, 
              bool binary, bool isdense) {

  time_spent t(DEBUG);
  dvector<T> lbl;
  linear_regression_model<T> lm;

  if(isdense) {
    colmajor_matrix<T> mat;
    if(binary) {
      mat = colmajor_matrix<T>(make_rowmajor_matrix_loadbinary<T>(input));
      t.show("load matrix: ");
      lbl = make_dvector_loadbinary<T>(label);
      t.show("load label: ");
    } else {
      mat = colmajor_matrix<T>(make_rowmajor_matrix_load<T>(input));
      t.show("load matrix: ");
      lbl = make_dvector_loadline<T>(label);
      t.show("load label: ");
    }
    lm = do_train_impl<colmajor_matrix<T>, T> (
                       mat, lbl, solver, num_iteration, alpha,
                       minibatch_fraction, hist_size,
                       intercept, convTol, mType);
  } else {
    crs_matrix<T> mat;
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
    lm = do_train_impl<crs_matrix<T>, T> (
                       mat, lbl, solver, num_iteration, alpha,
                       minibatch_fraction, hist_size,
                       intercept, convTol, mType);
  }
  t.show("train time: ");

  if(binary) lm.savebinary(output);
  else       lm.save(output);
  t.show("save model time: ");
}

template <class T>
void do_predict(const string& input, 
                const string& model, 
                const string& output,
                bool binary, bool isdense) {
  time_spent t(DEBUG);
  linear_regression_model<T> lm;
  std::vector<T> r;

  if(isdense) {
    rowmajor_matrix_local<T> mat;
    if(binary) {
      lm.loadbinary(model);
      t.show("load model: ");
      mat = make_rowmajor_matrix_local_loadbinary<T>(input);
      t.show("load matrix: ");
    } else {
      lm.load(model);
      t.show("load model: ");
      mat = make_rowmajor_matrix_local_load<T>(input);
      t.show("load matrix: ");
    }
    r = lm.predict(mat);
  } else {
    crs_matrix_local<T> mat;
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
    r = lm.predict(mat);
  }
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
    ("input,i", value<string>(), "input sparse data matrix")
    ("ell", "assume ell storage of input training data matrix")
    ("crs", "assume crs storage of input training data matrix (default for X86)")
    ("hybrid", "assume jds-crs hybrid storage of input training data matrix (default for SX)")
    ("label,l", value<string>(), "input label (for train)")
    ("model,m", value<string>(), "input model (for predict)")
    ("output,o", value<string>(), "output model or predict result")
    ("dtype", value<string>(), "target data type for input (double or float) (default: double)")
    ("solver,s", value<string>(), "linear regression solver: lapack (default for dense), sparse_lsqr (default for sparse), scalapack, sgd, lbfgs")
    ("num-iteration,n", value<size_t>(), "number of iteration (default: 1000)")
    ("alpha,a", value<double>(), "learning rate (default: 0.01)")
    ("minibatch-fraction,f", value<double>(), "fraction rate for minibatch (default: 1.0)")
    ("hist-size,h", value<size_t>(), "size of history vectors - applicable only for lbfgs solver (default: 10)")
    ("convergence-tolerance,c", value<double>(), "a tolerance value to determine convergence (default: 0.001)")
    ("intercept,t", "use bias or not")
    ("sparse", "use sparse matrix (default)")
    ("dense", "use dense matrix")
    ("verbose", "set loglevel to DEBUG")
    ("verbose2", "set loglevel to TRACE")
    ("binary,b", "use binary input/output");

  variables_map argmap;
  store(command_line_parser(argc,argv).options(opt).allow_unregistered().
        run(), argmap);
  notify(argmap);

  bool ispredict = false;
  string input, label, model, output;
  size_t num_iteration = 1000;
  double alpha = 0.01;
  double minibatch_fraction = 1.0;
  size_t hist_size = 10;
  double convTol = 0.001;
  bool intercept = false;
  bool binary = false;
  bool isdense = false;
#if defined(_SX) || defined(__ve__)
  MatType mType = HYBRID;
#else
  MatType mType = CRS;
#endif
  string solver = "sparse_lsqr";
  string dtype = "double";
  
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

  if(argmap.count("convergence-tolerance")){
    convTol = argmap["convergence-tolerance"].as<double>();
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

  if(argmap.count("dtype")){
    dtype = argmap["dtype"].as<string>();
  } 

  if(argmap.count("hist-size")){
    hist_size = argmap["hist-size"].as<size_t>();
  }

  if(argmap.count("sparse") && argmap.count("dense")) {
    cerr << "either provide --sparse or --dense to specify type of input matrix" << endl;
    cerr << opt << endl;
    exit(1);
  }

  if(argmap.count("sparse")){
    isdense = false;
    solver = "sparse_lsqr";
  }

  if(argmap.count("dense")){
    isdense = true;
    solver = "lapack";
  }

  if(argmap.count("solver")){
    solver = argmap["solver"].as<string>();
    if (not isdense && (solver == "lapack" || solver == "scalapack")) {
      cerr << "lapack/scalapack solver can only be used for dense data" << endl;
      cerr << opt << endl;
      exit(1);
    }
    else if (isdense && (solver == "sparse_lsqr")) {
      cerr << "sparse_lsqr solver can only be used for sparse data" << endl;
      cerr << opt << endl;
      exit(1);
    }
  } 

  if (dtype == "float") {
    if(ispredict) do_predict<float>(input, model, output, binary, isdense);
    else do_train<float>(input, label, output, solver,
                         num_iteration, alpha, minibatch_fraction, hist_size,
                         intercept, convTol, mType, binary, isdense);
  }
  else if (dtype == "double") {
    if(ispredict) do_predict<double>(input, model, output, binary, isdense);
    else do_train<double>(input, label, output, solver,
                          num_iteration, alpha, minibatch_fraction, hist_size,
                          intercept, convTol, mType, binary, isdense);
  }
  else REPORT_ERROR(USER_ERROR, "supported dtype is either float or double!\n");
}
