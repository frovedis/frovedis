#ifndef UTILITY_HPP
#define UTILITY_HPP

#include "rlog.hpp"

namespace frovedis {

double get_dtime();

template <class T>
T ceil_div(T a, T b){
  if(a == 0) return 0;
  else return (a - 1) / b + 1;
}

template <class T>
T add(T a, T b){return a + b;}

class time_spent {
public:
  time_spent() : t0(get_dtime()), t1(0), lap_sum(0), loglevel(INFO) {}
  time_spent(log_level l) : t0(get_dtime()), t1(0), lap_sum(0), loglevel(l) {}
  void show(const std::string& mes) {
    t1 = get_dtime();
    RLOG(loglevel) << mes << t1 - t0 << " sec" << std::endl;
    /* Since printing itself takes some time on VE, changed to exclude it. */
    // t0 = t1; 
    t0 = get_dtime();
  }
  void reset(){t0 = get_dtime(); lap_sum = 0;}
  void lap_start(){t0 = get_dtime();}
  void lap_stop(){lap_sum += get_dtime() - t0;}
  double get_lap(){return lap_sum;}
  void show_lap(const std::string& mes){
    RLOG(loglevel) << mes << lap_sum << " sec" << std::endl;
  }
private:
  double t0, t1;
  double lap_sum;
  log_level loglevel;
};

void make_directory(const std::string&);
bool directory_exists(const std::string&);

}

#endif
