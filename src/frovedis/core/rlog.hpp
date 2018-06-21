#ifndef RLOG_HPP
#define RLOG_HPP

#include "log.hpp"
#include "mpi_rpc.hpp"

namespace frovedis {
NID get_selfid();
}
  
using frovedis::TRACE;
using frovedis::DEBUG;
using frovedis::INFO;
using frovedis::WARNING;
using frovedis::ERROR;
using frovedis::FATAL;

#define RLOG(level) LOG(level) << "[rank " << frovedis::get_selfid() << "] "
  
#endif
