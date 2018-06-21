#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <boost/algorithm/string.hpp>

using namespace std;
using namespace boost;

int main(int argc, char* argv[]) {
  if(argc != 4) {
    cerr << "usage: " << argv[0] << " datafile matrix label" << endl;
    exit(1);
  }

  ifstream ifs(argv[1]);
  ofstream matfs(argv[2]);
  ofstream labelfs(argv[3]);
  string buf;
  while(ifs && getline(ifs, buf)) {
    vector<string> vs;
    split(vs, buf, is_any_of(","));
    for(size_t i = 0; i < vs.size() - 1; i++) {
      matfs << i << ":" << vs[i];
      if(i != vs.size() - 2) matfs << " ";
    }
    matfs << "\n";
    if(vs[vs.size() - 1] == "Iris-setosa") labelfs << 1 << "\n";
    else labelfs << -1 << "\n";
  }
}
