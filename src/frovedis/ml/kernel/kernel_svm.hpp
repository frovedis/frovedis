#ifndef _KERNEL_SVM_HPP_
#define _KERNEL_SVM_HPP_

#include "kernel_svm_model.hpp"


namespace frovedis {
template <typename K>
using kernel_csvc_model = kernel::csvc_model<K>;
}  // namespace frovedis

#endif  // _KERNEL_SVM_HPP_
