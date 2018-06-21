#include "tree_random.hpp"

#if defined(_SX) || defined(__ve__)

float* franv_area() {
  static float foobar[1024 / sizeof(float)];
  return foobar;
}

#ifndef __ve__
// SX-ACE
void init_random(const size_t seed) {
  static size_t n = 0;
  if (++n > 1) { return; }

  random(seed);

  float* area = franv_area();
  franvi(area);
  for (size_t i = 0; i < seed; i++) {
    franv(area);
  }
}
#else
// SX-Aurora TSUBASA
void init_random(const size_t) {
  // TODO: implement
}
#endif

#endif
