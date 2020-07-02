#ifndef _GRADIENT_HPP_
#define _GRADIENT_HPP_

#include <iostream>
#include <vector>
#include <cmath>

#define MAX_LOSS 1e12
#define MIN_LOSS -1e12
#define HINGE_THRESHOLD 1.0

namespace frovedis {

template <class T>
struct logistic_gradient {
  std::vector<T>
  compute_dloss(const std::vector<T>& label,
                const std::vector<T>& wtx,
                double& sumloss) {
    size_t workSize = wtx.size();
    auto wtxp = wtx.data();
    auto labelp = label.data();

    // computing total loss
    double loss = 0.0;
#pragma cdir on_adb(wtxp)
    for(size_t i = 0; i < workSize; i++) {
      auto z = labelp[i] * wtxp[i];
      if (z > 18.0) loss = exp(-z);
      else if (z < -18.0) loss = -z;
      else loss = log(1.0 + exp(-z));
      sumloss += loss;
    }

    // computing derivative of loss
    std::vector<T> dloss(workSize,0);
    auto dlossp = dloss.data();
#pragma cdir on_adb(wtxp)
#pragma cdir on_adb(lossp)
    for(size_t i = 0; i < workSize; i++) {
      auto neg_label = labelp[i] * -1.0;
      auto z = labelp[i] * wtxp[i];
      if (z > 18.0) dlossp[i] = exp(-z) * neg_label;
      else if (z < -18.0) dlossp[i] = neg_label;
      else dlossp[i] = neg_label * (1.0 / (1.0 + exp(z)));
      // clip derivative of loss with large values to avoid numerical instabilities
      if(dlossp[i] > MAX_LOSS) dlossp[i] = MAX_LOSS;
      else if(dlossp[i] < MIN_LOSS) dlossp[i] = MIN_LOSS;
    }
    return dloss;
  }
  SERIALIZE_NONE
};


template <class T>
struct linear_gradient {
  std::vector<T>
  compute_dloss(const std::vector<T>& label,
                const std::vector<T>& wtx,
                double& sumloss) {
    size_t workSize = wtx.size();
    auto wtxp = wtx.data();
    auto labelp = label.data();

    // computing total loss
#pragma cdir on_adb(wtxp)
    for(size_t i = 0; i < workSize; i++) {
      auto error = wtxp[i] - labelp[i];
      sumloss += (error * error);
    }
    sumloss *= 0.5; // squared loss

    // computing derivative of loss
    std::vector<T> dloss(workSize);
    auto dlossp = dloss.data();
#pragma cdir on_adb(wtxp)
#pragma cdir on_adb(lossp)
    for(size_t i = 0; i < workSize; i++) {
      dlossp[i] = wtxp[i] - labelp[i]; 
      // clip derivative of loss with large values to avoid numerical instabilities
      if(dlossp[i] > MAX_LOSS) dlossp[i] = MAX_LOSS;
      else if(dlossp[i] < MIN_LOSS) dlossp[i] = MIN_LOSS;
    }
    return dloss;
  }
  SERIALIZE_NONE
};

template <class T>
struct hinge_gradient {
  std::vector<T>
  compute_dloss(const std::vector<T>& label,
                const std::vector<T>& wtx,
                double& sumloss) {
    size_t workSize = wtx.size();
    auto wtxp = wtx.data();
    auto labelp = label.data();

    // computing total loss
    double loss = 0.0;
#pragma cdir on_adb(wtxp)
    for(size_t i = 0; i < workSize; i++) {
      auto z = wtxp[i] * labelp[i];
      if(z <= HINGE_THRESHOLD)  loss = HINGE_THRESHOLD - z; 
      else                      loss = 0.0;
      sumloss += loss;
    }

    // computing derivative of loss
    std::vector<T> dloss(workSize);
    auto dlossp = dloss.data();
#pragma cdir on_adb(wtxp)
#pragma cdir on_adb(lossp)
    for(size_t i = 0; i < workSize; i++) {
      auto z = wtxp[i] * labelp[i];
      if(z <= HINGE_THRESHOLD) dlossp[i] = -labelp[i]; 
      else                     dlossp[i] = 0.0;
    }
    return dloss;
  }
  SERIALIZE_NONE
};

template <class T>
struct epsilon_insensitive {
  epsilon_insensitive(): epsilon(0.0) {}
  epsilon_insensitive(double eps): epsilon(eps) {}
  std::vector<T>
  compute_dloss(const std::vector<T>& label,
                const std::vector<T>& wtx,
                double& sumloss) {
    size_t workSize = wtx.size();
    auto wtxp = wtx.data();
    auto labelp = label.data();
    // computing total loss
    double loss = 0.0;
#pragma cdir on_adb(wtxp)
    for(size_t i = 0; i < workSize; i++) {
      // TODO: replace fabs() from loop
      auto error = fabs(labelp[i] - wtxp[i]) - epsilon;
      if(error > 0) loss = error;
      else loss = 0;
      sumloss += loss;
    }

    // computing derivative of loss
    std::vector<T> dloss(workSize);
    auto dlossp = dloss.data();
#pragma cdir on_adb(wtxp)
#pragma cdir on_adb(lossp)
    for(size_t i = 0; i < workSize; i++) {
      if((labelp[i] - wtxp[i]) > epsilon) dlossp[i] = -1;
      else if((wtxp[i] - labelp[i]) > epsilon) dlossp[i] = 1;
      else dlossp[i] = 0;
    }
    return dloss;
  }
  double epsilon;
  SERIALIZE(epsilon)
};

template <class T>
struct squared_epsilon_insensitive {
  squared_epsilon_insensitive(): epsilon(0.0) {}
  squared_epsilon_insensitive(double eps): epsilon(eps) {}
  std::vector<T>
  compute_dloss(const std::vector<T>& label,
                const std::vector<T>& wtx,
                double& sumloss) {
    size_t workSize = wtx.size();
    auto wtxp = wtx.data();
    auto labelp = label.data();

    // computing total loss
    double loss = 0.0;
#pragma cdir on_adb(wtxp)
    for(size_t i = 0; i < workSize; i++) {
      auto error = fabs(labelp[i] - wtxp[i]) - epsilon;
      if(error > 0) loss = error * error;
      else loss = 0;
      sumloss += loss;
    }

    // computing derivative of loss
    std::vector<T> dloss(workSize);
    auto dlossp = dloss.data();
#pragma cdir on_adb(wtxp)
#pragma cdir on_adb(lossp)
    for(size_t i = 0; i < workSize; i++) {
      auto z = labelp[i] - wtxp[i];
      if(z > epsilon) dlossp[i] = -2 * (z - epsilon);
      else if(z < -epsilon) dlossp[i] = 2 * (-z - epsilon);
      else dlossp[i] = 0;
      // clip derivative of loss with large values to avoid numerical instabilities
      if(dlossp[i] > MAX_LOSS) dlossp[i] = MAX_LOSS;
      else if(dlossp[i] < MIN_LOSS) dlossp[i] = MIN_LOSS;
    }
    return dloss;
  }
  double epsilon;
  SERIALIZE(epsilon)
};



}
#endif
