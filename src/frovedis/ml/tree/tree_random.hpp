#ifndef _TREE_RANDOM_HPP_
#define _TREE_RANDOM_HPP_

#include <cstddef>

#if defined(_SX) || defined(__ve__)
#ifndef __ve__
// SX-ACE
#include <random.h>
#else
// SX-Aurora TSUBASA
#endif

float* franv_area();
void init_random(const size_t);

#else

// x86
#include <random>

#endif

#endif
