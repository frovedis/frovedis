#ifndef LOAD_CSV_HPP
#define LOAD_CSV_HPP

#include "load_text.hpp"
#include "words.hpp"

#if defined(__ve__) || defined(_SX)
#define LOAD_CSV_VLEN 256
#else
//#define LOAD_CSV_VLEN 1
#define LOAD_CSV_VLEN 4
#endif

namespace frovedis {

node_local<words>
load_csv(const std::string& path,
         node_local<std::vector<size_t>>& line_starts_byword,
         bool is_crlf = false,
         bool skip_head = false,
         int separator = ',');

node_local<words>
load_simple_csv(const std::string& path,
                node_local<std::vector<size_t>>& line_starts_byword,
                bool is_crlf = false,
                bool to_skip_head = false,
                int separator = ',');
}
#endif
