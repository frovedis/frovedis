#include "vector_operations.hpp"

namespace frovedis {

template <>
std::vector<int>
vector_divide(const std::vector<int>& vec,
              const int& by_elem) {
  auto vecsz = vec.size();
  if (by_elem == 0) {
    REPORT_WARNING(WARNING_MESSAGE,
        "RuntimeWarning: divide by zero encountered in divide");
    return vector_zeros<int>(vecsz);
  }
  std::vector<int> ret(vecsz);
  auto vecp = vec.data();
  auto retp = ret.data();
  for(size_t i = 0; i < vecsz; ++i) retp[i] = vecp[i] / by_elem;
  return ret;
}

template <>
int vector_squared_sum(const std::vector<int>& vec) {
  auto sz = vec.size();
  if (sz == 0) return 0;
  auto vptr = vec.data();
  int sqsum = 0;
  // might overflow here... 
  for(size_t i = 0; i < sz; ++i) sqsum += vptr[i] * vptr[i];
  return sqsum;
}

}
