#include "log.hpp"

namespace frovedis {

log_level log_reporting_level = INFO;

log_level set_loglevel(log_level level) {
  log_level oldlevel = log_reporting_level;
  log_reporting_level = level;
  return oldlevel;
}
log_level get_loglevel() {return log_reporting_level;}


}

