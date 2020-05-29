#ifndef _KERNEL_SVM_UTILITY_HPP_
#define _KERNEL_SVM_UTILITY_HPP_

#include "kernel_config.hpp"


namespace frovedis {
namespace kernel {

inline bool is_I_up(double a, double y, double Cp, double Cn) {
  return (y > 0 && a < Cp) || (y < 0 && a > 0);
}

inline bool is_I_low(double a, double y, double Cp, double Cn) {
  return (y > 0 && a > 0) || (y < 0 && a < Cn);
}

inline bool is_free(double a, double y, double Cp, double Cn) {
  return a > 0 && (y > 0 ? a < Cp : a < Cn);
}

inline bool is_I_up_bound(double a, double y, double Cp, double Cn) {
	return (y > 0 && a == 0) || (y < 0 && a == Cn);
}

inline bool is_I_low_bound(double a, double y, double Cp, double Cn) {
	return (y > 0 && a == Cp) || (y < 0 && a == 0);
}

}  // namespace kernel
}  // namespace frovedis

#endif  // _KERNEL_SVM_UTILITY_HPP_
