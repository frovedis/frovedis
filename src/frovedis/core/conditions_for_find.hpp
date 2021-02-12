#ifndef _CONDITIONS_FOR_FIND_
#define _CONDITIONS_FOR_FIND_

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

}

#endif
