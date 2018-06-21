#include <frovedis.hpp>
#include <boost/lexical_cast.hpp>

using namespace std;
using namespace boost;

int parse_data(const string& s) {
  return lexical_cast<int>(s);
}

int two_times(int i) {return i*2;}

int main(int argc, char* argv[]){
  frovedis::use_frovedis use(argc, argv);

  auto d1 = frovedis::make_dvector_loadline("./data");
  auto d2 = d1.map(parse_data).map(two_times);
/*  
  auto d1 = frovedis::make_dvector_loadline<int>("./data");
  auto d2 = d1.map(two_times);
*/
  d2.saveline("./result");
}
