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
        ("min-support,s", value<double>(), "minimal support level of the frequent pattern. [default: 0.2]") 
        ("min-confidence,c", value<double>(), "minimum confidence value for rule mining [default: 0.5]")
        ("tid-dtype", value<std::string>(), "how to load tid column (0th) from input data (as int, long etc.) [default: int]") 
        ("item-dtype", value<std::string>(), "how to load item column (1st) from input data (as int, long, string etc.) [default: int]") 
        ("tree-depth", value<int>(), "required depth till which tree needs to be constructed (>=1) [default: INT_MAX]") 
        ("compression-point", value<int>(), "ith point from which trees will be compressed (>=2) [default: 4]") 
        ("mem-opt-level", value<int>(), "memory opt level to use (either 0 or 1) [default: 0]") 
        ("verbose", "set loglevel to DEBUG")
        ("verbose2", "set loglevel to TRACE");
                
    variables_map argmap;
    store(command_line_parser(argc,argv).options(opt).allow_unregistered().
          run(), argmap);
    notify(argmap);                
                
    std::string data_p, out_p, tid_dtype = "int", item_dtype = "int";
    double min_support = 0.2, conf = 0.5;
    int tree_depth = std::numeric_limits<int>::max();
    int compression_point = 4;
    int mem_opt_level = 0;
    
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
       min_support = argmap["min-support"].as<double>();
    }

    if(argmap.count("min-confidence")){
       conf = argmap["min-confidence"].as<double>();
    }

    if(argmap.count("tid-dtype")){
      tid_dtype = argmap["tid-dtype"].as<std::string>();
    }    

    if(argmap.count("item-dtype")){
      item_dtype = argmap["item-dtype"].as<std::string>();
    }    

    if(argmap.count("tree-depth")){
       tree_depth = argmap["tree-depth"].as<int>();
    }

    if(argmap.count("compression-point")){
       compression_point = argmap["compression-point"].as<int>();
    }

    if(argmap.count("mem-opt-level")){
       mem_opt_level = argmap["mem-opt-level"].as<int>();
    }

    if(argmap.count("verbose")){
      set_loglevel(DEBUG);
    }

    if(argmap.count("verbose2")){
      set_loglevel(TRACE);
    }
    
    try {
      auto t = make_dftable_loadtext(data_p, 
                                 {tid_dtype, item_dtype}, 
                                 {"trans_id", "item"});
      time_spent t_tree(INFO), t_rule(INFO);
      t_tree.lap_start();
      auto model = grow_fp_tree(t, min_support, tree_depth, 
                                compression_point, mem_opt_level);
      t_tree.lap_stop();
      t_tree.show_lap("generate freq-itemsets: ");
      //model.debug_print();
      std::cout << "tree-depth: " << model.get_depth() 
                << "; FIS-count: " << model.get_count() << std::endl;

      t_rule.lap_start();
      auto rule = model.generate_rules(conf);
      t_rule.lap_stop();
      t_rule.show_lap("generate_rules: ");
      //rule.debug_print();
      std::cout << "rule-count: " << rule.get_count() << std::endl;

      make_directory(out_p);
      rule.save(out_p + "/rule");
      model.save(out_p + "/model");
      return 0;
    }
    catch (std::exception& e) {
      std::cout << "exception caught: " << e.what() << std::endl; 
    }
    return 0;
}

