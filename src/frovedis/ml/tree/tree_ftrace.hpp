#ifndef _TREE_FTRACE_HPP_
#define _TREE_FTRACE_HPP_

#include <string>

#if (defined(_SX) || defined(__ve__)) && defined(_FTRACE)
#define __TREE_FTRACE_REGION__
#include <ftrace.h>
#endif

namespace frovedis {
namespace tree {

class ftrace_region {
#ifdef __TREE_FTRACE_REGION__

  std::string id;

public:
  ftrace_region(const std::string& id) : id(id) {
    ftrace_region_begin(id.c_str());
  }

  void end() const { ftrace_region_end(id.c_str()); }

#else

public:
  ftrace_region(const char*) {}
  ftrace_region(const std::string&) {}
  void end() const {}

#endif
};

} // end namespace tree
} // end namespace frovedis

#endif
