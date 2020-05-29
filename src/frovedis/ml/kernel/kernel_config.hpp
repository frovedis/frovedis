#ifndef _KERNEL_CONFIG_HPP_
#define _KERNEL_CONFIG_HPP_


namespace frovedis {
namespace kernel {

enum kernel_function_type {
  LINEAR, POLY, RBF, SIGMOID, PRECOMPUTED
};

}  // namespace kernel
}  // namespace frovedis

#endif  // _KERNEL_CONFIG_HPP_
