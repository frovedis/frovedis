#include <frovedis.hpp>
#include <frovedis/ml/graph/graph.hpp>
#include <boost/program_options.hpp>

using namespace boost;
using namespace frovedis;
using namespace std;

template <class T>
void call_cc(const std::string& data_p, 
             const std::string& out_p,
             bool if_prep,
             int opt_level,
             double threshold) {
  graph<T> gr;
  if(if_prep) gr = read_edgelist<T>(data_p);
  else {
    auto mat = make_crs_matrix_load<T>(data_p);
    gr = graph<T>(mat);
  }
  time_spent t(INFO);
  auto res = gr.connected_components(opt_level, threshold);
  t.show("connected components computation time: ");
  res.save(out_p);
  res.debug_print(5);
}

int main(int argc, char* argv[]){
    frovedis::use_frovedis use(argc, argv);
    using namespace boost::program_options;
    
    options_description opt("option");
    opt.add_options()
        ("help,h", "produce help message")
        ("input,i" , value<std::string>(), "input data path containing either edgelist or adjacency matrix data") 
        ("dtype,t" , value<std::string>(), "input data type (int, float or double) [default: int]") 
        ("output,o" , value<std::string>(), "output data path to save cc results") 
        ("opt-level", value<int>(), "optimization level 0, 1 or 2 (default: 2)") 
        ("hyb-threshold", value<double>(), "threshold value for hybrid optimization [0.0 ~ 1.0] (default: 0.4)") 
        ("verbose", "set loglevel to DEBUG")
        ("verbose2", "set loglevel to TRACE")
        ("prepare,p" , "whether to generate the CRS matrix from original edgelist file ");
                
    variables_map argmap;
    store(command_line_parser(argc,argv).options(opt).allow_unregistered().
          run(), argmap);
    notify(argmap);                
                
    bool if_prep = 0; // true if prepare data from raw dataset
    int opt_level = 2;
    double threshold = 0.4;
    std::string data_p, out_p, dtype = "int";
    
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
    if(argmap.count("opt-level")){
       opt_level = argmap["opt-level"].as<size_t>();
    }
    if(argmap.count("hyb-threshold")){
       threshold = argmap["hyb-threshold"].as<double>();
    }
    if(argmap.count("verbose")){
      set_loglevel(DEBUG);
    }
    if(argmap.count("verbose2")){
      set_loglevel(TRACE);
    }

    try{
      if (dtype == "int") {
        call_cc<int>(data_p, out_p, if_prep, opt_level, threshold);
      }      
      else if (dtype == "float") {
        call_cc<float>(data_p, out_p, if_prep, opt_level, threshold);
      }      
      else if (dtype == "double") {
        call_cc<double>(data_p, out_p, if_prep, opt_level, threshold);
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

