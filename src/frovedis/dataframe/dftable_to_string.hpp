#ifndef DFTABLE_TO_STRING
#define DFTABLE_TO_STRING

#include "dftable.hpp"

namespace frovedis {

// NOTE: obsolete. use dftable_to_words. this function will be deleted
dvector<std::vector<std::string>> dftable_to_string(dftable_base& table);

}
#endif
