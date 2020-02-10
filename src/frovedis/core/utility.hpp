#ifndef UTILITY_HPP
#define UTILITY_HPP

#include "rlog.hpp"
#include <cmath>
#include <limits>

#define SIZEOF_INT8 1
#define SIZEOF_INT16 2
#define SIZEOF_INT32 4
#define SIZEOF_INT64 8
#define SIZEOF_UINT8 1
#define SIZEOF_UINT16 2
#define SIZEOF_UINT32 4
#define SIZEOF_UINT64 8
#define SIZEOF_FLOAT32 4
#define SIZEOF_FLOAT64 8

namespace frovedis {

double get_dtime();

template <class T>
inline
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

template <class T>
std::string get_type_name() {
  if (std::numeric_limits<T>::is_integer) {
    if(std::numeric_limits<T>::is_signed) {
      if (sizeof(T) == SIZEOF_INT8) return "int8_t";
      else if (sizeof(T) == SIZEOF_INT16) return "int16_t";
      else if (sizeof(T) == SIZEOF_INT32) return "int32_t";
      else if (sizeof(T) == SIZEOF_INT64) return "int64_t";
      else throw std::runtime_error("get_type_name: Unknown signed integer type!\n");
    }
    else {
      if (sizeof(T) == SIZEOF_UINT8) return "uint8_t";
      else if (sizeof(T) == SIZEOF_UINT16) return "uint16_t";
      else if (sizeof(T) == SIZEOF_UINT32) return "uint32_t";
      else if (sizeof(T) == SIZEOF_UINT64) return "uint64_t";
      else throw std::runtime_error("get_type_name: Unknown unsigned integer type!\n");
    }
  }
  else {
    if (sizeof(T) == SIZEOF_FLOAT32) return "float";
    else if (sizeof(T) == SIZEOF_FLOAT64) return "double";
    else throw std::runtime_error("get_type_name: Unknown non-integer type!\n");
  }
}

template <class T>
bool confirm_given_type_against_expected(const std::string& e_tname,
                                         const std::string& fname,
                                         const std::string& target) {
  auto g_tname = get_type_name<T>();
  if (g_tname != e_tname) {
    std::string errmsg = fname + ": " + target + " type mismatch detected! ";
    errmsg += "expected type: " + e_tname;
    errmsg += "; given type: "  + g_tname + "\n";
    throw std::runtime_error(errmsg);
  }
  else return true;
}

// temporary; to improve vectorization
#ifdef __ve__
inline double myexp(double _Left) {
  return (__builtin_exp(_Left));
}
inline float myexp(float _Left) {
  return (__builtin_expf(_Left));
}
#else
inline double myexp(double _Left) {
  return std::exp(_Left);
}
inline float myexp(float _Left) {
  return std::exp(_Left);
}
#endif


}

#endif
