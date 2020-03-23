#include "parseint.hpp"
#include "parsefloat.hpp"

namespace frovedis {

template <> std::vector<float> parsenumber<float>(const words& w) {
  return parsefloat<float>(w);
}
template <> std::vector<double> parsenumber<double>(const words& w) {
  return parsefloat<double>(w);
}
template <> std::vector<int> parsenumber<int>(const words& w) {
  return parseint<int>(w);
}
template <> std::vector<long> parsenumber<long>(const words& w) {
  return parseint<long>(w);
}
template <> std::vector<long long> parsenumber<long long>(const words& w) {
  return parseint<long long>(w);
}
template <> std::vector<unsigned int> 
parsenumber<unsigned int>(const words& w) {
  return parseint<unsigned int>(w);
}
template <> std::vector<unsigned long>
parsenumber<unsigned long>(const words& w) {
  return parseint<unsigned long>(w);
}
template <> std::vector<unsigned long long>
parsenumber<unsigned long long>(const words& w) {
  return parseint<unsigned long long>(w);
}

template <>
std::vector<float> parsenumber<float>(const std::vector<int>& chars,
                                      const std::vector<size_t>& starts,
                                      const std::vector<size_t>& lens) {
  return parsefloat<float>(chars, starts, lens);
}
template <>
std::vector<double> parsenumber<double>(const std::vector<int>& chars,
                                        const std::vector<size_t>& starts,
                                        const std::vector<size_t>& lens) {
  return parsefloat<double>(chars, starts, lens);
}
template <>
std::vector<int> parsenumber<int>(const std::vector<int>& chars,
                                  const std::vector<size_t>& starts,
                                  const std::vector<size_t>& lens) {
  return parseint<int>(chars, starts, lens);
}
template <>
std::vector<long> parsenumber<long>(const std::vector<int>& chars,
                                    const std::vector<size_t>& starts,
                                    const std::vector<size_t>& lens) {
  return parseint<long>(chars, starts, lens);
}
template <>
std::vector<long long> parsenumber<long long>(const std::vector<int>& chars,
                                              const std::vector<size_t>& starts,
                                              const std::vector<size_t>& lens) {
  return parseint<long long>(chars, starts, lens);
}
template <>
std::vector<unsigned int>
parsenumber<unsigned int>(const std::vector<int>& chars,
                          const std::vector<size_t>& starts,
                          const std::vector<size_t>& lens) {
  return parseint<unsigned int>(chars, starts, lens);
}
template <> std::vector<unsigned long>
parsenumber<unsigned long>(const std::vector<int>& chars,
                           const std::vector<size_t>& starts,
                           const std::vector<size_t>& lens) {
  return parseint<unsigned long>(chars, starts, lens);
}
template <> std::vector<unsigned long long>
parsenumber<unsigned long long>(const std::vector<int>& chars,
                                const std::vector<size_t>& starts,
                                const std::vector<size_t>& lens) {
  return parseint<unsigned long long>(chars, starts, lens);
}

}
