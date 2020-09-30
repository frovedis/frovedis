#ifndef FIND_HPP
#define FIND_HPP

#include <vector>
#include <string>
#include <cstddef>

#if defined(__ve__) || defined(_SX)
#define FIND_VLEN 1024
#define FIND_VLEN_EACH 256
#define FIND_ALIGN_SIZE 128
#else
#define FIND_VLEN 4
#define FIND_VLEN_EACH 1
#define FIND_ALIGN_SIZE 1
#endif


namespace frovedis {

std::vector<size_t> find_impl(const int* svp, size_t size,
                              const std::string& to_find);

std::vector<size_t> find_impl(const std::vector<int>& sv,
                              const std::string& to_find);

std::vector<size_t> find(const int* vp, size_t size,
                         const std::string& to_find,
                         size_t block_size_mb = 10);

std::vector<size_t> find(const std::vector<int>& sv,
                         const std::string& to_find,
                         size_t block_size_mb = 10);

}

#endif
