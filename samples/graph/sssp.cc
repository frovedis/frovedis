#include <frovedis.hpp>
#include <frovedis/ml/graph/graph.hpp>

#include <boost/program_options.hpp>

using namespace boost;
using namespace frovedis;
using namespace std;

int main(int argc, char* argv[]){
    frovedis::use_frovedis use(argc, argv);
    
    using namespace boost::program_options;
    
    options_description opt("option");
    opt.add_options()
        ("help,h", "produce help message")
        ("input,i" , value<std::string>(), "input data path containing either edgelist or adjacency matrix data") 
        ("dtype,t" , value<std::string>(), "input data type (int, float or double) [default: int]") 
        ("output,o" , value<std::string>(), "output data path to save cc results")
        ("source,s", value<size_t>(), "source vertex id (default: 1)") 
        ("prepare,p" , "whether to generate the CRS matrix from original edgelist file ");
                
    variables_map argmap;
    store(command_line_parser(argc,argv).options(opt).allow_unregistered().
          run(), argmap);
    notify(argmap);                
                
    bool if_prep = 0; // true if prepare data from raw dataset
    std::string data_p, out_p, dtype = "int";
    size_t source_vertex = 1;
    
    //////////////////////////   command options   ///////////////////////////
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
    if(argmap.count("prepare")){
      if_prep = true;
    }
    if(argmap.count("source")){
       source_vertex = argmap["source"].as<size_t>();
    }
    /////////////////////////////////////////////////////////////////
    
    if (dtype == "int") {
      graph<int> gr;
      if(if_prep) gr = read_edgelist<int>(data_p);
      else {
        auto mat = make_crs_matrix_load<int>(data_p);
        gr = graph<int>(mat);
      }
      auto res = gr.single_source_shortest_path(source_vertex);
      res.save(out_p);
    }      
    else if (dtype == "float") {
      graph<float> gr;
      if(if_prep) gr = read_edgelist<float>(data_p);
      else {
        auto mat = make_crs_matrix_load<float>(data_p);
        gr = graph<float>(mat);
      }
      auto res = gr.single_source_shortest_path(source_vertex);
      res.save(out_p);
    }      
    else if (dtype == "double") {
      graph<double> gr;
      if(if_prep) gr = read_edgelist<double>(data_p);
      else {
        auto mat = make_crs_matrix_load<double>(data_p);
        gr = graph<double>(mat);
      }
      auto res = gr.single_source_shortest_path(source_vertex);
      res.save(out_p);
    }      
    else {
      std::cerr << "Supported dtypes are only int, float and double!\n";
      std::cerr << opt << std::endl;
      exit(1);
    }
    return 0;
}

