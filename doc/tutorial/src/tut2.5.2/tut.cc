#include <frovedis.hpp>
#include <boost/algorithm/string.hpp>

std::vector<std::string> splitter(const std::string& line) {
  std::vector<std::string> wordvec;
  boost::split(wordvec, line, boost::is_space());
  return wordvec;
}

std::pair<std::string, int> counter(const std::string& v) {
  return std::make_pair(v, 1);
}

int sum(int a, int b){return a + b;}

int main(int argc, char* argv[]){
  frovedis::use_frovedis use(argc, argv);
  
  auto d1 = frovedis::make_dvector_loadline("./sample.txt");
  auto d2 = d1.flat_map(splitter).map(counter).reduce_by_key<std::string,int>(sum);
  auto r = d2.as_dvector().gather();
  for(auto& i: r) std::cout << i.first << ": " << i.second << std::endl;
}
