#ifndef LOG_HPP
#define LOG_HPP

#include <iostream>

namespace frovedis {

enum log_level {
  TRACE = 0,
  DEBUG,
  INFO,
  WARNING,
  ERROR,
  FATAL,
};

extern log_level log_reporting_level;

// see the trick at www.drdobbs.com/cpp/201804215?pgno=2
#define LOG(level) if(level < frovedis::log_reporting_level); else std::cerr

log_level set_loglevel(log_level);
log_level get_loglevel();

extern log_level old_verbose_level;
void set_verbose_level(int verbose);
void reset_verbose_level();

}
#endif
