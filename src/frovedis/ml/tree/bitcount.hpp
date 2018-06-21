#ifndef _BITCOUNT_HPP_
#define _BITCOUNT_HPP_

#include <climits>
#include <cstdint>

namespace frovedis {

// a constexpr recursive popcount function
template <typename T>
constexpr T CX_POPCOUNT(const T x) {
  return (x == 0) ? 0 : (x & 1) + CX_POPCOUNT(x >> 1);
}

// 0xAB -> 0xABABABAB...
template <typename T>
constexpr T fill(const T byte, const T work = 0) {
  return (byte > 0) ? (
    fill(byte << CHAR_BIT, byte | work)
  ) : (
    work
  );
}

// Gerd Isenberg's popcount algorithm
template <typename T>
inline T popcount(T x) {
  constexpr T __0x55__ = fill<T>(0x55);
  constexpr T __0x33__ = fill<T>(0x33);
  constexpr T __0x0f__ = fill<T>(0x0f);
  constexpr T __0x01__ = fill<T>(0x01);
  constexpr size_t RSHIFT = CHAR_BIT * (sizeof(T) - 1);

  x = x - ((x >> 1) & __0x55__);
  x = (x & __0x33__) + ((x >> 2) & __0x33__);
  x = (x + (x >> 4)) & __0x0f__;
  return (x * __0x01__) >> RSHIFT;
}

// count the number of trailing zeros
template <typename T>
inline T ntzcount(const T x) {
  return popcount((~x) & (x - 1));
}

// only the highest bit is one (from Hacker's Delight)
template <typename T>
inline T hibit(T x) {
  x |= (x >> 1);
  x |= (x >> 2);
  x |= (x >> 4);
  x |= (x >> 8);
  x |= (x >> 16);
  x |= (x >> 32);
  return x - (x >> 1);
}

} // end namespace frovedis

#endif
