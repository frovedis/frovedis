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


}

#endif
