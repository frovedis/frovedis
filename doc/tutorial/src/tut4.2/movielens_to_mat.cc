#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>

using namespace std;
using namespace boost;

int main(int argc, char* argv[]) {
  if(argc != 3) {
    cerr << "usage: " << argv[0] << " datafile matrix" << endl;
    exit(1);
  }

  ifstream ifs(argv[1]);
  ofstream matfs(argv[2]);
  string buf;
  size_t current_row = 1;
  while(ifs && getline(ifs, buf)) {
    vector<string> vs;
    boost::trim(buf);
    split(vs, buf, is_space(), boost::algorithm::token_compress_on);
    if(current_row != lexical_cast<size_t>(vs[0])) {
      matfs << "\n";
      current_row++;
    }
    matfs << lexical_cast<size_t>(vs[1]) - 1 << ":" << vs[2] << " ";
  }
}
