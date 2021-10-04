#ifndef _CONDITIONS_FOR_FIND_
#define _CONDITIONS_FOR_FIND_

#include <limits>
#include <math.h>

namespace frovedis {

template <class T>
struct is_zero {
  int operator()(T a) const {return (a == 0);}
};

template <class T>
struct is_nonzero {
  int operator()(T a) const {return (a != 0);}
};

template <class T>
struct is_one {
  int operator()(T a) const {return (a == 1);}
};

template <class T>
struct is_not_lf {
  int operator()(T a) const {return a != '\n';}
};

template <class T>
struct is_colon {
  int operator()(T a) const {return a == ':';}
};

template <class T>
struct is_positive {
  int operator()(T a) const {return (a >= 0);}
};

template <class T>
struct is_negative {
  int operator()(T a) const {return (a < 0);}
};

template <class T>
struct is_tmax {
  int operator()(T a) const {return (a == std::numeric_limits<T>::max());}
};

template <class T>
struct is_not_tmax {
  int operator()(T a) const {return (a != std::numeric_limits<T>::max());}
};

template <class T>
struct is_tmin {
  int operator()(T a) const {return (a == std::numeric_limits<T>::min());}
};

template <class T>
struct is_not_tmin {
  int operator()(T a) const {return (a != std::numeric_limits<T>::min());}
};

template <class T>
struct is_binary {
  int operator()(T a) const {return (a == 0 || a == 1);}
};

template <class T>
struct is_gt {
  T threshold;
  is_gt() {}
  is_gt(T thr): threshold(thr) {}
  int operator()(T a) const {return (a > threshold);}
};

template <class T>
struct is_gt_and_neq {
  T threshold, threshold2;
  is_gt_and_neq() {}
  is_gt_and_neq(T thr, T thr2): threshold(thr), threshold2(thr2) {}
  int operator()(T a) const {return (a > threshold && a != threshold2);}
};

template <class T>
struct is_ge {
  T threshold;
  is_ge() {}
  is_ge(T thr): threshold(thr) {}
  int operator()(T a) const {return (a >= threshold);}
};

template <class T>
struct is_ge_and_neq {
  T threshold, threshold2;
  is_ge_and_neq() {}
  is_ge_and_neq(T thr, T thr2): threshold(thr), threshold2(thr2) {}
  int operator()(T a) const {return (a >= threshold && a != threshold2);}
};

template <class T>
struct is_lt {
  T threshold;
  is_lt() {}
  is_lt(T thr): threshold(thr) {}
  int operator()(T a) const {return (a < threshold);}
};

template <class T>
struct is_lt_and_neq {
  T threshold, threshold2;
  is_lt_and_neq() {}
  is_lt_and_neq(T thr, T thr2): threshold(thr), threshold2(thr2) {}
  int operator()(T a) const {return (a < threshold && a != threshold2);}
};

template <class T>
struct is_le {
  T threshold;
  is_le() {}
  is_le(T thr): threshold(thr) {}
  int operator()(T a) const {return (a <= threshold);}
};

template <class T>
struct is_le_and_neq {
  T threshold, threshold2;
  is_le_and_neq() {}
  is_le_and_neq(T thr, T thr2): threshold(thr), threshold2(thr2) {}
  int operator()(T a) const {return (a <= threshold && a != threshold2);}
};

template <class T>
struct is_eq {
  T threshold;
  is_eq() {}
  is_eq(T thr): threshold(thr) {}
  int operator()(T a) const {return (a == threshold);}
};

template <class T>
struct is_neq {
  T threshold;
  is_neq() {}
  is_neq(T thr): threshold(thr) {}
  int operator()(T a) const {return (a != threshold);}
};

template <class T>
struct is_nan {
  int operator()(T a) const {return isnan(a);}
};

#ifdef __ve__
/*
 * Since isnan cannot be vectorized, specialized version is added.
 * This code causes "dereferencing type-punned pointer will break
 * strict-aliasing rules" warning in the case of g++, so ifdef'ed only for VE.
 * (using double/float instead of int does not work, since it is NaN)
 * */
template <>
struct is_nan<double> {
  is_nan() {
    double n = std::numeric_limits<double>::quiet_NaN();
    mynan = *reinterpret_cast<uint64_t*>(&n);
  }
  int operator()(double a) const {
    return (mynan == *reinterpret_cast<uint64_t*>(&a));
  }
  uint64_t mynan;
};

template <>
struct is_nan<float> {
  is_nan() {
    float n = std::numeric_limits<float>::quiet_NaN();
    mynan = *reinterpret_cast<uint32_t*>(&n);
  }
  int operator()(float a) const {
    return (mynan == *reinterpret_cast<uint32_t*>(&a));
  }
  uint32_t mynan;
};
#endif

template <class T>
struct is_inf {
  int operator()(T a) const {return (a == std::numeric_limits<T>::infinity());}
};

template <class T>
struct is_neg_inf {
  int operator()(T a) const {return (a == -std::numeric_limits<T>::infinity());}
};

template <class T>
struct is_even {
  int operator()(T a) const {return (a % 2 == 0);}
};

template <class T>
struct is_odd {
  int operator()(T a) const {return (a % 2 != 0);}
};

}
#endif
