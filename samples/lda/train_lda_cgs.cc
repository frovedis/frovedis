
#include <boost/program_options/options_description.hpp>
#include <boost/program_options.hpp>

#include <frovedis/ml/lda/lda_cgs.hpp>


using namespace frovedis;

int main(int argc, char* argv[]){
    
    use_frovedis use(argc, argv);
    
    using namespace boost::program_options;
    
    options_description opt("option");
    opt.add_options()
        ("help,h", "print help message")
        ("algorithm,g", value<std::string>(), "sampling algorithm: \n\"original\": original cgs, "
            "\n\"dp\": doc_proposal, \n\"wp\": word_proposal, \n\"cp\":cycle_proposal, \n\"sparse\": sparse_lda")
        ("input,i", value<std::string>(), "path for input corpus of crs matrix")
        ("output,o", value<std::string>(), "path for output model of LDA")       
        ("topics,p", value<int>(), "number of topics.")           
        ("train_iter", value<int>(), "number of iterations.")     
        ("explore_iter", value<int>(), "number of iteration to explore optimal hyperparams")       
        // ("delay_update", "update model after sampling for all tokens")
        ("eval_cycle", value<int>(), "evaluate during training: 0 means not to evaluate")
        ("trace", "set log level to TRACE")
        ("debug", "set log level to DEBUG")
        ("binary", "save/load file with binary")
        ("alpha", value<double>(), "alpha")
        ("beta" , value<double>(), "beta");   
        
    variables_map argmap;
    store(command_line_parser(argc,argv).options(opt).allow_unregistered().run(), argmap);
    notify(argmap);                

    std::string algorithm; 
    std::string input_path;
    std::string output_path;
    double alpha = 0.1;
    double beta  = 0.01;
    int topics = 1000;
    int train_iter = 30;
    int explore_iter = 0;  
    bool vectorized  = true;  
    int eval_cycle = 0;
    // bool delay_update = false;
    set_loglevel(INFO);
    bool binary = false;
    
#if !(defined(_SX) || defined(__ve__))
    vectorized = false;
#endif
    
    if(argmap.count("help")){
        std::cerr << opt << std::endl;
        finalizefrovedis(0);
    }
    if(argmap.count("algorithm")){
        algorithm = argmap["algorithm"].as<std::string>();
        if(algorithm!="original" && algorithm!="dp" && algorithm!="wp" && algorithm!="cp" && algorithm!="sparse"){
            std::cerr<<"ERROR: Algorithm "<<algorithm<<" doesn't exist!"<<std::endl;
            std::cerr << opt << std::endl;
            finalizefrovedis(1);
        }
    } else {
      std::cerr << "algorithm is not specified" << std::endl;
      std::cerr << opt << std::endl;
      finalizefrovedis(1);    
    }    
    if(argmap.count("input")){
      input_path = argmap["input"].as<std::string>();
    } else {
      std::cerr << "input path for corpus is not specified" << std::endl;
      std::cerr << opt << std::endl;
      finalizefrovedis(1);
    }     
    if(argmap.count("output")){
      output_path = argmap["output"].as<std::string>();
    } else {
      std::cerr << "output path for model is not specified" << std::endl;
      std::cerr << opt << std::endl;
      finalizefrovedis(1);
    }     
    if(argmap.count("alpha")) alpha = argmap["alpha"].as<double>();     
    if(argmap.count("beta"))  beta = argmap["beta"].as<double>();
    if(argmap.count("topics")){       
        topics = argmap["topics"].as<int>();
        // alpha = 50/(double)topics;   // XXX
    }     
    if(argmap.count("train_iter")) { train_iter = argmap["train_iter"].as<int>(); }
    if(argmap.count("explore_iter")) { explore_iter = argmap["explore_iter"].as<int>(); }
    // if(argmap.count("delay_update")) { delay_update = true; }
    if(argmap.count("eval_cycle")) { eval_cycle = argmap["eval_cycle"].as<int>(); }
    if (argmap.count("debug")) { set_loglevel(DEBUG); }
    if (argmap.count("trace")) { set_loglevel(TRACE); }
    if (argmap.count("binary")) { binary = true; }

    // if(delay_update && algorithm!="dp" && algorithm!="wp" && algorithm!="cp" && algorithm!="original"){
    //     std::cerr<<"WARNING: delay update for algorithm *" << algorithm << "* has not been implemented!" << " ";
    //     std::cerr<<"The model will be updated after per-token sampling!" << std::endl;
    //     delay_update = false;
    // }    
    if(vectorized && algorithm!="dp" && algorithm!="wp" && algorithm!="cp" && algorithm!="original") {
        std::cerr<<"WARNING: vectorized sampling for algorithm *" << algorithm << "* has not been implemented!" << " ";
        std::cerr<<"Running original CGS" << std::endl;
        algorithm = "original";
    }
    
    rowmajor_matrix<int32_t> doc_topic_count;
    auto data_train = binary ? make_crs_matrix_loadbinary<int32_t>(input_path) : make_crs_matrix_load<int32_t>(input_path);
    auto model = lda_train<int32_t>(data_train,alpha,beta,topics,train_iter,algorithm,explore_iter,   
                                    eval_cycle, doc_topic_count); 
    model.save_as(output_path, binary); 
    binary ? doc_topic_count.savebinary(output_path + "/doc_topic")
           : doc_topic_count.save(output_path + "/doc_topic"); 

}
