
#ifndef GRAPH_GLOBAL_HPP
#define GRAPH_GLOBAL_HPP

#include <frovedis.hpp>
#include <frovedis/matrix/crs_matrix.hpp>
//#include <frovedis/matrix/ccs_matrix.hpp>
#include <frovedis/matrix/jds_crs_hybrid.hpp>
#include <frovedis/matrix/shrink_matrix.hpp>
#include <frovedis/core/set_operations.hpp>
#include <frovedis/core/utility.hpp>
#include <frovedis/core/zipped_dvectors.hpp>
#include "frovedis/ml/graph/sparse_vector.hpp"
#include <vector>
#include <iostream>
#include <string>

#include <numeric>
#include <cmath>
#include <algorithm>
#include <utility>

namespace frovedis {

#define TYPE_BITMAP int
#define TYPE_MATRIX int
#define TYPE_MATRIX_PAGERANK double
#define MAX_MATRIX INT_MAX
#define TYPE_IDXV size_t
#define TIME_RECORD_LOG_LEVEL DEBUG

#define FNAME_NODES_DIST "nodes_dist"
#define FNAME_NODES_IN_WHICH_CC "nodes_in_which_cc"
#define FNAME_NODES_PRED "nodes_pred"
#define FNAME_NUM_NODES_IN_EACH_CC "num_nodes_in_each_cc"


class time_spent_graph {
public:
  time_spent_graph() : t0(frovedis::get_dtime()), t1(0), lap_sum(0), loglevel(INFO) {}
  time_spent_graph(frovedis::log_level l) : t0(frovedis::get_dtime()), t1(0), lap_sum(0), loglevel(l) {}
  void show(const std::string& mes) {
    t1 = frovedis::get_dtime();
    RLOG(loglevel) << mes << t1 - t0 << " sec" << std::endl;
    t0 = t1;
  }
  void reset(){t0 = frovedis::get_dtime(); lap_sum = 0;}
  void lap_start(){t0 = frovedis::get_dtime();}
  void lap_stop(){lap_sum += frovedis::get_dtime() - t0;}
  double get_lap(){return lap_sum;}
  void show_lap(const std::string& mes){
    RLOG(loglevel) << mes << lap_sum << " sec" << std::endl;
  }
  double show_lap_value(){return lap_sum;}
  
private:
  double t0, t1;
  double lap_sum;
  frovedis::log_level loglevel;
};

}
#endif /* GRAPH_GLOBAL_HPP */

