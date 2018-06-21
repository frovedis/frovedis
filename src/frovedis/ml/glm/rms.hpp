#ifndef _RMS_HPP_
#define _RMS_HPP_

#include <iostream>
#include <cmath>
#include <algorithm>

namespace frovedis {

template <class T, class MODEL>
T get_rms(const MODEL& model) {
  bool isIntercept = (model.intercept == 0) ? false : true;

#ifdef _RMS_DEBUG_
  std::cout << "[get_rms] model: \n";
  model.debug_print();
  std::cout << "intermediate sqrs': ";
#endif

  T sum = 0;
  size_t numFeatures = model.get_num_features();
  for(size_t i=0; i<numFeatures; i++) {
     sum += model.weight[i] * model.weight[i];
#ifdef _RMS_DEBUG_
     std::cout << sum << " ";
#endif
  }
  
  if(isIntercept) {
    numFeatures += 1;
    sum += model.intercept * model.intercept;
#ifdef _RMS_DEBUG_
     std::cout << sum << " ";
#endif
  }

  T rms = sqrt(sum/numFeatures);
#ifdef _RMS_DEBUG_
     std::cout << "\n total rms: " << rms << std::endl;
#endif
  return rms;
}


template <class T, class MODEL>
T get_model_rms_error(const MODEL& m1, const MODEL& m2) {
  T err = get_rms<T,MODEL>(m1 - m2);
#ifdef _RMS_DEBUG_
     std::cout << "\n model-rms-error: " << err << std::endl;
#endif
  return err;
}

}

#endif
