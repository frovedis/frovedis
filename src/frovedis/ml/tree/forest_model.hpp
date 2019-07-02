#ifndef _FOREST_MODEL_HPP_
#define _FOREST_MODEL_HPP_

#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include <cereal/archives/binary.hpp>
#include <cereal/archives/json.hpp>
#include <cereal/types/vector.hpp>

#include "../../../frovedis.hpp"
#include "../../matrix/rowmajor_matrix.hpp"
// TODO: implement predict functions for sparse-data
//#include "../../matrix/crs_matrix.hpp"

#include "tree_assert.hpp"
#include "tree_config.hpp"
#include "tree_model.hpp"
#include "tree_utility.hpp"

namespace frovedis {

template <typename T>
class random_forest_model {
  tree::algorithm algo;
  std::vector<decision_tree_model<T>> trees;

  SERIALIZE(algo, trees)

public:
  template <typename DATA>
  using predict_type =
  typename decision_tree_model<T>::template predict_type<DATA>;

  random_forest_model() {}

  random_forest_model(
    const tree::algorithm algo,
    std::vector<decision_tree_model<T>> trees
  ) :
    algo(algo), trees(std::move(trees))
  {}

  tree::algorithm get_algo() const { return algo; }
  tree::algorithm get_algorithm() const { return get_algo(); }
  size_t get_num_trees() const { return trees.size(); }

  size_t get_total_num_nodes() const {
    size_t ret = 0;
    for (const auto& tree: trees) { ret += tree.get_num_nodes(); }
    return ret;
  }

  const std::vector<decision_tree_model<T>>&
  get_trees() const& { return trees; }

  std::vector<decision_tree_model<T>>
  get_trees() && { return std::move(trees); }

  template <typename DATA>
  predict_type<DATA>
  predict(DATA&& x) const {
    switch (algo) {
    case tree::algorithm::Classification:
      return predict_by_vote(std::forward<DATA>(x));
    case tree::algorithm::Regression:
      return predict_by_average(std::forward<DATA>(x));
    default:
      throw std::logic_error("invalid tree algorithm");
    }
  }

  // file IO
  void save(const std::string& path) const {
    _save<cereal::JSONOutputArchive>(path);
  }
  void load(const std::string& path) {
    _load<cereal::JSONInputArchive>(path);
  }
  void savebinary(const std::string& path) const {
    _save<cereal::BinaryOutputArchive>(path);
  }
  void loadbinary(const std::string& path) {
    _load<cereal::BinaryInputArchive>(path);
  }

  // string output
  std::string to_string() const;
  void debug_print() const { std::cout << to_string(); }

  // shortcuts
  bool is_classification_model() const {
    return algo == tree::algorithm::Classification;
  }
  bool is_regression_model() const {
    return algo == tree::algorithm::Regression;
  }

  node_local<random_forest_model<T>> broadcast() const {
    return make_node_local_broadcast(*this);
  }

private:
  // predict for a single dense-data point
  T predict_by_vote(const std::vector<T>&) const;
  T predict_by_average(const std::vector<T>&) const;

  // predict for the given dense-data point
  std::vector<T> predict_by_vote(const rowmajor_matrix_local<T>&) const;
  std::vector<T> predict_by_average(const rowmajor_matrix_local<T>&) const;

  template <typename Archive>
  void _save(const std::string& path) const {
    tree::save_model<random_forest_model<T>, Archive>(*this, path);
  }

  template <typename Archive>
  void _load(const std::string& path) {
    *this = tree::load_model<random_forest_model<T>, Archive>(path);
  }
};

template <typename T>
std::ostream& operator<<(
  std::ostream& os, const random_forest_model<T>& model
) {
  return os << model.to_string();
}

template <typename T>
std::string random_forest_model<T>::to_string() const {
  std::ostringstream sout;

  switch (algo) {
  case tree::algorithm::Classification:
    sout << "-------- Classification Trees:: --------" << std::endl;
    break;
  case tree::algorithm::Regression:
    sout << "-------- Regression Trees:: --------" << std::endl;
    break;
  }

  sout << "# of trees: " << get_num_trees() << std::endl;

  size_t i = 0;
  for (const auto& tree: trees) {
    sout << "---- [" << i++ << "] ----" << std::endl;
    sout << "  ";
    sout << "# of nodes: " << tree.get_num_nodes() << ", ";
    sout << "height: " << tree.get_height() << std::endl;
    sout << tree.get_top_node()->to_string("  ");
  }

  return sout.str();
}

template <typename T>
T random_forest_model<T>::predict_by_vote(
  const std::vector<T>& v
) const {
  std::unordered_map<T, size_t> voting_box;
  for (const auto& tree: trees) {
    const T candidate = tree.predict(v);
    if (voting_box.count(candidate)) {
      voting_box[candidate] += 1;
    } else {
      voting_box.emplace(candidate, 1);
    }
  }

  T candidate = 0;
  size_t count = 0;
  for (const auto& vote: voting_box) {
    if (count < vote.second) {
      candidate = vote.first;
      count = vote.second;
    }
  }

  return candidate;
}

template <typename T>
T random_forest_model<T>::predict_by_average(
  const std::vector<T>& v
) const {
  T sum = 0;
  for (const auto& tree: trees) { sum += tree.predict(v); }
  return sum / get_num_trees();
}

template <typename T>
std::vector<T> random_forest_model<T>::predict_by_vote(
  const rowmajor_matrix_local<T>& x
) const {
  const size_t num_records = x.local_num_row;
  std::vector<std::unordered_map<T, size_t>> voting_boxes(num_records);
  std::unordered_map<T, size_t>* vbox = voting_boxes.data();

  for (const auto& tree: trees) {
    const std::vector<T> predicts = tree.predict(x);
    const T* src = predicts.data();
    for (size_t i = 0; i < num_records; i++) {
      if (vbox[i].count(src[i])) {
        vbox[i][src[i]] += 1;
      } else {
        vbox[i].emplace(src[i], 1);
      }
    }
  }

  std::vector<T> ret(num_records, 0);
  std::vector<size_t> counts(num_records, 0);
  T* dest = ret.data();
  size_t* cnt = counts.data();

  for (size_t i = 0; i < num_records; i++) {
    for (const auto& vote: vbox[i]) {
      if (cnt[i] < vote.second) {
        dest[i] = vote.first;
        cnt[i] = vote.second;
      }
    }
  }

  return ret;
}

template <typename T>
std::vector<T> random_forest_model<T>::predict_by_average(
  const rowmajor_matrix_local<T>& x
) const {
  const size_t num_records = x.local_num_row;
  std::vector<T> ret(num_records, 0);
  T* dest = ret.data();

  for (const auto& tree: trees) {
    const std::vector<T> predicts = tree.predict(x);
    const T* src = predicts.data();
    for (size_t i = 0; i < num_records; i++) { dest[i] += src[i]; }
  }

  const size_t num_trees = get_num_trees();
  for (size_t i = 0; i < num_records; i++) { dest[i] /= num_trees; }

  return ret;
}

// shortcut functions
template <typename T>
random_forest_model<T> make_forest_by_load(const std::string& path) {
  random_forest_model<T> model;
  model.load(path);
  return model;
}

template <typename T>
random_forest_model<T> make_forest_by_loadbinary(const std::string& path) {
  random_forest_model<T> model;
  model.loadbinary(path);
  return model;
}

} // end namespace frovedis

#endif
