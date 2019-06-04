#ifndef ____RANDOM_HPP____
#define ____RANDOM_HPP____

#include <algorithm>
#include <cstring>
#include <fstream>
#include <ios>
#include <limits>
#include <random>
#include <sstream>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#ifdef __ve__
#include <asl.h>
#endif

#include "../../../frovedis.hpp"
#include "../../core/type_utility.hpp"
#include "../../core/radix_sort.hpp"

#ifdef ___RANDOM_DEBUG___
#include <cassert>
#define ___myassert___(expr) assert(expr)
#else
#define ___myassert___(ignore) ((void) 0)
#endif

namespace frovedis {

template <typename T>
struct always_false : public std::false_type {};
template <typename T, typename U>
using enable_if_same_t = enable_if_t<std::is_same<T, U>::value>;
template <typename T, typename U>
using enable_if_differ_t = enable_if_t<!std::is_same<T, U>::value>;

// ---------------------------------------------------------------------

#ifdef __ve__
template <typename U = asl_uint32_t>
class vh_random_device {
  std::string name;
  std::ifstream f;

  union {
    U value;
    typename decltype(f)::char_type bytes[sizeof(U)];
  } u;

public:
  using result_type = U;

  explicit vh_random_device(const std::string& token = "/dev/urandom") :
    name(token)
  {
    try {
      f.exceptions(std::ios::badbit | std::ios::failbit);
      f.open(name, std::ios::in);
    } catch (const std::ios::failure& e) {
      std::ostringstream sout;
      sout << "failed to open '" << name << "' (" << e.what() << ")";
      throw std::runtime_error(sout.str());
    }
  }

  vh_random_device(const vh_random_device&) = delete;
  vh_random_device(vh_random_device&&) = delete;
  // ~vh_random_device() = default;

  void operator=(const vh_random_device&) = delete;
  void operator=(vh_random_device&&) = delete;

  result_type operator()() {
    try {
      f.read(u.bytes, sizeof(u.bytes));
    } catch (const std::ios::failure& e) {
      std::ostringstream sout;
      sout << "failed to read '" << name << "' (" << e.what() << ")";
      throw std::runtime_error(sout.str());
    }

    return u.value;
  }

  double entropy() const { return 0.0; }

  static constexpr
  result_type min() { return std::numeric_limits<result_type>::min(); }
  static constexpr
  result_type max() { return std::numeric_limits<result_type>::max(); }
};

using random_device = vh_random_device<>;
#else
using random_device = std::random_device;
#endif

// ---------------------------------------------------------------------

#ifdef __ve__
namespace aslutil {

inline void checkerror(const asl_error_t status, const std::string& msg) {
  if (status != ASL_ERROR_OK) {
    std::ostringstream sout;
    sout << msg << ": 0x" << std::hex << status;
    const std::string errmsg(sout.str());
    RLOG(ERROR) << errmsg << std::endl;
    throw std::runtime_error(errmsg);
  }
}

inline void initlib() {
  if (!asl_library_is_initialized()) {
    checkerror(asl_library_initialize(), "failed to initialize ASL");
  }
}

template <typename I>
struct another_int_s;
template <>
struct another_int_s<asl_int32_t> { using type = asl_int64_t; };
template <>
struct another_int_s<asl_int64_t> { using type = asl_int32_t; };

template <typename U>
struct another_uint_s;
template <>
struct another_uint_s<asl_uint32_t> { using type = asl_uint64_t; };
template <>
struct another_uint_s<asl_uint64_t> { using type = asl_uint32_t; };

using another_int_t = typename another_int_s<asl_int_t>::type;
using another_uint_t = typename another_uint_s<asl_uint_t>::type;

template <typename I>
struct is_asl_signed_int {
  static constexpr bool value = (
    std::is_same<I, asl_int32_t>::value ||
    std::is_same<I, asl_int64_t>::value
  );
};

template <typename U>
struct is_asl_unsigned_int {
  static constexpr bool value = (
    std::is_same<U, asl_uint32_t>::value ||
    std::is_same<U, asl_uint64_t>::value
  );
};

template <typename T>
struct is_asl_int {
  static constexpr bool value = (
    is_asl_signed_int<T>::value || is_asl_unsigned_int<T>::value
  );
};

template <typename T>
struct is_asl_floating_point {
  static constexpr bool value = (
    std::is_same<T, float>::value || std::is_same<T, double>::value
  );
};

template <typename U>
using is_seedable = is_asl_unsigned_int<U>;
template <typename U>
using enable_if_seedable_t = enable_if_t<is_seedable<U>::value>;
template <typename U>
using enable_if_unseedable_t = enable_if_t<!is_seedable<U>::value>;

template <typename T>
using is_randombitable = is_asl_int<T>;
template <typename T>
using enable_if_randombitable_t = enable_if_t<is_randombitable<T>::value>;
template <typename T>
using enable_if_unrandombitable_t = enable_if_t<!is_randombitable<T>::value>;

template <typename T>
struct is_generateable {
  static constexpr bool value = (
    is_asl_floating_point<T>::value || is_asl_signed_int<T>::value
  );
};
template <typename T>
using enable_if_generateable_t = enable_if_t<is_generateable<T>::value>;
template <typename T>
using enable_if_ungenerateable_t = enable_if_t<!is_generateable<T>::value>;

inline void initializehelper(
  asl_random_t& handle, const size_t num, const asl_uint32_t* src
) {
  checkerror(
    asl_random_initialize(handle, static_cast<asl_int_t>(num), src),
    "failed to initialize ASL random handle"
  );
}

template <typename U>
inline void seed(asl_random_t&, const U) {
  static_assert(
    always_false<U>::value,
    "ERROR: the specified type U is not compatible"
  );
}

template <>
inline void seed<asl_uint32_t>(
  asl_random_t& handle, const asl_uint32_t value
) {
  initializehelper(handle, 1, &value);
}

template <>
inline void seed<asl_uint64_t>(
  asl_random_t& handle, const asl_uint64_t value
) {
  seed<asl_uint32_t>(handle, static_cast<asl_uint32_t>(value));
}

template <typename U>
inline void seeds(asl_random_t&, const size_t, const U*) {
  static_assert(
    always_false<U>::value,
    "ERROR: the specified type U is not compatible"
  );
}

template <>
inline void seeds<asl_uint32_t>(
  asl_random_t& handle, const size_t num, const asl_uint32_t* src
) {
  initializehelper(handle, num, src);
}

template <>
inline void seeds<asl_uint64_t>(
  asl_random_t& handle, const size_t num, const asl_uint64_t* src
) {
  std::vector<asl_uint32_t> vec(num, 0);
  asl_uint32_t* tmp = vec.data();
  for (size_t i = 0; i < num; i++) {
    tmp[i] = static_cast<asl_uint32_t>(src[i]);
  }
  seeds<asl_uint32_t>(handle, num, tmp);
}

template <typename ResultType, typename GenerateFunc>
inline void generatehelper(
  const GenerateFunc& func,
  asl_random_t& handle, const size_t num, ResultType* dest,
  const std::string& msg
) {
  checkerror(func(handle, static_cast<asl_int_t>(num), dest), msg);
}

template <typename T>
inline void randombits(asl_random_t&, const size_t, T*) {
  static_assert(
    always_false<T>::value,
    "ERROR: the specified type T is not compatible"
  );
}

template <>
inline void randombits<asl_uint32_t>(
  asl_random_t& handle, const size_t num, asl_uint32_t* dest
) {
  generatehelper(
    asl_random_generate_uniform_bits, handle, num, dest,
    "failed to generate ASL random bits"
  );
}

template <>
inline void randombits<asl_uint64_t>(
  asl_random_t& handle, const size_t num, asl_uint64_t* dest
) {
  generatehelper(
    asl_random_generate_uniform_long_bits, handle, num, dest,
    "failed to generate ASL random long bits"
  );
}

template <>
inline void randombits<asl_int32_t>(
  asl_random_t& handle, const size_t num, asl_int32_t* dest
) {
  randombits<asl_uint32_t>(
    handle, num, reinterpret_cast<asl_uint32_t*>(dest)
  );
}

template <>
inline void randombits<asl_int64_t>(
  asl_random_t& handle, const size_t num, asl_int64_t* dest
) {
  randombits<asl_uint64_t>(
    handle, num, reinterpret_cast<asl_uint64_t*>(dest)
  );
}

/*
template <>
inline void randombits<float>(
  asl_random_t& handle, const size_t num,
  typename std::enable_if<
    sizeof(float) == sizeof(asl_uint32_t), float*
  >::type dest
) {
  randombits<asl_uint32_t>(
    handle, num, reinterpret_cast<asl_uint32_t*>(dest)
  );
}

template <>
inline void randombits<double>(
  asl_random_t& handle, const size_t num,
  typename std::enable_if<
    sizeof(double) == sizeof(asl_uint64_t), double*
  >::type dest
) {
  randombits<asl_uint64_t>(
    handle, num, reinterpret_cast<asl_uint64_t*>(dest)
  );
}
*/

template <typename T>
inline void generate(asl_random_t&, const size_t, T*) {
  static_assert(
    always_false<T>::value,
    "ERROR: the specified type T is not compatible"
  );
}

template <>
inline void generate<float>(
  asl_random_t& handle, const size_t num, float* dest
) {
  generatehelper(
    asl_random_generate_s, handle, num, dest,
    "failed to generate ASL random float numbers"
  );
}

template <>
inline void generate<double>(
  asl_random_t& handle, const size_t num, double* dest
) {
  generatehelper(
    asl_random_generate_d, handle, num, dest,
    "failed to generate ASL random double numbers"
  );
}

template <>
inline void generate<asl_int_t>(
  asl_random_t& handle, const size_t num, asl_int_t* dest
) {
  generatehelper(
    asl_random_generate_i, handle, num, dest,
    "failed to generate ASL random integer numbers"
  );
}

template <>
inline void generate<another_int_t>(
  asl_random_t& handle, const size_t num, another_int_t* dest
) {
  std::vector<asl_int_t> vec(num, 0);
  asl_int_t* tmp = vec.data();
  generate<asl_int_t>(handle, num, tmp);
  for (size_t i = 0; i < num; i++) {
    dest[i] = static_cast<another_int_t>(tmp[i]);
  }
}

template <asl_randommethod_t Method>
constexpr asl_uint32_t default_seed_for() {
  static_assert(
    Method == ASL_RANDOMMETHOD_MT19937 ||
    Method == ASL_RANDOMMETHOD_MT19937_64,
    "ERROR: the specified ASL random method is not supported"
  );
}

template <>
constexpr asl_uint32_t default_seed_for<ASL_RANDOMMETHOD_MT19937>() {
  return std::mt19937::default_seed;
}

template <>
constexpr asl_uint32_t default_seed_for<ASL_RANDOMMETHOD_MT19937_64>() {
  return std::mt19937_64::default_seed;
}

} // end namespace aslutil
#endif

// ---------------------------------------------------------------------

template <typename Engine>
struct std_random_wrapper {
  Engine engine;
  bool released = false;

public:
  using kernel_type = Engine;
  using result_type = typename Engine::result_type;
  static constexpr result_type default_seed = Engine::default_seed;

  explicit std_random_wrapper(const result_type value = default_seed) :
    engine(value)
  {}

  template <typename SeedType>
  explicit std_random_wrapper(const std::vector<SeedType>& vec) :
    engine()
  {
    seed(vec);
  }

  std_random_wrapper(const std_random_wrapper<Engine>& src) :
    engine(src.engine), released(src.released)
  {}

  std_random_wrapper(std_random_wrapper<Engine>&& src) :
    engine(std::move(src.engine)), released(src.released)
  {
    src.released = true;
  }

  std_random_wrapper<Engine>&
  operator=(const std_random_wrapper<Engine>& src) {
    _destroy();
    engine = src.engine;
    released = src.released;
    return *this;
  }

  std_random_wrapper<Engine>&
  operator=(std_random_wrapper<Engine>&& src) {
    _destroy();
    engine = std::move(src.engine);
    released = src.released;
    src.released = true;
    return *this;
  }

  ~std_random_wrapper() { _destroy(); }

  void seed(const result_type value = default_seed) { engine.seed(value); }

  template <typename SeedType>
  void seed(const std::vector<SeedType>& vec) {
    std::seed_seq seq(vec.cbegin(), vec.cend());
    engine.seed(seq);
  }

  result_type operator()() { return generate<result_type>(); }

  void operator()(const size_t num, result_type* dest) {
    generate<result_type>(num, dest);
  }

  std::vector<result_type> operator()(const size_t num) {
    return generate<result_type>(num);
  }

  template <typename R>
  R generate() { return static_cast<R>(engine()); }

  template <typename R>
  void generate(const size_t num, R* dest) {
    for (size_t i = 0; i < num; i++) { dest[i] = generate<R>(); }
  }

  template <typename R>
  std::vector<R> generate(const size_t num) {
    std::vector<R> ret(num, 0);
    generate<R>(num, ret.data());
    return ret;
  }

  void discard(const size_t num) { engine.discard(num); }

  static constexpr result_type min() { return Engine::min(); }
  static constexpr result_type max() { return Engine::max(); }

  kernel_type& refer_kernel() { return engine; }

  kernel_type release_kernel() {
    released = true;
    return std::move(engine);
  }

  bool is_released() const { return released; }

private:
  void _destroy() { released = true; }
};

#ifdef __ve__
template <typename U, asl_randommethod_t Method>
class asl_random_wrapper {
  asl_random_t handle;
  bool released = false;

public:
  using kernel_type = asl_random_t;
  using result_type = U;

  static constexpr
  result_type default_seed = aslutil::default_seed_for<Method>();

  explicit asl_random_wrapper(const result_type value = default_seed) :
    handle(_create_handle())
  {
    seed(value);
#ifdef _ASL_RANDOM_DEFAULT_FULL_RESOLUTION_
    set_full_resolution_mode(true);
#endif
  }

  template <typename SeedType>
  explicit asl_random_wrapper(const std::vector<SeedType>& vec) :
    handle(_create_handle())
  {
    seed(vec);
#ifdef _ASL_RANDOM_DEFAULT_FULL_RESOLUTION_
    set_full_resolution_mode(true);
#endif
  }

  asl_random_wrapper(const asl_random_wrapper<U, Method>& src) :
    handle(), released(src.released)
  {
    aslutil::checkerror(
      asl_random_copy(&handle, src.handle),
      "failed to copy ASL random handle"
    );
    set_full_resolution_mode(src.is_full_resolution_mode());
  }

  asl_random_wrapper(asl_random_wrapper<U, Method>&& src) :
    handle(std::move(src.handle)), released(src.released)
  {
    set_full_resolution_mode(src.is_full_resolution_mode());
    src.released = true;
  }

  asl_random_wrapper<U, Method>&
  operator=(const asl_random_wrapper<U, Method>& src) {
    _destroy();
    aslutil::checkerror(
      asl_random_copy(&handle, src.handle),
      "failed to copy ASL random handle"
    );
    released = src.released;
    set_full_resolution_mode(src.is_full_resolution_mode());
    return *this;
  }

  asl_random_wrapper<U, Method>&
  operator=(asl_random_wrapper<U, Method>&& src) {
    _destroy();
    handle = std::move(src.handle);
    released = src.released;
    set_full_resolution_mode(src.is_full_resolution_mode());
    src.released = true;
    return *this;
  }

  ~asl_random_wrapper() { _destroy(); }

  void seed(const result_type value = default_seed) {
    aslutil::seed<result_type>(handle, value);
  }

  template <typename SeedType>
  void seed(const std::vector<SeedType>& vec) {
    aslutil::seeds<SeedType>(handle, vec.size(), vec.data());
  }

  result_type operator()() { return generate<result_type>(); }

  void operator()(const size_t num, result_type* dest) {
    generate<result_type>(num, dest);
  }

  std::vector<result_type> operator()(const size_t num) {
    return generate<result_type>(num);
  }

  template <typename R>
  R generate() {
    result_type ret;
    generate<result_type>(1, &ret);
    return static_cast<R>(ret);
  }

  template <typename R, aslutil::enable_if_randombitable_t<R> = nullptr>
  void generate(const size_t num, R* dest) {
    aslutil::randombits<R>(handle, num, dest);
  }

  template <typename R, aslutil::enable_if_unrandombitable_t<R> = nullptr>
  void generate(const size_t num, R* dest) {
    std::vector<result_type> vec(num, 0);
    result_type* tmp = vec.data();
    generate<result_type>(num, tmp);
    for (size_t i = 0; i < num; i++) { dest[i] = static_cast<R>(tmp[i]); }
  }

  template <typename R>
  std::vector<R> generate(const size_t num) {
    std::vector<R> ret(num, 0);
    generate<R>(num, ret.data());
    return ret;
  }

  void discard(const size_t num) { this->operator()(num, ASL_NULL); }

  static constexpr
  result_type min() { return std::numeric_limits<result_type>::min(); }
  static constexpr
  result_type max() { return std::numeric_limits<result_type>::max(); }

  kernel_type& refer_kernel() { return handle; }

  kernel_type release_kernel() {
    released = true;
    return std::move(handle);
  }

  bool is_released() const { return released; }

  bool is_full_resolution_mode() const {
    return asl_random_is_full_resolution_enabled(
      const_cast<asl_random_wrapper<U, Method>*>(this)->handle
    );
  }

  void set_full_resolution_mode(const bool value) {
    aslutil::checkerror(
      asl_random_enable_full_resolution(handle, value),
      "failed to set ASL random full resolution mode"
    );
  }

private:
  static asl_random_t _create_handle() {
    asl_random_t ret;
    aslutil::initlib();
    aslutil::checkerror(
      asl_random_create(&ret, Method),
      "failed to create ASL random handle"
    );
    return ret;
  }

  void _destroy() {
    if (!released) {
      aslutil::checkerror(
        asl_random_destroy(handle),
        "failed to destroy ASL random handle"
      );
      released = true;
    }
  }
};
#endif

// definitions for static member
template <typename Engine>
constexpr typename std_random_wrapper<Engine>::result_type
std_random_wrapper<Engine>::default_seed;
#ifdef __ve__
template <typename U, asl_randommethod_t Method>
constexpr typename asl_random_wrapper<U, Method>::result_type
asl_random_wrapper<U, Method>::default_seed;
#endif

// aliases
#ifndef __ve__
using mt19937 = std_random_wrapper<std::mt19937>;
using mt19937_64 = std_random_wrapper<std::mt19937_64>;
#else
using asl_mt19937 =
  asl_random_wrapper<asl_uint32_t, ASL_RANDOMMETHOD_MT19937>;
using asl_mt19937_64 =
  asl_random_wrapper<asl_uint64_t, ASL_RANDOMMETHOD_MT19937_64>;
using mt19937 = asl_mt19937;
using mt19937_64 = asl_mt19937_64;
#endif

// ---------------------------------------------------------------------

template <typename IntType =
          typename std::uniform_int_distribution<>::result_type>
class uniform_int_distribution {
  std::uniform_int_distribution<IntType> dist;

public:
  using result_type = IntType;

  explicit uniform_int_distribution(
    IntType a = 0, IntType b = std::numeric_limits<IntType>::max()
  ) :
    dist(a, b)
  {
    if (b < a) {
      std::ostringstream sout;
      sout << "invalid range: [" << a << ", " << b << "]";
      throw std::runtime_error(sout.str());
    }
  }

  explicit uniform_int_distribution(
    const std::uniform_int_distribution<IntType>& dist
  ) :
    uniform_int_distribution(dist.a(), dist.b())
  {}

  template <typename Engine>
  result_type operator()(std_random_wrapper<Engine>& w) {
    return dist(w.refer_kernel());
  }

  template <typename Engine>
  void operator()(
    std_random_wrapper<Engine>& w, const size_t num, result_type* dest
  ) {
    for (size_t i = 0; i < num; i++) { dest[i] = this->operator()(w); }
  }

#ifdef __ve__
  template <typename U, asl_randommethod_t Method>
  result_type operator()(asl_random_wrapper<U, Method>& w) {
    result_type ret;
    this->operator()(w, 1, &ret);
    return ret;
  }

  // TODO: optimize
  template <typename U, asl_randommethod_t Method>
  void operator()(
    asl_random_wrapper<U, Method>& w, const size_t num, result_type* dest
  ) {
    w.generate(num, dest);

    using uresult_type = typename std::make_unsigned<result_type>::type;
    const result_type a = dist.a(), b = dist.b();
    const uresult_type d = b - a, m = d + 1;

    if (d == std::numeric_limits<uresult_type>::max()) {
      ___myassert___(a == std::numeric_limits<result_type>::min());
      ___myassert___(b == std::numeric_limits<result_type>::max());
      return;
    }

    ___myassert___(d < m);
    auto* udest = reinterpret_cast<uresult_type*>(dest);

    if (a == 0) {
      ___myassert___(d <= std::numeric_limits<result_type>::max());
      for (size_t i = 0; i < num; i++) { udest[i] %= m; }
    } else {
#pragma cdir novovertake
#pragma _NEC novovertake
      for (size_t i = 0; i < num; i++) {
        udest[i] %= m;
        dest[i] += a;
      }
    }
  }
#endif

  template <typename Wrapper>
  std::vector<result_type> operator()(Wrapper& w, const size_t num) {
    std::vector<result_type> ret(num, 0);
    this->operator()(w, num, ret.data());
    return ret;
  }

  result_type a() const { return dist.a(); }
  result_type b() const { return dist.b(); }
  result_type min() const { return dist.min(); }
  result_type max() const { return dist.max(); }
};

template <typename RealType =
          typename std::uniform_real_distribution<>::result_type>
class uniform_real_distribution {
  std::uniform_real_distribution<RealType> dist;

public:
  using result_type = RealType;

  explicit uniform_real_distribution(
    RealType a = 0.0, RealType b = 1.0
  ) :
    dist(a, b)
  {
    if (b <= a) {
      std::ostringstream sout;
      sout << "invalid range: [" << a << ", " << b << ")";
      throw std::runtime_error(sout.str());
    }
  }

  explicit uniform_real_distribution(
    const std::uniform_real_distribution<RealType>& dist
  ) :
    uniform_real_distribution(dist.a(), dist.b())
  {}

  template <typename Engine>
  result_type operator()(std_random_wrapper<Engine>& w) {
    return dist(w.refer_kernel());
  }

  template <typename Engine>
  void operator()(
    std_random_wrapper<Engine>& w, const size_t num, result_type* dest
  ) {
    for (size_t i = 0; i < num; i++) { dest[i] = this->operator()(w); }
  }

#ifdef __ve__
  template <typename U, asl_randommethod_t Method>
  result_type operator()(asl_random_wrapper<U, Method>& w) {
    result_type ret;
    this->operator()(w, 1, &ret);
    return ret;
  }

  template <typename U, asl_randommethod_t Method>
  void operator()(
    asl_random_wrapper<U, Method>& w, const size_t num, result_type* dest
  ) {
    aslutil::checkerror(
      asl_random_distribute_uniform(w.refer_kernel()),
      "failed to set ASL random uniform distribution"
    );
    aslutil::generate<result_type>(w.refer_kernel(), num, dest);

    const result_type a = dist.a(), b = dist.b(), d = b - a;
    if (d == 1) {
      if (a != 0) {
        for (size_t i = 0; i < num; i++) { dest[i] += a; }
      }
    } else {
      if (a == 0) {
        for (size_t i = 0; i < num; i++) { dest[i] *= d; }
      } else {
        for (size_t i = 0; i < num; i++) { dest[i] = dest[i] * d + a; }
      }
    }
  }
#endif

  template <typename Wrapper>
  std::vector<result_type> operator()(Wrapper& w, const size_t num) {
    std::vector<result_type> ret(num, 0);
    this->operator()(w, num, ret.data());
    return ret;
  }

  result_type a() const { return dist.a(); }
  result_type b() const { return dist.b(); }
  result_type min() const { return dist.min(); }
  result_type max() const { return dist.max(); }
};

// ---------------------------------------------------------------------

template <typename Engine>
class buffered_random_engine {
  Engine engine;
  std::vector<typename Engine::result_type> buffer;
  size_t index;

public:
  using result_type = typename Engine::result_type;
  static constexpr result_type default_seed = Engine::default_seed;

  explicit buffered_random_engine(
    const result_type value = default_seed,
    const size_t buffer_size = 256 * 1024
  ) :
    engine(value), buffer(buffer_size, 0), index(buffer_size)
  {}

  template <typename SeedType>
  explicit buffered_random_engine(
    const std::vector<SeedType>& vec,
    const size_t buffer_size = 256 * 1024
  ) :
    engine(vec), buffer(buffer_size, 0), index(buffer_size)
  {}

  buffered_random_engine(const buffered_random_engine<Engine>&) = default;
  buffered_random_engine(buffered_random_engine<Engine>&&) = default;
  // ~buffered_random_engine() = default;

  buffered_random_engine<Engine>&
  operator=(const buffered_random_engine<Engine>&) = default;
  buffered_random_engine<Engine>&
  operator=(buffered_random_engine<Engine>&&) = default;

  template <typename... Args>
  void seed(Args&&... args) { engine.seed(std::forward<Args>(args)...); }

  result_type operator()() { return generate<result_type>(); }

  void operator()(const size_t num, result_type* dest) {
    generate<result_type>(num, dest);
  }

  std::vector<result_type> operator()(const size_t num) {
    return generate<result_type>(num);
  }

  template <typename R>
  R generate() {
    result_type ret;
    generate<result_type>(1, &ret);
    return static_cast<R>(ret);
  }

  template <typename R, enable_if_same_t<R, result_type> = nullptr>
  void generate(const size_t num, result_type* dest) {
    const size_t num_remains = get_num_remains();
    if (num <= num_remains) {
      std::memcpy(dest, _head(), _bytes(num));
      index += num;
      ___myassert___(index <= buffer.size());
      return;
    }

    // use all the remains in buffer
    if (num_remains > 0) {
      std::memcpy(dest, _head(), _bytes(num_remains));
      index = buffer.size();
    }

    ___myassert___(is_empty());
    ___myassert___(num_remains < num);
    if (num <= num_remains + buffer.size()) {
      engine(buffer.size(), buffer.data());
      const size_t num_short = num - num_remains;
      ___myassert___(0 < num_short && num_short <= buffer.size());
      std::memcpy(dest + num_remains, _head(), _bytes(num_short));
      index = num_short;
    } else {
      engine(num - num_remains, dest + num_remains);
      ___myassert___(is_empty());
    }
  }

  template <typename R, enable_if_differ_t<R, result_type> = nullptr>
  void generate(const size_t num, R* dest) {
    std::vector<result_type> vec(num, 0);
    result_type* tmp = vec.data();
    generate<result_type>(num, tmp);
    for (size_t i = 0; i < num; i++) { dest[i] = static_cast<R>(tmp[i]); }
  }

  template <typename R>
  std::vector<R> generate(const size_t num) {
    std::vector<R> ret(num, 0);
    generate<R>(num, ret.data());
    return ret;
  }

  void discard(const size_t num) {
    const size_t num_remains = get_num_remains();
    if (num <= num_remains) {
      index += num;
      ___myassert___(index <= buffer.size());
      return;
    }

    index = buffer.size();
    ___myassert___(num_remains < num);
    engine.discard(num - num_remains);
    ___myassert___(is_empty());
  }

  static constexpr result_type min() { return Engine::min(); }
  static constexpr result_type max() { return Engine::max(); }

  size_t get_buffer_size() const { return buffer.size(); }

  size_t get_num_remains() const {
    ___myassert___(index <= buffer.size());
    return buffer.size() - index;
  }

  bool is_empty() const { return (index == buffer.size()); }
  bool is_full() const { return (index == 0); }

  void fill() {
    if (is_full()) { return; }

    if (is_empty()) {
      engine(buffer.size(), buffer.data());
    } else {
      result_type* ptr = buffer.data();
      const size_t num_remains = get_num_remains();
      std::memmove(ptr, ptr + index, _bytes(num_remains));
      engine(index, ptr + num_remains);
    }

    index = 0;
  }

  void resize_buffer(const size_t buffer_size) {
    if (get_num_remains() > 0) {
      //discard(num_remains);
      RLOG(TRACE) <<
        "the remains of random numbers in buffer have discarded" <<
      std::endl;
    }

    buffer = std::vector<result_type>(buffer_size, 0);
    index = buffer_size;
    ___myassert___(is_empty());
  }

  Engine& refer_engine() { return engine; }

private:
  result_type* _head() { return buffer.data() + index; }
  size_t _bytes(const size_t n) const { return sizeof(result_type) * n; }
};

// definition for static member
template <typename Engine>
constexpr typename buffered_random_engine<Engine>::result_type
buffered_random_engine<Engine>::default_seed;

// aliases
using buffered_mt19937 = buffered_random_engine<mt19937>;
using buffered_mt19937_64 = buffered_random_engine<mt19937_64>;

// ---------------------------------------------------------------------

template <typename T, typename Engine>
struct shuffle_helper {
  static void inplace(std::vector<T>& vec, Engine& eng) {
    std::shuffle(vec.begin(), vec.end(), eng);
  }

  static std::vector<T> copy(const std::vector<T>& vec, Engine& eng) {
    std::vector<T> ret(vec);
    std::shuffle(ret.begin(), ret.end(), eng);
    return ret;
  }
};

#ifdef __ve__
template <typename T, typename U, asl_randommethod_t Method>
struct shuffle_helper<T, asl_random_wrapper<U, Method>> {
private:
  using Engine = asl_random_wrapper<U, Method>;

  static constexpr
  bool positive_only() { return std::is_unsigned<U>::value; }

public:
  static void inplace(std::vector<T>& vec, Engine& eng) {
    std::vector<U> keys = eng(vec.size());
    radix_sort<U, T>(keys, vec, positive_only());
  }

  static std::vector<T> copy(const std::vector<T>& vec, Engine& eng) {
    std::vector<T> ret(vec);
    std::vector<U> keys = eng(vec.size());
    radix_sort<U, T>(keys, ret, positive_only());
    return ret;
  }
};
#endif

template <typename T, typename UniformRandomBitGenerator>
inline void shuffle_inplace(
  std::vector<T>& vec, UniformRandomBitGenerator& g
) {
  shuffle_helper<T, UniformRandomBitGenerator>::inplace(vec, g);
}

template <typename T, typename UniformRandomBitGenerator>
inline std::vector<T> shuffle_copy(
  const std::vector<T>& vec, UniformRandomBitGenerator& g
) {
  return shuffle_helper<T, UniformRandomBitGenerator>::copy(vec, g);
}

template <typename T, typename UniformRandomBitGenerator>
inline void shuffle(std::vector<T>& vec, UniformRandomBitGenerator& g) {
  shuffle_inplace(vec, g);
}

template <typename T, typename UniformRandomBitGenerator>
std::vector<T> sampling_without_replacement(
  const std::vector<T>& vec, const size_t num,
  UniformRandomBitGenerator& g
) {
  if (vec.size() < num) {
    const std::string errmsg("too large number to sample");
    RLOG(ERROR) << errmsg << std::endl;
    throw std::runtime_error(errmsg);
  }

  if (vec.size() == 0 || num == 0) { return std::vector<T>(); }
  if (vec.size() == 1) { return vec; }
  if (num == 1) { return std::vector<T>(1, vec[g() % vec.size()]); }

  std::vector<T> shuffled = shuffle_copy(vec, g);
  if (vec.size() == num) { return shuffled; }

  std::vector<T> ret(num, 0);
  std::memcpy(ret.data(), shuffled.data(), sizeof(T) * num);
  return ret;
}

} // end namespace frovedis

#undef ___myassert___

#endif
