#include "crs_matrix.hpp"

namespace frovedis {

template <>
float strtox<float>(char* s, char** next) {
  return strtof(s, next);
}

template <>
double strtox<double>(char* s, char** next) {
  return strtod(s, next);
}

template <>
int strtox<int>(char* s, char** next) {
  return strtol(s, next, 10);
}

}
