#ifndef _TREE_ASSERT_HPP_
#define _TREE_ASSERT_HPP_

#include <cassert>

namespace frovedis {
namespace tree {

#if defined(_TREE_ASSERT_) || defined(_TREE_DEBUG_)
#define tree_assert(expr) assert(expr)
#else
#define tree_assert(ignore) ((void) 0)
#endif

} // end namespace tree
} // end namespace frovedis

#endif
