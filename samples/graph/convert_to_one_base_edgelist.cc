#include <frovedis.hpp>
#include <frovedis/dataframe.hpp>
#include <boost/program_options.hpp>

using namespace boost;
using namespace frovedis;
using namespace std;

void to_one_base(const std::string& i_fname,
                 const std::string& o_fname) {
  auto df = make_dftable_loadtext(i_fname, 
                                 {"unsigned long", "unsigned long"},
                                 {"src", "dst"}, ' ');
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

  auto ret = df.bcast_join(tmp, eq("src", "id"))
               .select({"src", "dst", "encoded_id"})
               .rename("encoded_id", "encoded_src")
               .bcast_join(tmp, eq("dst", "id"))
               .select({"src", "encoded_src", "dst", "encoded_id"})
               .rename("encoded_id", "encoded_dst")
               .select({"encoded_src", "encoded_dst"});
  std::cout << "encoded df: \n"; ret.show();
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
      ("output,o", value<std::string>(), "output edgelist file name"); 
                
  variables_map argmap;
  store(command_line_parser(argc,argv).options(opt).allow_unregistered().
        run(), argmap);
  notify(argmap);                

  std::string input_p, output_p;

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
    
  try {
    to_one_base(input_p, output_p);
  }
  catch(std::exception& e) {
    std::cout << "exception caught: " << e.what() << std::endl;
  }
  return 0;
}
