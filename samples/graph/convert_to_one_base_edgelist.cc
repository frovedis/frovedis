#include <frovedis.hpp>
#include <frovedis/dataframe.hpp>
#include <boost/program_options.hpp>

using namespace boost;
using namespace frovedis;
using namespace std;

void to_one_base(const std::string& i_fname,
                 const std::string& o_fname,
                 char delim,
                 bool withWeight) {
  dftable df;
  if(withWeight) {
    df = make_dftable_loadtext(i_fname, 
                               {"unsigned long", "unsigned long", "double"},
                               {"src", "dst", "wgt"}, delim);
  }
  else {
    df = make_dftable_loadtext(i_fname, 
                               {"unsigned long", "unsigned long"},
                               {"src", "dst"}, delim);
  }
  std::cout << "input df: \n"; df.show();

  auto srcvec = df.group_by({"src"})
                  .select({"src"})
                  .sort("src")
                  .as_dvector<unsigned long>("src")
                  .gather();

  auto dstvec = df.group_by({"dst"})
                  .select({"dst"})
                  .sort("dst")
                  .as_dvector<unsigned long>("dst")
                  .gather();

  auto nodeid = set_union(srcvec, dstvec);
  auto num_node = nodeid.size();
  std::vector<unsigned long> enc_nodeid(num_node);
  auto enc_nodeidp = enc_nodeid.data();
  for(size_t i = 0; i < num_node; ++i) enc_nodeidp[i] = i + 1;

  dftable tmp; 
  tmp.append_column("id", make_dvector_scatter(nodeid));
  tmp.append_column("encoded_id", make_dvector_scatter(enc_nodeid));
  //tmp.show();

  auto c1 = df.columns();
  auto c2 = tmp.columns();
  auto d1c1 = c1[0];
  auto d1c2 = c1[1];
  auto d2c1 = c2[0];
  auto d2c2 = c2[1];
  auto s1 = c1; s1[0] = d2c2;
  auto s2 = s1; s2[0] = "encsrc"; s2[1] = d2c2;
  auto ret = df.bcast_join(tmp, eq(d1c1, d2c1))
               .select(s1)
               .rename(d2c2, "encsrc")
               .bcast_join(tmp, eq(d1c2, d2c1))
               .select(s2)
               .rename(d2c2, "encdst");
  std::cout << "encoded df: \n"; ret.show();
  //auto sorted_ret = ret.sort("encdst").sort("encsrc").materialize();
  //std::cout << "encoded df: \n"; sorted_ret.show();
  size_t precision = 6;
  std::string datetime_fmt = "%Y-%m-%d";
  std::string sep = " ";
  ret.savetext(o_fname, precision, datetime_fmt, sep);
}

int main(int argc, char* argv[]) {
  use_frovedis use(argc, argv);

  using namespace boost::program_options;
    
  options_description opt("option");
  opt.add_options()
      ("help,h", "produce help message")
      ("input,i" , value<std::string>(), "input edgelist file name") 
      ("output,o", value<std::string>(), "output edgelist file name") 
      ("delim,d", value<char>(), 
       "delimeter character - use ASCII for tab etc. (default: whitespace)") 
      ("with-weight", "whether input files contains weight (default: false)"); 
                
  variables_map argmap;
  store(command_line_parser(argc,argv).options(opt).allow_unregistered().
        run(), argmap);
  notify(argmap);                

  std::string input_p, output_p;
  char delim = ' ';
  bool withWeight = false;

  if(argmap.count("help")) {
    std::cerr << opt << std::endl;
    exit(1);
  }

  if(argmap.count("input")) {
    input_p = argmap["input"].as<std::string>();
  } else {
    std::cerr << "input path is not specified" << std::endl;
    std::cerr << opt << std::endl;
    exit(1);
  }    

  if(argmap.count("output")) {
    output_p = argmap["output"].as<std::string>();
  } else {
    std::cerr << "output path is not specified" << std::endl;
    std::cerr << opt << std::endl;
    exit(1);
  }
    
  if(argmap.count("delim")) {
    delim = argmap["delim"].as<char>();
  } 

  if(argmap.count("with-weight")) {
    withWeight = true;
  } 

  try {
    to_one_base(input_p, output_p, delim, withWeight);
  }
  catch(std::exception& e) {
    std::cout << "exception caught: " << e.what() << std::endl;
  }
  return 0;
}
