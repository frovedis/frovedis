#include <frovedis.hpp>
#include <frovedis/dataframe.hpp>

#include <boost/program_options.hpp>
#include <boost/algorithm/string.hpp>

using namespace std;
using namespace frovedis;

void to_binary_df(string& input, string& output,
                  vector<string>& columns, vector<string>& types) {
  auto t = frovedis::make_dftable_loadtext(input, types, columns);
  t.save(output);
}

int main(int argc, char* argv[]) {
  use_frovedis use(argc, argv);

  using namespace boost::program_options;

  options_description opt("option");
  opt.add_options()
    ("help,h", "print help")
    ("input,i", value<string>(), "input csv file")
    ("output,o", value<string>(), "output binary dataframe directory")
    ("columns,c", value<string>(), "comma separated column names")
    ("types,t", value<string>(), "comma separated type names");

  variables_map argmap;
  store(command_line_parser(argc,argv).options(opt).allow_unregistered().
        run(), argmap);
  notify(argmap);

  string input, output, columns, types;
  
  if(argmap.count("help")){
    cerr << opt << endl;
    exit(1);
  }

  if(argmap.count("input")){
    input = argmap["input"].as<string>();
  } else {
    cerr << "input csv file is not specified" << endl;
    cerr << opt << endl;
    exit(1);
  }

  if(argmap.count("output")){
    output = argmap["output"].as<string>();
  } else {
    cerr << "output matrix directory is not specified" << endl;
    cerr << opt << endl;
    exit(1);
  }

  if(argmap.count("columns")){
    columns = argmap["columns"].as<string>();
  } else {
    cerr << "column names are not specified" << endl;
    cerr << opt << endl;
    exit(1);
  }

  if(argmap.count("types")){
    types = argmap["types"].as<string>();
  } else {
    cerr << "type names are not specified" << endl;
    cerr << opt << endl;
    exit(1);
  }

  vector<string> columns_vec, types_vec;
  string delim (",");
  boost::split(columns_vec, columns, boost::is_any_of(delim));
  boost::split(types_vec, types, boost::is_any_of(delim));
  to_binary_df(input, output, columns_vec, types_vec);
}
