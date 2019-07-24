#ifndef _ENSEMBLE_MODEL_HPP_
#define _ENSEMBLE_MODEL_HPP_

#include <cstring>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include <cereal/archives/binary.hpp>
#include <cereal/archives/json.hpp>
#include <cereal/types/base_class.hpp>
#include <cereal/types/vector.hpp>

#include "../../../frovedis.hpp"
#include "../../matrix/rowmajor_matrix.hpp"
#include "../../matrix/colmajor_matrix.hpp"
#include "../../matrix/blas_wrapper.hpp"

// TODO: implement predict functions for sparse-data
//#include "../../matrix/crs_matrix.hpp"

#include "tree_assert.hpp"
#include "tree_config.hpp"
#include "tree_model.hpp"
#include "tree_utility.hpp"
#include "gbts_loss.hpp"

namespace frovedis {

template <typename T>
class tree_ensemble_model {
  tree::algorithm algo;
  std::vector<decision_tree_model<T>> trees;

  SERIALIZE(algo, trees)

public:
  template <typename DATA>
  using predict_type =
  typename decision_tree_model<T>::template predict_type<DATA>;

  tree_ensemble_model() {}

  tree_ensemble_model(
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

  // string output
  std::string _str(const std::string&) const;

  // shortcuts
  bool is_classification_model() const {
    return algo == tree::algorithm::Classification;
  }
  bool is_regression_model() const {
    return algo == tree::algorithm::Regression;
  }
};

template <typename T>
std::string tree_ensemble_model<T>::_str(
  const std::string& ensemble_str
) const {
  std::ostringstream sout;

  sout << "-------- ";
  switch (algo) {
  case tree::algorithm::Classification:
    sout << "Classification";
    break;
  case tree::algorithm::Regression:
    sout << "Regression";
    break;
  }
  sout << " Trees (" << ensemble_str << "):: --------" << std::endl;
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
class random_forest_model : public tree_ensemble_model<T> {
  SERIALIZE(cereal::base_class<tree_ensemble_model<T>>(this))

public:
  template <typename DATA>
  using predict_type =
  typename tree_ensemble_model<T>::template predict_type<DATA>;

  random_forest_model() {}

  random_forest_model(
    const tree::algorithm algo,
    std::vector<decision_tree_model<T>> trees
  ) :
    tree_ensemble_model<T>(algo, std::move(trees))
  {}

  template <typename DATA>
  predict_type<DATA>
  predict(DATA&& x) const {
    switch (this->get_algo()) {
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
  std::string to_string() const { return this->_str("Random Forest"); }
  void debug_print() const { std::cout << to_string(); }

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
T random_forest_model<T>::predict_by_vote(
  const std::vector<T>& v
) const {
  std::unordered_map<T, size_t> voting_box;
  for (const auto& tree: this->get_trees()) {
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
  for (const auto& tree: this->get_trees()) { sum += tree.predict(v); }
  return sum / this->get_num_trees();
}

template <typename T>
std::vector<T> random_forest_model<T>::predict_by_vote(
  const rowmajor_matrix_local<T>& x
) const {
  const size_t num_records = x.local_num_row;
  std::vector<std::unordered_map<T, size_t>> voting_boxes(num_records);
  std::unordered_map<T, size_t>* vbox = voting_boxes.data();

  for (const auto& tree: this->get_trees()) {
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

  for (const auto& tree: this->get_trees()) {
    const std::vector<T> predicts = tree.predict(x);
    const T* src = predicts.data();
    for (size_t i = 0; i < num_records; i++) { dest[i] += src[i]; }
  }

  const size_t num_trees = this->get_num_trees();
  for (size_t i = 0; i < num_records; i++) { dest[i] /= num_trees; }

  return ret;
}

template <typename T>
class gradient_boosted_trees_model : public tree_ensemble_model<T> {
  std::vector<T> weights;
  SERIALIZE(cereal::base_class<tree_ensemble_model<T>>(this), weights)

public:
  template <typename DATA>
  using predict_type =
  typename tree_ensemble_model<T>::template predict_type<DATA>;

  gradient_boosted_trees_model() {}

  gradient_boosted_trees_model(
    const tree::algorithm algo,
    std::vector<decision_tree_model<T>> trees,
    std::vector<T> weights
  ) :
    tree_ensemble_model<T>(algo, std::move(trees)),
    weights(std::move(weights))
  {
    tree_assert(this->get_num_trees() == this->weights.size());
  }

  const std::vector<T>& get_tree_weights() const& { return weights; }
  std::vector<T> get_tree_weights() && { return std::move(weights); }

  template <typename DATA>
  predict_type<DATA>
  predict(DATA&& x) const {
    switch (this->get_algo()) {
    case tree::algorithm::Classification:
      return predict_as_binary(std::forward<DATA>(x));
    case tree::algorithm::Regression:
      return predict_as_rawvalue(std::forward<DATA>(x));
    default:
      throw std::logic_error("invalid tree algorithm");
    }
  }

  // TODO: implement
  std::vector<T> evaluate_each_iteration(
    const std::vector<T>& data, const T label,
    const tree::loss_type loss
  ) const {
    throw std::logic_error("NOT SUPPORTED YET");
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
  std::string to_string() const { return this->_str("GBTs"); }
  void debug_print() const { std::cout << to_string(); }

  node_local<gradient_boosted_trees_model<T>> broadcast() const {
    return make_node_local_broadcast(*this);
  }

private:
  // predict for a single dense-data point
  T predict_as_binary(const std::vector<T>&) const;
  T predict_as_rawvalue(const std::vector<T>&) const;

  // predict for the given dense-data point
  std::vector<T> predict_as_binary(const rowmajor_matrix_local<T>&) const;
  std::vector<T> predict_as_rawvalue(const rowmajor_matrix_local<T>&) const;

  template <typename Archive>
  void _save(const std::string& path) const {
    tree::save_model<gradient_boosted_trees_model<T>, Archive>(*this, path);
  }

  template <typename Archive>
  void _load(const std::string& path) {
    *this = tree::load_model<gradient_boosted_trees_model<T>, Archive>(path);
  }
};

template <typename T>
std::ostream& operator<<(
  std::ostream& os, const gradient_boosted_trees_model<T>& model
) {
  return os << model.to_string();
}

template <typename T>
T gradient_boosted_trees_model<T>::predict_as_binary(
  const std::vector<T>& v
) const {
  return (predict_as_rawvalue(v) > 0) ? 1 : -1;
}

template <typename T>
T gradient_boosted_trees_model<T>::predict_as_rawvalue(
  const std::vector<T>& v
) const {
  const size_t num_trees = this->get_num_trees();
  tree_assert(num_trees == weights.size());

  std::vector<T> inferences(num_trees, 0);
  T* infs = inferences.data();

  const auto* trees = this->get_trees().data();
_Pragma(__novector__)
  for (size_t t = 0; t < num_trees; t++) { infs[t] = trees[t].predict(v); }

  T ret = 0;
  const T* w = weights.data();
  for (size_t i = 0; i < num_trees; i++) { ret += w[i] * infs[i]; }

  return ret;
}

template <typename T>
std::vector<T> gradient_boosted_trees_model<T>::predict_as_binary(
  const rowmajor_matrix_local<T>& x
) const {
  const size_t num_records = x.local_num_row;
  std::vector<T> predicts = predict_as_rawvalue(x);
  tree_assert(num_records == predicts.size());

  T* preds = predicts.data();
  for (size_t i = 0; i < num_records; i++) {
    preds[i] = (preds[i] > 0) ? 1 : -1;
  }

  return predicts;
}

template <typename T>
std::vector<T> gradient_boosted_trees_model<T>::predict_as_rawvalue(
  const rowmajor_matrix_local<T>& x
) const {
  const size_t num_trees = this->get_num_trees();
  tree_assert(num_trees == weights.size());

  const size_t num_records = x.local_num_row;
  colmajor_matrix_local<T> predicts(num_records, num_trees);
  T* preds = predicts.val.data();

  const auto* trees = this->get_trees().data();
_Pragma(__novector__)
  for (size_t t = 0; t < num_trees; t++) {
    const std::vector<T> inferences = trees[t].predict(x);
    std::memcpy(
      preds + t * num_records, inferences.data(),
      sizeof(T) * num_records
    );
  }

  std::vector<T> ret(num_records, 0);
  gemv<T>(predicts, weights, ret, 'N', 1, 0);
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

template <typename T>
gradient_boosted_trees_model<T>
make_gbts_by_load(const std::string& path) {
  gradient_boosted_trees_model<T> model;
  model.load(path);
  return model;
}

template <typename T>
gradient_boosted_trees_model<T>
make_gbts_by_loadbinary(const std::string& path) {
  gradient_boosted_trees_model<T> model;
  model.loadbinary(path);
  return model;
}

} // end namespace frovedis

#endif
