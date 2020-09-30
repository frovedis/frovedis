#ifndef DFTABLE_TO_WORDS
#define DFTABLE_TO_WORDS

#include "dftable.hpp"

namespace frovedis {

node_local<words> dftable_to_words(dftable_base& table,
                                   size_t precision = 6,
                                   const std::string& datetime_fmt = "%Y-%m-%d",
                                   bool quote_escape = false,
                                   const std::string& nullstr = "NULL");

}
#endif
