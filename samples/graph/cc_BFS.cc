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
        ("direct,d" , " if to generate direct graph file")
        ("prepare,p" , "if to generate the CRS matrix from original graph list file ")
        ("method,m" , value<std::string>(), "method (cc_bfs_2DMerge, cc_bfs_Scatter, cc_bfs_idxSort, cc_bfs_spMV)")
        // ("cout" , " enable screen output for the search result" )
        // ("fout" , " enable file output for the search result" )            
        ("upperbound,u" ,value<size_t>(), " upperbound for 2D merge")
        ("allgather", "use all_gatherv instead of gather to reduce communication overhead (currently only for CC_BFS)")
        ("hyb", "use hybrid spMspV strategy: 2DMerge + Scatter (for scale-free graphs)")
        ("matformat,f" , value<std::string>(), " matrix format (default: spMV: CRS(X86) HYB(SX) spMspV: CCS )") 
        ("scalefree,s" , " for scale-free graphs");
                
    variables_map argmap;
    store(command_line_parser(argc,argv).options(opt).allow_unregistered().
          run(), argmap);
    notify(argmap);                
                
    size_t hyb_th = 10000000;
    size_t upperbound_2DMerge = 256;

    bool if_prep = 0; // true if prepare data from raw dataset
    bool if_binary = 0; 
    bool if_graph_direct = 0;
    bool if_cout = 0;
    bool if_fout = 0;  
    bool if_scale_free = 0;
    bool if_allgather = 0;
    bool if_hyb = 0;
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
    if(argmap.count("direct")){
        if_graph_direct = true;
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
    if(argmap.count("upperbound")){
       upperbound_2DMerge = argmap["upperbound"].as<size_t>();
    }  
    if(argmap.count("matformat")){
       matformat = argmap["matformat"].as<std::string>();
    } 
    if(argmap.count("scalefree")){
       if_scale_free = true;
    }     
    if(argmap.count("allgather")){
       if_allgather = true;
    }    
    if(argmap.count("hyb")){
       if_hyb = true;
    }        
/////////////////////////////////////////////////////////////////
    
    
    
    graph* CCptr = new graph();      
    CCptr->set_if_cout(if_cout);
    CCptr->set_if_fout(if_fout);
    
    if(if_prep == true){
        CCptr->prep_graph_crs(data_p, if_graph_direct);
        data_p = data_p + "_graph_in_crs";      
    }
    //connected component
    CCptr->read_graph(data_p, if_binary);
    
    if(method_name == "cc_bfs_spMV"){
        throw std::logic_error("This method is not supported now");
        // CCptr->cc_bfs_spmv(matformat);
    }
    else if(method_name == "cc_bfs_Scatter"){
        CCptr->cc_bfs_scatter();
    }
    else if(method_name == "cc_bfs_idxSort"){

        CCptr->cc_bfs_idxsort(hyb_th, if_hyb);
    }     
    else if(method_name == "cc_bfs_2DMerge"){
        if(if_allgather == 1){
            throw std::logic_error("This method is not supported now");
            // CCptr->cc_bfs_2dmerge_allgather(upperbound_2DMerge);
        }else{
            CCptr->cc_bfs_2dmerge_gather(upperbound_2DMerge, if_scale_free, if_hyb);
        }
    }                
    else{
        std::cerr<<"ERROR: Method "<<method_name<<" doesn't exist!"<<std::endl;
        std::cerr << opt << std::endl;
        exit(1);
    }
    return 0;
}



