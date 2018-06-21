#ifndef _REGULARIZER_HPP_
#define _REGULARIZER_HPP_

namespace frovedis {

enum RegType {
  ZERO = 0,
  L1,
  L2
};

template <class T>
struct zero_regularizer {
  zero_regularizer(T regPrm = 0.01): regParam(regPrm) {}
  void regularize(std::vector<T> &weight) {}
  void debug_print() { std::cout << "zero_regularizer\n"; }
  T regParam;
  SERIALIZE(regParam)
};

template <class T> 
struct l1_regularizer {
  l1_regularizer(T regPrm = 0.01): regParam(regPrm) {}

  void regularize(std::vector<T> &weight) {
    T* weightp = &weight[0];
#pragma cdir on_adb(weightp)
    for(size_t i = 0; i < weight.size(); i++) {
      weightp[i] -= regParam * ((weightp[i]>0) - (weightp[i]<0));
    }
  }
  void debug_print() { std::cout << "l1_regularizer\n"; }

  T regParam;
  SERIALIZE(regParam)
};

template <class T>
struct l2_regularizer {
  l2_regularizer(T regPrm = 0.01): regParam(regPrm) {}

  void regularize(std::vector<T> &weight) {
    T* weightp = &weight[0];
#pragma cdir on_adb(weightp)
    for(size_t i = 0; i < weight.size(); i++) {
      weightp[i] -= regParam * weightp[i];
    }
  }
  void debug_print() { std::cout << "l2_regularizer\n"; }

  T regParam;
  SERIALIZE(regParam)
};

}
#endif
