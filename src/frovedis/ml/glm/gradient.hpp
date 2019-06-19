#ifndef _GRADIENT_HPP_
#define _GRADIENT_HPP_

#include <iostream>
#include <vector>
#include <cmath>
#include "../../core/utility.hpp"

namespace frovedis {

template <class T>
class logistic_gradient {
public:
  std::vector<T>
  compute(const std::vector<T>& label,
          const std::vector<T>& wtx) {

    size_t workSize = wtx.size();
    std::vector<T> gradScalar(workSize,0);
    T* gradScalarp = &gradScalar[0];
    const T* wtxp = &wtx[0];
    const T* labelp = &label[0];

#pragma cdir on_adb(wtxp)
#pragma cdir on_adb(gradScalarp)
    for(size_t i = 0; i < workSize; i++) {
      T outLabel = labelp[i] * -1.0;
      auto tmp = outLabel * wtxp[i];
      // in the case of overflow, SX does not handle properly
      if(tmp > 709) gradScalarp[i] = outLabel;
      else gradScalarp[i] = outLabel * (1-1/(1+exp(tmp)));
//    gradScalarp[i] = outLabel * (1-1/(1+exp(outLabel * wtxp[i])));
    }
    return gradScalar;
  }
  SERIALIZE_NONE
};


template <class T>
class linear_gradient {
public:
  std::vector<T>
  compute(const std::vector<T>& label,
          const std::vector<T>& wtx) {

    size_t workSize = wtx.size();
    std::vector<T> gradScalar(workSize,0);
    T* gradScalarp = &gradScalar[0];
    const T* wtxp = &wtx[0];
    const T* labelp = &label[0];

#pragma cdir on_adb(wtxp)
#pragma cdir on_adb(gradScalarp)
    for(size_t i = 0; i < workSize; i++) {
      gradScalarp[i] = wtxp[i] - labelp[i];
    }
    return gradScalar;
  }
  SERIALIZE_NONE
};



template <class T>
class hinge_gradient {
public:
  std::vector<T>
  compute(const std::vector<T>& label,
          const std::vector<T>& wtx) {

    size_t workSize = wtx.size();
    std::vector<T> gradScalar(workSize,0);
    T* gradScalarp = &gradScalar[0];
    const T* wtxp = &wtx[0];
    const T* labelp = &label[0];

#pragma cdir on_adb(wtxp)
#pragma cdir on_adb(gradScalarp)
    for(size_t i = 0; i < workSize; i++) {
      gradScalarp[i] = (wtxp[i] * labelp[i]) < 1 ? -labelp[i] : 0;
    }
    return gradScalar;
  }
  SERIALIZE_NONE
};


}
#endif
