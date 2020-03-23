#ifndef MAKE_DFTABLE_STRING_HPP
#define MAKE_DFTABLE_STRING_HPP

#include "dftable.hpp"
#include <boost/tokenizer.hpp>

namespace frovedis {

dftable make_dftable_string(dvector<std::string>& d,
                            const std::vector<std::string>& types,
                            const std::vector<std::string>& names);

dftable make_dftable_string(dvector<std::string>& d,
                            const std::vector<std::string>& types);

dftable make_dftable_string(dvector<std::string>& d,
                            const std::vector<std::string>& types,
                            const std::vector<std::string>& names,
                            const std::string& sep);

dftable make_dftable_string(dvector<std::string>& d,
                            const std::vector<std::string>& types,
                            const std::string& sep);

}

#endif
