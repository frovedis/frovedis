#include <frovedis.hpp>
#include <boost/lexical_cast.hpp>

using namespace std;
using namespace boost;

int two_times(int i) {return i*2;}

int main(int argc, char* argv[]){
  frovedis::use_frovedis use(argc, argv);
  
  auto d1 = frovedis::make_dvector_loadbinary<int>("./data_binary");
  auto d2 = d1.map(two_times);
  d2.savebinary("./result_binary");
}
