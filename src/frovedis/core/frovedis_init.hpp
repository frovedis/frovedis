#ifndef FROVEDIS_INIT_HPP
#define FROVEDIS_INIT_HPP

namespace frovedis {

void initfrovedis(int argc, char* argv[]);
void finalizefrovedis(int code = 0);

struct use_frovedis {
  use_frovedis(int argc, char* argv[]) {
    initfrovedis(argc, argv);
  }
  ~use_frovedis() {
    finalizefrovedis();
  }
  use_frovedis(const use_frovedis&) = delete;
  use_frovedis & operator=(const use_frovedis&) = delete;
};

}
#endif
