
#include <boost/program_options/options_description.hpp>
#include <boost/program_options.hpp>

#include <frovedis.hpp>
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
        ("model,m", value<std::string>(), "path for trained LDA model")
        ("test,t", value<std::string>(), "path for test corpus of crs matrix")       
        ("perplexity", value<std::string>(), "path for output perplexity")       
        ("likelihood", value<std::string>(), "path for output likelihood")       
        ("test_iter", value<int>(), "number of testing iterations.")
        ("explore_iter", value<int>(), "number of iteration to explore optimal hyperparams")       
        // ("delay_update", "update model after sampling for all tokens")
        ("trace", "set log level to TRACE")
        ("debug", "set log level to DEBUG")
        ("binary", "save/load file with binary")
        ("alpha", value<double>(), "alpha")
        ("beta" , value<double>(), "beta");   
        
    variables_map argmap;
    store(command_line_parser(argc,argv).options(opt).allow_unregistered().run(), argmap);
    notify(argmap);                

    std::string algorithm; 
    std::string model_path;
    std::string test_path;
    std::string perplexity_path;
    std::string likelihood_path;
    double alpha = 0.1;
    double beta  = 0.01;
    int test_iter = 0;
    int explore_iter = 10;  
    bool vectorized  = true;  
    // bool delay_update = false;
    // bool save_doc_model = false;
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
    if(argmap.count("model")){
      model_path = argmap["model"].as<std::string>();
    } else {
      std::cerr << "path for LDA model is not specified" << std::endl;
      std::cerr << opt << std::endl;
      finalizefrovedis(1);
    }     
    if(argmap.count("test")){
      test_path = argmap["test"].as<std::string>();
    } else {
      std::cerr << "path for test corpus is not specified" << std::endl;
      std::cerr << opt << std::endl;
      finalizefrovedis(1);
    }     
    if(argmap.count("alpha")) alpha = argmap["alpha"].as<double>();     
    if(argmap.count("beta"))  beta = argmap["beta"].as<double>();
    if(argmap.count("test_iter")) { test_iter = argmap["test_iter"].as<int>(); }
    if(argmap.count("explore_iter")) { explore_iter = argmap["explore_iter"].as<int>(); }
    // if(argmap.count("delay_update")) { delay_update = true; }
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
    
    auto data_test = binary ? make_crs_matrix_loadbinary<int32_t>(test_path) : make_crs_matrix_load<int32_t>(test_path);
    lda_model<int32_t> model;
    model.load_as(model_path, binary);
    std::vector<double> perplexity_buf;
    std::vector<double> likelihood_buf;
    lda_test<int32_t>(data_test, alpha,beta,test_iter,algorithm,explore_iter,
                      model, perplexity_buf, likelihood_buf);
    
    if(argmap.count("perplexity")){
        perplexity_path = argmap["perplexity"].as<std::string>();
        if (binary) {
            make_dvector_scatter(perplexity_buf).savebinary(perplexity_path);
        } else {
            make_dvector_scatter(perplexity_buf).saveline(perplexity_path);            
        }
    }     
    if(argmap.count("likelihood")){
        likelihood_path = argmap["likelihood"].as<std::string>();
        if (binary) {
            make_dvector_scatter(likelihood_buf).savebinary(likelihood_path);
        } else {
            make_dvector_scatter(likelihood_buf).saveline(likelihood_path);
        }
    }     
}
