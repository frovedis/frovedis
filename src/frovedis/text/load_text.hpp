#ifndef LOAD_TEXT_HPP
#define LOAD_TEXT_HPP

#include "../core/node_local.hpp"
#include "char_int_conv.hpp"
#include "find.hpp"

#ifdef USE_YAS
#include <yas/types/std/vector.hpp>
#include <yas/types/std/string.hpp>
#endif
#ifdef USE_CEREAL
#include <cereal/types/vector.hpp>
#include <cereal/types/string.hpp>
#endif
#ifdef USE_BOOST_SERIALIZATION
#include <boost/serialization/vector.hpp>
#include <boost/serialization/string.hpp>
#endif

namespace frovedis {

node_local<std::vector<int>>
load_text(const std::string& path,
          const std::string& delim,
          node_local<std::vector<size_t>>& sep,
          node_local<std::vector<size_t>>& len);

node_local<std::vector<int>>
load_text_separate(const std::string& path,
                   const std::string& delim,
                   node_local<std::vector<size_t>>& sep,
                   node_local<std::vector<size_t>>& len,
                   ssize_t start, ssize_t& end); // end is input/output

std::vector<int>
load_text_local(const std::string& path,
                const std::string& delim,
                std::vector<size_t>& sep,
                std::vector<size_t>& len);

}

#endif
