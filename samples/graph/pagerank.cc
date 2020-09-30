#include <frovedis.hpp>
#include <frovedis/ml/graph/graph.hpp>
#include <boost/program_options.hpp>

using namespace boost;
using namespace frovedis;
using namespace std;

template <class T>
void call_pagerank(const std::string& data_p,
                   const std::string& out_p,
                   bool if_prep,
                   double df, 
                   double epsilon, 
                   size_t niter) {
  graph<T> gr;
  if(if_prep) gr = read_edgelist<T>(data_p);
  else {
    auto mat = make_crs_matrix_load<T>(data_p);
    gr = graph<T>(mat);
  }
  auto res = gr.pagerank(df, epsilon, niter);
  make_dvector_scatter(res).saveline(out_p);
}

int main(int argc, char* argv[]){
    frovedis::use_frovedis use(argc, argv);
    using namespace boost::program_options;
    
    options_description opt("option");
    opt.add_options()
        ("help,h", "produce help message")
        ("input,i" , value<std::string>(), "input data path containing either edgelist or adjacency matrix data") 
        ("dtype,t" , value<std::string>(), "input data type (int, float or double) [default: int]") 
        ("output,o" , value<std::string>(), "output data path to save ranking scores")
        ("dfactor,d", value<double>(), "damping factor (default: 0.15)") 
        ("epsilon,e", value<double>(), "convergence threshold (default: 1E-4)")
        ("max_iter,k", value<size_t>(), "maximum no. of iterations (default: 100)") 
        ("verbose", "set loglevel to DEBUG")
        ("verbose2", "set loglevel to TRACE")
        ("prepare,p" , "whether to generate the CRS matrix from original edgelist file ");
                
    variables_map argmap;
    store(command_line_parser(argc,argv).options(opt).allow_unregistered().
          run(), argmap);
    notify(argmap);                
                
    bool if_prep = 0; // true if prepare data from raw dataset
    std::string data_p, out_p, dtype = "int";
    size_t niter = 100;
    double epsilon = 1e-4; 
    double df = 0.15;

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
    if(argmap.count("output")){
      out_p = argmap["output"].as<std::string>();
    } else {
      std::cerr << "output path is not specified" << std::endl;
      std::cerr << opt << std::endl;
      exit(1);
    }    
    if(argmap.count("prepare")){
      if_prep = true;
    }
    if(argmap.count("dtype")){
      dtype = argmap["dtype"].as<std::string>();
    }    
    if(argmap.count("dfactor")){
       df = argmap["dfactor"].as<double>();
    }
    if(argmap.count("epsilon")){
       epsilon = argmap["epsilon"].as<double>();
    }
    if(argmap.count("max_iter")){
       niter = argmap["max_iter"].as<size_t>();
    }
    if(argmap.count("verbose")){
      set_loglevel(DEBUG);
    }
    if(argmap.count("verbose2")){
      set_loglevel(TRACE);
    }

    try {
      if (dtype == "int") {
        call_pagerank<int>(data_p, out_p, if_prep, df, epsilon, niter);
      }      
      else if (dtype == "float") {
        call_pagerank<float>(data_p, out_p, if_prep, df, epsilon, niter);
      }      
      else if (dtype == "double") {
        call_pagerank<double>(data_p, out_p, if_prep, df, epsilon, niter);
      }      
      else {
        std::cerr << "Supported dtypes are only int, float and double!\n";
        std::cerr << opt << std::endl;
        exit(1);
      }
    }
    catch (std::exception& e) {
      std::cout << "exception caught: " << e.what() << std::endl; 
    }
    return 0;
}

