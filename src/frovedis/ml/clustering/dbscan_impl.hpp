#ifndef DBSCAN_IMPL_HPP
#define DBSCAN_IMPL_HPP

#include "../../matrix/rowmajor_matrix.hpp"
#include "../../matrix/crs_matrix.hpp"
#include "./common.hpp"

namespace frovedis {

//  Originally written by Tadayoshi Hara
class dbscan_impl {
public:
  template<typename T>
  void fit(rowmajor_matrix<T>& vector_set) {
    num_row = vector_set.num_row;
    d_graph = calc_graph(vector_set, eps);
    d_core_flgs = calc_core_flgs(min_pts);
    d_core_graph = calc_core_graph();
    cluster();
  }
  std::vector<int> labels() {return clustered_labels;}
  dbscan_impl(double eps=0.5, int min_pts=5) : eps(eps), min_pts(min_pts) {}

private:
  template<typename T>
  node_local<crs_matrix_local<int>> calc_graph(rowmajor_matrix<T>& vector_set,
                                               double eps) {
    auto g_vector_set = get_global_data(vector_set);
    // get distance matrix
    auto dist_m = construct_distance_matrix(g_vector_set, true);
    auto ret = construct_connectivity_graph<int>(dist_m, eps);
    return ret.data;
  }

  dvector<int> calc_core_flgs(int min_pts) {
    auto min_pts_l = make_node_local_broadcast(min_pts);
    auto is_bigger_than_min_pts = +[] (crs_matrix_local<int>& graph,
                                       int& min_pts) {
      auto core_flgs = std::vector<int>(graph.off.size() - 1, 0);
      for(int i=1;i<graph.off.size();i++) {
        core_flgs[i - 1] = (graph.off[i] - graph.off[i-1]) > (min_pts - 1);
      }
      return core_flgs;
    };
    return d_graph.map(is_bigger_than_min_pts, min_pts_l).
      template moveto_dvector<int>();
  };

  node_local<crs_matrix_local<int>> calc_core_graph() {
    auto map_core_graph = +[] (crs_matrix_local<int>& graph,
                               std::vector<int>& d_core_flgs,
                               std::vector<int>& g_core_flgs) {
      auto result = crs_matrix_local<int>(d_core_flgs.size(),
                                          g_core_flgs.size());
      auto* idxp = graph.idx.data();
      auto* offp = graph.off.data();
      auto* g_flgp = g_core_flgs.data();
      auto* d_flgp = d_core_flgs.data();
      size_t total_count = 0;
      for(size_t r=0;r<d_core_flgs.size();r++) {
        if (d_flgp[r] == 0) continue;
        auto size = offp[r+1] - offp[r];
        auto crntoff = offp[r];
        #pragma _NEC ivdep
        for(size_t i = 0; i < size; i++) {
          auto j = i + crntoff;
          if (g_flgp[idxp[j]] == 1) total_count++;
        }
      }
      result.val.resize(total_count);
      result.idx.resize(total_count);
      result.off.resize(d_core_flgs.size() + 1, 0);
      auto* c_valp = result.val.data();
      auto* c_idxp = result.idx.data();
      auto* c_offp = result.off.data();
      size_t idxoff = 0;
      for(size_t r=0;r<d_core_flgs.size();r++) {
        if (d_flgp[r] == 1) {
          auto size = offp[r+1] - offp[r];
          auto crntoff = offp[r];
          size_t inc = 0;
#pragma _NEC ivdep
          for(size_t i = 0; i < size; i++) {
            auto j = i + crntoff;
            if (g_flgp[idxp[j]] != 0) {
              c_valp[inc + idxoff] = 1;
              c_idxp[inc + idxoff] = idxp[j];
              inc++;
            }
          }
          idxoff += inc;
        }
        c_offp[r+1] = idxoff;
      }
      return result;
    };
    auto g_core_flgs = make_node_local_broadcast(d_core_flgs.gather());
    return d_graph.map(map_core_graph, d_core_flgs.viewas_node_local(),
                       g_core_flgs);
  };

  void core_labeling() {
    clustered_labels = std::vector<int>(num_row, -1);

    // core labeling
    auto label_l = make_node_local_broadcast(0);
    auto d_labels = dvector<int>();
    auto core_flgs = d_core_flgs.gather();
    // lambda function for map
    auto is_all_zero = [](std::vector<int> v) {
      auto* ptr = v.data();
      size_t total = 0;
      for(size_t i=0;i<v.size();i++) total += ptr[i];
      return total == 0;
    };
    auto vist_core = +[](crs_matrix_local<int>& core_graph,
                         std::vector<int>& frontier) {
      return core_graph * frontier;
    };
    auto labeling = +[](std::vector<int>& labels, std::vector<int>& visit,
                        int& label) {
      for(int i=0;i<visit.size();i++) {
        if (visit[i] == 0) continue;
        if (labels[i] != -1) continue;
        labels[i] = label;
      }
    };
    auto update_frontier = +[](std::vector<int>& frontier,
                               std::vector<int>& visit,
                               std::vector<int>& labels) {
      for(int i=0;i<visit.size();i++) {
        frontier[i] = (int) (visit[i] != 0 && labels[i] == -1);
      }
    };
    for(int i=0;i<num_row;i++) {
      if (clustered_labels[i] != -1 || core_flgs[i] != 1) continue;
      auto frontier = std::vector<int>(num_row, 0);
      frontier[i] = 1;
      clustered_labels[i] = label_l.get(0);
      auto d_frontier = make_dvector_scatter(frontier);
      d_labels = make_dvector_scatter(clustered_labels);
      while (!is_all_zero(frontier)) {
        auto g_frontier = make_node_local_broadcast(frontier);
        auto d_visit = d_core_graph.map(vist_core, g_frontier);
        d_frontier.viewas_node_local().mapv(update_frontier, d_visit,
                                            d_labels.viewas_node_local());
        d_labels.viewas_node_local().mapv(labeling, d_visit, label_l);
        frontier = d_frontier.gather();
      }
      label_l.mapv(+[](int& label){label++;});
      clustered_labels = d_labels.gather();
    }
  };

  void other_labeling() {
    // others labeling
    auto labeling = +[] (crs_matrix_local<int>& graph,
                         std::vector<int>& d_labels,
                         std::vector<int>& g_labels) {
      auto ret = g_labels;
      for(int i=0;i<d_labels.size();i++) {
        if (d_labels[i] == -1) continue;
        auto label = d_labels[i];
        for (int j=graph.off[i];j<graph.off[i+1];j++) {
          ret[graph.idx[j]] = label;
        }
      }
      return ret;
    };
    auto d_labels = make_dvector_scatter(clustered_labels);
    auto g_lables = make_node_local_broadcast(clustered_labels);
    clustered_labels = d_graph.map(labeling, d_labels.viewas_node_local(),
                                   g_lables).
      reduce(+[] (std::vector<int>& a, std::vector<int>& b) {
          auto ret = b;
          for(int i=0;i<ret.size();i++) {
            if (ret[i] != -1) continue;
            ret[i] = a[i];
          }
          return ret;
        });
  };

  void cluster() {
    core_labeling();
    other_labeling();
  };

  int num_row = 0;
  node_local<crs_matrix_local<int>> d_graph;
  dvector<int> d_core_flgs;
  node_local<crs_matrix_local<int>> d_core_graph;
  std::vector<int> clustered_labels;
  double eps;
  int min_pts;
};
}

#endif
