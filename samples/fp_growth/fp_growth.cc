#include <frovedis.hpp>
#include <frovedis/ml/fpm/fp_growth.hpp>
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
        ("input,i" , value<std::string>(), "input data path containing transaction dataframe.") 
        ("output,o" , value<std::string>(), "output path for saving model.") 
        ("min-support,s", value<double>(), "minimum support value [default: 0.2]") //TODO: rephrase the user message
        ("conf,c", value<double>(), "confidence value for rule mining [default: 0.5]")
        ("to-compress", value<bool>(), "whether to compress fp trees [default: true]") 
        ("verbose", "set loglevel to DEBUG")
        ("verbose2", "set loglevel to TRACE");
                
    variables_map argmap;
    store(command_line_parser(argc,argv).options(opt).allow_unregistered().
          run(), argmap);
    notify(argmap);                
                
    std::string data_p, out_p;
    double min_sup = 0.2, conf = 0.5;
    bool to_compression_out = 1;
    
    if(argmap.count("help")){
      std::cerr << opt << std::endl;
      exit(1);
    }
    if(argmap.count("input")){
      data_p = argmap["input"].as<std::string>();
    }    
    else {
      cerr << "input is not specified" << endl;
      cerr << opt << endl;
      exit(1);
    }

    if(argmap.count("output")){
      out_p = argmap["output"].as<std::string>();
    }    
    else {
      cerr << "output is not specified" << endl;
      cerr << opt << endl;
      exit(1);
    }

    if(argmap.count("min-support")){
       min_sup = argmap["min-support"].as<double>();
    }

    if(argmap.count("conf")){
       conf = argmap["conf"].as<double>();
    }

    if(argmap.count("to-compress")){
       to_compression_out = argmap["to-compress"].as<bool>();
    }

    if(argmap.count("verbose")){
      set_loglevel(DEBUG);
    }

    if(argmap.count("verbose2")){
      set_loglevel(TRACE);
    }
    
    try {
      auto t = make_dftable_loadtext(data_p, 
                                 {"int", "int"}, 
                                 {"trans_id", "item"});
      time_spent grow(INFO), tree(INFO);
      grow.lap_start();
      auto model = grow_fp_tree(t, min_sup, to_compression_out);
      grow.lap_stop();
      grow.show_lap("grow_fp_tree: ");

      std::cout << "tree-depth: " << model.get_depth() 
                << "; FIS-count: " << model.get_count() << std::endl;
      //model.debug_print();
      model.save(out_p);

      /*
       * needs to be fixed
       *
      tree.lap_start();
      auto rule = model.generate_rules(conf);
      rule.debug_print();
      tree.lap_stop();
      tree.show_lap("generate_rules: ");
      */
    }
    catch (std::exception& e) {
      std::cout << "exception caught: " << e.what() << std::endl; 
    }
    return 0;
}

