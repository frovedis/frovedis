/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   main.cpp
 * Author: haoran
 *
 * Created on October 2, 2017, 9:09 AM
 */

#include <cstdlib>
#include <boost/program_options/options_description.hpp>
#include <frovedis.hpp>
#include <frovedis/matrix/crs_matrix.hpp>
#include <vector>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>      // std::istringstream
#include <unordered_map>
#include <ctime>// record time 
#include <cmath>
#include <numeric>
#include <frovedis/ml/graph/graph.hpp>
// #include <frovedis/ml/graph/graph.cpp>
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
      ("input,i" , value<std::string>(), "input data path")
      ("output,o" , value<std::string>(), "output data path")
      ("binary,b" , "use binary matrix file" ) 
      ("dfactor,d", value<double>(), "damping factor (default: 0.15)")
      ("epsilon,e", value<double>(), "convergence threshold (default: 1E-4)")
      ("iter_max,t", value<size_t>(), "maximum iterations (default: 100)")
      ("prepare,p" , "output CRS matrix from mtx file (do not execute pagerank): each line contains src_node dst_node")
      ("method,m" , value<std::string>(), "method (pagerank_v1, pagerank_v1_shrink, pagerank_v2, pagerank_v2_shrink (only affect performance. default: pagerank_v1)")
      ("matformat,f" , value<std::string>(), "matrix format (default: spMV: CRS(X86) HYB(SX)")
      ("verbose,v", "show execution time")
      ("verbose2", "show various information");
    
    variables_map argmap;
    store(command_line_parser(argc,argv).options(opt).allow_unregistered().
          run(), argmap);
    notify(argmap);                

    bool if_prep = 0; // true if prepare data from raw dataset
    bool if_binary = 0; 

    size_t iter_max = 100;
    double epsilon = 1e-4; //default convergence threshold
    double df = 0.15;
    
    std::string data_p;
    std::string output_p;
    std::string method_name = "pagerank_v1";
    std::string matformat;
// #ifndef _SX
#if !defined(_SX) && !defined(__ve__)
    matformat = "CRS";
#else
    matformat = "HYB";
#endif
    bool if_verbose = 0;
    bool if_verbose2 = 0;
    
//////////////////////////   command options   ///////////////////////////
    if(argmap.count("help")){
        std::cerr << opt << std::endl;
        exit(1);
    }

    if(argmap.count("input")){
      data_p = argmap["input"].as<std::string>();
    } else {
      std::cerr << "input is not specified" << std::endl;
      std::cerr << opt << std::endl;
      exit(1);
    }    
    if(argmap.count("output")){
      output_p = argmap["output"].as<std::string>();
    } else {
      std::cerr << "output is not specified" << std::endl;
      std::cerr << opt << std::endl;
      exit(1);
    }    
    if(argmap.count("binary")){
        if_binary = true;
    }
    if(argmap.count("prepare")){
        if_prep = true;
    } 
    if(argmap.count("method")){
        method_name = argmap["method"].as<std::string>();
    }
    if(argmap.count("matformat")){
       matformat = argmap["matformat"].as<std::string>();
    }    
    if(argmap.count("dfactor")){
       df = argmap["dfactor"].as<double>();
    }  
    if(argmap.count("epsilon")){
       epsilon = argmap["epsilon"].as<double>();
    } 
    if(argmap.count("iter_max")){
       iter_max = argmap["iter_max"].as<size_t>();
    }        
    if(argmap.count("verbose")){
        if_verbose = true;
    }
    if(argmap.count("verbose2")){
        if_verbose2 = true;
    }
/////////////////////////////////////////////////////////////////

    if(if_verbose2) set_loglevel(TIME_RECORD_LOG_LEVEL);
  
    graph* CCptr = new graph();      
    
    if(if_prep == true){
        CCptr->prep_graph_crs_pagerank(data_p, output_p);
        return 0;
    }
    time_spent t;
    CCptr->read_graph_pagerank(data_p, if_binary);
    if(if_verbose) t.show("read: ");
    
    if(method_name == "pagerank_v1"){
        CCptr->pagerank_v1(matformat, df, epsilon, iter_max);
    }              
    else if(method_name == "pagerank_v1_shrink"){
        CCptr->pagerank_v1_shrink(matformat, df, epsilon, iter_max);
    }
    else if(method_name == "pagerank_v2"){
        CCptr->pagerank_v2(matformat, df, epsilon, iter_max);
    }
    else if(method_name == "pagerank_v2_shrink"){
        CCptr->pagerank_v2_shrink(matformat, df, epsilon, iter_max);
    }    
    else{
        std::cerr<<"ERROR: Method "<<method_name<<" doesn't exist!"<<std::endl;
        std::cerr << opt << std::endl;
        exit(1);
    }
    if(if_verbose) t.show("pagerank: ");
    if(if_binary) CCptr->savebinary_pagerank(output_p);
    else CCptr->save_pagerank(output_p);
    if(if_verbose) t.show("save: ");
    return 0;
}



