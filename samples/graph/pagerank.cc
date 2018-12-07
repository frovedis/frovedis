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
        ("input,i" , value<std::string>(), " input data path. When using binary files, the path is the directory containing all files. When using string file, the path is the file path.")           
        ("binary,b" , " use binary graph matrix file" )    
        ("dfactor,d", value<double>(), "damping factor (default: 0.15)")
        ("epsilon,e", value<double>(), "convergence threshold (default: 1E-4)")
        ("iter_max,", value<size_t>(), "maximum iterations (default: 100)")
        ("prepare,p" , "if to generate the CRS matrix from original graph list file ")
        ("method,m" , value<std::string>(), "method (pagerank_v1, pagerank_v1_shrink, pagerank_v2, pagerank_v2_shrink")    
        // ("cout" , " enable screen output for the search result" )
        // ("fout" , " enable file output for the search result" )  
        ("matformat,f" , value<std::string>(), " matrix format (default: spMV: CRS(X86) HYB(SX)");
    
    variables_map argmap;
    store(command_line_parser(argc,argv).options(opt).allow_unregistered().
          run(), argmap);
    notify(argmap);                

    bool if_prep = 0; // true if prepare data from raw dataset
    bool if_binary = 0; 
    bool if_cout = 0;
    bool if_fout = 0;  

    size_t iter_max = 100;
    double epsilon = 1e-4; //default convergence threshold
    double df = 0.15;
    
    std::string data_p;
    std::string method_name;
    std::string matformat;
// #ifndef _SX
#if !defined(_SX) && !defined(__ve__)
    matformat = "CRS";
#else
    matformat = "HYB";
#endif
    
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
    if(argmap.count("binary")){
        if_binary = true;
    }
    if(argmap.count("prepare")){
        if_prep = true;
    } 
    if(argmap.count("method")){
        method_name = argmap["method"].as<std::string>();
    }else{
      std::cerr << "input is not specified" << std::endl;
      std::cerr << opt << std::endl;
      exit(1);        
    }      
    // if(argmap.count("cout")){
    //     if_cout = true;
    // }   
    // if(argmap.count("fout")){
    //     if_fout = true;
    // }  
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
       epsilon = argmap["iter_max"].as<size_t>();
    }        
/////////////////////////////////////////////////////////////////
  
    graph* CCptr = new graph();      
    CCptr->set_if_cout(if_cout);
    CCptr->set_if_fout(if_fout);
    
    if(if_prep == true){
        CCptr->prep_graph_crs_pagerank(data_p);
        data_p = data_p + "_graph_in_crs_pg";      
    }
    //connected component
    CCptr->read_graph_pagerank(data_p, if_binary);
    
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
    return 0;
}



