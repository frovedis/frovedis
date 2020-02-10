#ifndef LOAD_TEXT_HPP
#define LOAD_TEXT_HPP

#include "../core/node_local.hpp"
#include "char_int_conv.hpp"
#include "find.hpp"

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

}

#endif