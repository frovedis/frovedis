#include <stdexcept>
#include "log.hpp"

namespace frovedis {

log_level log_reporting_level = INFO;

log_level set_loglevel(log_level level) {
  log_level oldlevel = log_reporting_level;
  log_reporting_level = level;
  return oldlevel;
}
log_level get_loglevel() {return log_reporting_level;}

log_level old_verbose_level = INFO;

void set_verbose_level(int verbose) {
  old_verbose_level = get_loglevel();
  if(verbose == 0) set_loglevel(INFO);
  else if(verbose == 1) set_loglevel(DEBUG);
  else if(verbose == 2) set_loglevel(TRACE);
  else throw std::runtime_error("supported verbose levels are 0, 1 or 2"); 
}
void reset_verbose_level() {log_reporting_level = old_verbose_level;}

}

