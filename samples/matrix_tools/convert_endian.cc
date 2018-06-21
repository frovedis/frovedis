#include <frovedis.hpp>

#include <boost/program_options.hpp>
#include <boost/lexical_cast.hpp>

using namespace std;
using namespace frovedis;

inline uint32_t swap32(uint32_t val) {
  return ( (((val) >> 24) & 0x000000FF) | (((val) >>  8) & 0x0000FF00) | 
           (((val) <<  8) & 0x00FF0000) | (((val) << 24) & 0xFF000000) );
}

inline uint64_t swap64(uint64_t val) {
  return ( (((val) >> 56) & 0x00000000000000FF) |
           (((val) >> 40) & 0x000000000000FF00) |
           (((val) >> 24) & 0x0000000000FF0000) |
           (((val) >>  8) & 0x00000000FF000000) |
           (((val) <<  8) & 0x000000FF00000000) |
           (((val) << 24) & 0x0000FF0000000000) |
           (((val) << 40) & 0x00FF000000000000) |
           (((val) << 56) & 0xFF00000000000000) );
}

std::vector<uint32_t>
endian_conversion_helper_32(std::vector<uint32_t>& vec) {
  std::vector<uint32_t> ret(vec.size());
#if defined(_SX) && !defined(__ve__)
  void* vecp = reinterpret_cast<void*>(&vec[0]);
  void* retp = reinterpret_cast<void*>(&ret[0]);
  bswap_memcpy(retp, vecp, 4, vec.size());
#else
  for(size_t i = 0; i < vec.size(); i++) {
    ret[i] = swap32(vec[i]);
  }
#endif
  return ret;
}

std::vector<uint64_t>
endian_conversion_helper_64(std::vector<uint64_t>& vec) {
  std::vector<uint64_t> ret(vec.size());
#if defined(_SX) && !defined(__ve__)
  void* vecp = reinterpret_cast<void*>(&vec[0]);
  void* retp = reinterpret_cast<void*>(&ret[0]);
  bswap_memcpy(retp, vecp, 8, vec.size());
#else
  for(size_t i = 0; i < vec.size(); i++) {
    ret[i] = swap64(vec[i]);
  }
#endif
  return ret;
}

void endian_conversion(string& input, string& output, int elmsize) {
  if(elmsize == 4) {
    make_dvector_loadbinary<uint32_t>(input).
      viewas_node_local().
      map(endian_conversion_helper_32).
      viewas_dvector<uint32_t>().
      savebinary(output);
  } else if (elmsize == 8) {
    make_dvector_loadbinary<uint64_t>(input).
      viewas_node_local().
      map(endian_conversion_helper_64).
      viewas_dvector<uint64_t>().
      savebinary(output);
  }
}

int main(int argc, char* argv[]) {
  use_frovedis use(argc, argv);

  using namespace boost::program_options;

  options_description opt("option");
  opt.add_options()
    ("help,h", "print help")
    ("input,i", value<string>(), "input binary file")
    ("output,o", value<string>(), "output binary file")
    ("size,s", value<int>(), "size of each element: 4 or 8");

  variables_map argmap;
  store(command_line_parser(argc,argv).options(opt).allow_unregistered().
        run(), argmap);
  notify(argmap);

  string input, output;
  int elmsize;
  
  if(argmap.count("help")){
    cerr << opt << endl;
    exit(1);
  }

  if(argmap.count("input")){
    input = argmap["input"].as<string>();
  } else {
    cerr << "input binary file is not specified" << endl;
    cerr << opt << endl;
    exit(1);
  }

  if(argmap.count("output")){
    output = argmap["output"].as<string>();
  } else {
    cerr << "output binary file is not specified" << endl;
    cerr << opt << endl;
    exit(1);
  }

  if(argmap.count("size")){
    elmsize = argmap["size"].as<int>();
  } else {
    cerr << "element size is not specified" << endl;
    cerr << opt << endl;
    exit(1);
  }

  if(elmsize != 4 && elmsize != 8) {
    cerr << "element size should be 4 or 8" << endl;
    cerr << opt << endl;
    exit(1);
  }

  endian_conversion(input, output, elmsize);
}
