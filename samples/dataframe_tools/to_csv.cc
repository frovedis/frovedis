#include <frovedis.hpp>
#include <frovedis/dataframe.hpp>

#include <boost/program_options.hpp>
#include <boost/algorithm/string.hpp>

using namespace std;
using namespace frovedis;

void to_csv(string& input, string& output) {
  auto t = frovedis::make_dftable_load(input);
  t.savetext(output);
}

int main(int argc, char* argv[]) {
  use_frovedis use(argc, argv);

  using namespace boost::program_options;

  options_description opt("option");
  opt.add_options()
    ("help,h", "print help")
    ("input,i", value<string>(), "input binary dataframe directory")
    ("output,o", value<string>(), "output csv file");

  variables_map argmap;
  store(command_line_parser(argc,argv).options(opt).allow_unregistered().
        run(), argmap);
  notify(argmap);

  string input, output;
  
  if(argmap.count("help")){
    cerr << opt << endl;
    exit(1);
  }

  if(argmap.count("input")){
    input = argmap["input"].as<string>();
  } else {
    cerr << "input binary dataframe directory is not specified" << endl;
    cerr << opt << endl;
    exit(1);
  }

  if(argmap.count("output")){
    output = argmap["output"].as<string>();
  } else {
    cerr << "output csv file is not specified" << endl;
    cerr << opt << endl;
    exit(1);
  }

  to_csv(input, output);
}
