#ifndef _MAKE_UNIQUE_HPP_
#define _MAKE_UNIQUE_HPP_

// if under C++14
#if __cplusplus <= 201103L

#include <cstddef>
#include <memory>
#include <type_traits>
#include <utility>

namespace std {
	template <typename T>
	struct __unique_if {
		using __single_object = unique_ptr<T>;
	};

	template <typename T>
	struct __unique_if<T[]> {
		using __unknown_bound = unique_ptr<T[]>;
	};

	template <typename T, size_t N>
	struct __unique_if<T[N]> {
		using __known_bound = void;
	};

	template <typename T, typename... Args>
	inline typename __unique_if<T>::__single_object
	make_unique(Args&&... args) {
		return unique_ptr<T>(new T(std::forward<Args>(args)...));
	}

	template <typename T>
	inline typename __unique_if<T>::__unknown_bound
	make_unique(size_t n) {
		using U = typename remove_extent<T>::type;
		return unique_ptr<T>(new U[n]());
	}

	template <typename T, typename... Args>
	typename __unique_if<T>::__known_bound
	make_unique(Args&&...) = delete;
}

#endif

#endif
