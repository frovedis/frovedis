#include <frovedis.hpp>
#include <frovedis/matrix/rowmajor_matrix.hpp>
#include <frovedis/matrix/tsne.hpp>
#include <boost/program_options.hpp>

using namespace boost;
using namespace frovedis;

template <class T>
void call_tsne(const std::string& data_p, const std::string& out_p, double perplexity,
               double early_exaggeration, double min_grad_norm, double learning_rate, 
               size_t n_components, size_t max_iter, size_t n_iter_without_progress, 
               const std::string& metric, const std::string& method, 
               const std::string& init, bool verbose) { 
  time_spent load_t(INFO);
  load_t.lap_start();
  auto mat = make_rowmajor_matrix_load<T>(data_p);
  load_t.lap_stop();
  load_t.show_lap("data loading time: ");
  std::cout << "n_samples = " << mat.num_row
            << ", n_features = " << mat.num_col
            << std::endl;
  time_spent tsne_t(INFO);
  tsne_t.lap_start();

  TSNE<T> t1;
  t1.set_perplexity(perplexity).
     set_early_exaggeration(early_exaggeration).
     set_min_grad_norm(min_grad_norm).
     set_learning_rate(learning_rate).
     set_n_components(n_components).
     set_n_iter(max_iter).
     set_n_iter_without_progress(n_iter_without_progress).
     set_metric(metric).
     set_method(method).
     set_init(init).
     set_verbose(verbose); 

  auto Y_mat = t1.fit_transform(mat); 
  auto n_iter = t1.get_n_iter_();
  auto kl_divergence = t1.get_kl_divergence_();

  tsne_t.lap_stop();
  tsne_t.show_lap("Overall computation time: ");
  Y_mat.save(out_p);
  std::cout << "n_iter_ = " << n_iter << std::endl;
  std::cout << "kl_divergence_ = " << kl_divergence << std::endl;
}

int main(int argc, char* argv[]) {
  use_frovedis use(argc, argv);

  using namespace boost::program_options;

  options_description opt("option");
  opt.add_options()
      ("help,h", "produce help message")
      ("input,i" , value<std::string>(), "input data path containing input data for T-SNE") 
      ("dtype,t" , value<std::string>(), "input data type (float or double) [default: double]") 
      ("output,o" , value<std::string>(), "output data path to save output embeddings")
      ("max_iter,k", value<size_t>(), "maximum no. of iterations (default: 1000)") 
      ("perplexity,p", value<double>(), "number of nearest neighbors for each point (default: 30.0)")
      ("early_exaggeration,e", value<double>(), "controls the space between natural clusters in the embedded space (default: 12.0)")
      ("min_grad_norm,g", value<double>(), "gradient norm threshold (default: 1e-7)")
      ("learning_rate,l", value<double>(), "learning rate for t-SNE (default: 200.0)")
      ("n_components,n", value<size_t>(), "dimension of the embedded space (default: 2)")
      ("niter_without_progress", value<size_t>(), "maximum number of iterations without progress before we abort the optimization (default: 300)")
      ("metric,m" , value<std::string>(), "the metric (euclidean or precomputed) to use when calculating distance (default: euclidean)")
      ("method" , value<std::string>(), "the method (exact) to use for TSNE computation (default: exact)")
      ("init" , value<std::string>(), "the init (random) to use for initializing Y mat (default: random)")
      ("verbose", "set loglevel to DEBUG");

  variables_map argmap;
  store(command_line_parser(argc,argv).options(opt).allow_unregistered().
        run(), argmap);
  notify(argmap);                

  std::string dtype =  "double";
  std::string data_p =  "";
  std::string out_p =  "";
  size_t max_iter = 1000;
  double perplexity = 30.0;
  double early_exaggeration = 12.0;
  double min_grad_norm = 1e-7;
  double learning_rate = 200.0;
  size_t n_components = 2;
  size_t n_iter_without_progress = 300;
  std::string metric = "euclidean"; //possible values = ["euclidean", "precomputed"]
  std::string method = "exact"; //possible values = ["exact"]
  std::string init = "random"; //possible values = ["random"]
  bool verbose = false;

  if(argmap.count("help")){
    std::cerr << opt << std::endl;
    exit(1);
  }
  if(argmap.count("input")){
    data_p = argmap["input"].as<std::string>();
  } else {
    std::cerr << "input path is not specified" << std::endl;
    std::cerr << opt << std::endl;
    exit(1);
  }    
  if(argmap.count("dtype")){
    dtype = argmap["dtype"].as<std::string>();
  }    
  if(argmap.count("output")){
    out_p = argmap["output"].as<std::string>();
  } else {
    std::cerr << "output path is not specified" << std::endl;
    std::cerr << opt << std::endl;
    exit(1);
  }    
  if(argmap.count("max_iter")){
     max_iter = argmap["max_iter"].as<size_t>();
  }
  if(argmap.count("perplexity")){
     perplexity = argmap["perplexity"].as<double>();
  }
  if(argmap.count("early_exaggeration")){
     early_exaggeration = argmap["early_exaggeration"].as<double>();
  }
  if(argmap.count("min_grad_norm")){
     min_grad_norm = argmap["min_grad_norm"].as<double>();
  }
  if(argmap.count("learning_rate")){
     learning_rate = argmap["learning_rate"].as<double>();
  }
  if(argmap.count("n_components")){
     n_components = argmap["n_components"].as<size_t>();
  }
  if(argmap.count("niter_without_progress")){
     n_iter_without_progress = argmap["niter_without_progress"].as<size_t>();
  }
  if(argmap.count("metric")){
    metric = argmap["metric"].as<std::string>();
  }  
  if(argmap.count("method")){
    method = argmap["method"].as<std::string>();
  }
  if(argmap.count("init")){
    init = argmap["init"].as<std::string>();
  }
  if(argmap.count("verbose")){
    set_loglevel(DEBUG);
    verbose = true;
  }

  try {
    if (dtype == "float") {
      call_tsne<float>(data_p, out_p, perplexity, early_exaggeration, min_grad_norm, 
                       learning_rate, n_components, max_iter, 
                       n_iter_without_progress, metric, method, init, verbose);
    }
    else if (dtype == "double") {
      call_tsne<double>(data_p, out_p, perplexity, early_exaggeration, min_grad_norm, 
                        learning_rate, n_components, max_iter, 
                        n_iter_without_progress, metric, method, init, verbose);
    }
    else {
      std::cerr << "Supported dtypes are only float and double!\n";
      std::cerr << opt << std::endl;
      exit(1);
    }
  }
  catch(std::exception& e) {
    std::cout << "exception caught: " << e.what() << std::endl;
  }
  return 0;
}

