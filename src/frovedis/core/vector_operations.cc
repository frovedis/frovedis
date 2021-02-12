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

// similar to numpy.sqrt(x) for integral x
std::vector<double>
integral_vector_sqrt(const std::vector<int>& vec) {
  auto vecsz = vec.size();
  auto vecp = vec.data();
  std::vector<double> ret(vecsz);
  auto retp = ret.data();
  for(size_t i = 0; i < vecsz; ++i) {
    retp[i] = (vecp[i] == 0) ? 0.0 : std::sqrt(vecp[i]);
  }
  return ret;
}

template <>
size_t vector_count_negatives(const std::vector<size_t>& vec) {
  return 0; // vector of size_t (unsigned) should have all elements >= 0
}

}
