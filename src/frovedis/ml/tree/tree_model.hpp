#ifndef _TREE_MODEL_HPP_
#define _TREE_MODEL_HPP_

#include <algorithm>
#include <iostream>
#include <iterator>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include "../../../frovedis.hpp"
#include "../../core/type_utility.hpp"
#include "../../matrix/rowmajor_matrix.hpp"
#include "../../matrix/crs_matrix.hpp"

#include "pragmas.hpp"
#include "bitcount.hpp"
#include "tree_assert.hpp"
#include "tree_config.hpp"

namespace frovedis {
namespace tree {

// a forward declaration of the helper class
template <typename T> class modelvector;

template <typename T>
class predict_pair {
  T val, prob;
  SERIALIZE(val, prob)

public:
  predict_pair(const T value = 0, const T probability = 0) :
    val(value), prob(probability)
  {}

  T get_predict() const { return val; }
  T get_probability() const { return prob; }
};

template <typename T>
class split {
  size_t findex;
  continuity fconty;
  std::vector<T> categories;
  T threshold;

  SERIALIZE(findex, fconty, categories, threshold)

public:
  split() {}

  split(
    const size_t feature_index,
    const continuity feature_continuity,
    std::vector<T> categories,
    const T threshold
  ) :
    findex(feature_index), fconty(feature_continuity),
    categories(std::move(categories)), threshold(threshold)
  {}

  // for compatibility
  split(
    const size_t feature_index,
    const T threshold,
    const continuity feature_continuity,
    std::vector<T> categories
  ) :
    split(
      feature_index, feature_continuity,
      std::move(categories), threshold
    )
  {}

  // shortcut ctors
  split(const size_t feature_index, std::vector<T> categories) :
    findex(feature_index),
    fconty(continuity::Categorical),
    categories(std::move(categories)), threshold()
  {}
  split(const size_t feature_index, const T threshold) :
    findex(feature_index),
    fconty(continuity::Continuous),
    categories(), threshold(threshold)
  {}

  size_t get_feature_index() const { return findex; }
  continuity get_feature_continuity() const { return fconty; }

  const std::vector<T>& get_categories() const& {
    ASSUME_CATEGORICAL();
    return categories;
  }
  std::vector<T> get_categories() && { // rvalue overload
    ASSUME_CATEGORICAL();
    return std::move(categories);
  }

  T get_threshold() const {
    ASSUME_CONTINUOUS();
    return threshold;
  }

  // for compatibility
  size_t get_feature() const { return get_feature_index(); }
  continuity get_feature_type() const {
    return get_feature_continuity();
  }

  // shortcuts
  bool is_categorical() const {
    return fconty == continuity::Categorical;
  }
  bool is_continuous() const {
    return fconty == continuity::Continuous;
  }

private:
  void ASSUME_CATEGORICAL() const {
    if (!is_categorical()) {
      throw std::logic_error("this feature is not categorical");
    }
  }
  void ASSUME_CONTINUOUS() const {
    if (!is_continuous()) {
      throw std::logic_error("this feature is not continuous");
    }
  }
};

template <typename T>
class information_gain_stats {
  T gain, impurity, left_impurity, right_impurity;
  predict_pair<T> left_predict, right_predict;

  SERIALIZE(
    gain, impurity, left_impurity, right_impurity,
    left_predict, right_predict
  )

public:
  information_gain_stats() {}

  information_gain_stats(
    const T gain, const T impurity,
    const T left_impurity, const T right_impurity,
    predict_pair<T> left_predict, predict_pair<T> right_predict
  ) :
    gain(gain), impurity(impurity),
    left_impurity(left_impurity), right_impurity(right_impurity),
    left_predict(std::move(left_predict)),
    right_predict(std::move(right_predict))
  {}

  T get_gain() const { return gain; }
  T get_impurity() const { return impurity; }
  T get_left_impurity() const { return left_impurity; }
  T get_right_impurity() const { return right_impurity; }

  const predict_pair<T>& get_left_predict() const& {
    return left_predict;
  }
  const predict_pair<T>& get_right_predict() const& {
    return right_predict;
  }

  // rvalue overloads
  predict_pair<T> get_left_predict() && {
    return std::move(left_predict);
  }
  predict_pair<T> get_right_predict() && {
    return std::move(right_predict);
  }
};

constexpr size_t ROOT_ID = 1;

class nodeid_helper {
  size_t id;
  SERIALIZE(id)

public:
  nodeid_helper(const size_t node_index) : id(node_index) {
    if (node_index < ROOT_ID) {
      throw std::invalid_argument("invalid node_index");
    }
  }

  bool is_root() const { return id == ROOT_ID; }
  bool is_left_child() const { return id % 2 == 0; }
  bool is_right_child() const { return !is_left_child() && !is_root(); }

  size_t get_index() const { return id; }
  size_t get_left_child_index() const { return id * 2; }
  size_t get_right_child_index() const {
    return get_left_child_index() + 1;
  }

  // the parent index of this node, or 0 if it is the root
  size_t get_parent_index() const { return id / 2; }
  // the depth of a tree which this node is in
  size_t get_depth() const { return ntzcount(hibit(id)); }

  // the index of the first node in the given depth
  static size_t start_index_in_depth(const size_t depth) {
    return static_cast<size_t>(1) << depth;
  }
  // the maximum number of nodes which CAN BE in the given depth
  static size_t max_nodes_in_depth(const size_t depth) {
    return start_index_in_depth(depth);
  }

  std::string to_string() const { return std::to_string(id); }
};

template <typename T>
class node : public std::enable_shared_from_this<node<T>> {
  nodeid_helper id;
  predict_pair<T> prediction;
  T impurity;
  bool leaf;
  std::shared_ptr<split<T>> split_ptr;
  std::shared_ptr<node<T>> left_ptr, right_ptr;
  std::shared_ptr<information_gain_stats<T>> stats_ptr;

  SERIALIZE(
    id, prediction, impurity, leaf,
    split_ptr, left_ptr, right_ptr, stats_ptr
  )

public:
  node(
    const size_t node_index,
    predict_pair<T> prediction,
    const T impurity,
    const bool leaf,
    const std::shared_ptr<split<T>>& split,
    const std::shared_ptr<node<T>>& left_node,
    const std::shared_ptr<node<T>>& right_node,
    const std::shared_ptr<information_gain_stats<T>>& stats
  ) :
    id(node_index),
    prediction(std::move(prediction)),
    impurity(impurity),
    leaf(leaf),
    split_ptr(split),
    left_ptr(left_node), right_ptr(right_node),
    stats_ptr(stats)
  {}

  node(
    const size_t node_index,
    predict_pair<T> prediction,
    const T impurity,
    const bool leaf = true
  ) :
    id(node_index),
    prediction(std::move(prediction)),
    impurity(impurity),
    leaf(leaf)
  {}

  size_t get_id() const { return id.get_index(); }
  const predict_pair<T>& get_predict() const& { return prediction; }
  predict_pair<T> get_predict() && { return std::move(prediction); }
  T get_impurity() const { return impurity; }

  bool is_leaf() const {
    tree_assert((left_ptr && right_ptr) || (!left_ptr && !right_ptr));
    return leaf || !(left_ptr && right_ptr);
  }

  std::shared_ptr<split<T>> get_split() const {
    ASSUME_INTERNAL_NODE();
    tree_assert(split_ptr);
    return split_ptr;
  }

  std::shared_ptr<information_gain_stats<T>> get_stats() const {
    ASSUME_INTERNAL_NODE();
    tree_assert(stats_ptr);
    return stats_ptr;
  }

  bool is_root() const { return id.is_root(); }
  bool is_left_child() const { return id.is_left_child(); }
  bool is_right_child() const { return id.is_right_child(); }

  size_t get_left_child_index() const {
    return id.get_left_child_index();
  }
  size_t get_right_child_index() const {
    return id.get_right_child_index();
  }

  std::shared_ptr<node<T>> get_left_node() const {
    ASSUME_INTERNAL_NODE();
    return left_ptr;
  }
  std::shared_ptr<node<T>> get_right_node() const {
    ASSUME_INTERNAL_NODE();
    return right_ptr;
  }

  size_t get_parent_index() const { return id.get_parent_index(); }
  size_t get_depth() const { return id.get_depth(); }

  std::shared_ptr<node<T>> search_node(const size_t) const;

/*
  template <typename DATA>
  decltype(predict(std::declval<DATA>())) predict(DATA&& x) const {
    return predict(std::forward<DATA>(x));
  }
*/

  // predict for a single data point
  T predict(const std::vector<T>& v) const {
    return predict_with_probability(v).get_predict();
  }

  // predict for a single sparse-data point
  template <typename I>
  T predict(const sparse_vector<T, I>& spv, const size_t num_feats) const {
    return predict_with_probability(spv, num_feats).get_predict();
  }

  // predict for the given data set
  std::vector<T> predict(const rowmajor_matrix_local<T>& x) const {
    const size_t num_records = x.local_num_row;
    const size_t num_feats = x.local_num_col;

    const T* valp = x.val.data();
    std::vector<T> ret(num_records, 0);
_Pragma(__novector__)
    for (size_t i = 0; i < num_records; i++) {
      ret[i] = predict_impl<const T*>(
        valp + i * num_feats, num_feats
      ).get_predict();
    }

    return ret;
  }

  // predict for the given sparse-data set
  template <typename I, typename O>
  std::vector<T> predict(const crs_matrix_local<T, I, O>& spx) const {
    const size_t num_records = spx.local_num_row;
    const size_t num_feats = spx.local_num_col;

    std::vector<T> ret(num_records, 0);
    for (size_t i = 0; i < num_records; i++) {
      ret[i] = predict(spx.get_row(i), num_feats);
    }

    return ret;
  }

  // with probability versions
  const predict_pair<T>& predict_with_probability(
    const std::vector<T>& v
  ) const {
    return predict_impl<const T*>(v.data(), v.size());
  }

  template <typename I>
  const predict_pair<T>& predict_with_probability(
    const sparse_vector<T, I>& spv, const size_t num_feats
  ) const {
    return predict_impl<const sparse_vector<T, I>&>(spv, num_feats);
  }

  std::vector<predict_pair<T>> predict_with_probability(
    const rowmajor_matrix_local<T>& x
  ) const {
    const size_t num_records = x.local_num_row;
    const size_t num_feats = x.local_num_col;

    const T* valp = x.val.data();
    std::vector<predict_pair<T>> ret(num_records, predict_pair<T>());
_Pragma(__novector__)
    for (size_t i = 0; i < num_records; i++) {
      ret[i] = predict_impl<const T*>(
        valp + i * num_feats, num_feats
      );
    }

    return ret;
  }

  template <typename I, typename O>
  std::vector<predict_pair<T>> predict_with_probability(
    const crs_matrix_local<T, I, O>& spx
  ) const {
    const size_t num_records = spx.local_num_row;
    const size_t num_feats = spx.local_num_col;

    std::vector<predict_pair<T>> ret(num_records, predict_pair<T>());
    for (size_t i = 0; i < num_records; i++) {
      ret[i] = predict_with_probability(spx.get_row(i), num_feats);
    }

    return ret;
  }

  std::string to_string(const std::string& = std::string()) const;

private:
  template <typename V>
  const predict_pair<T>& predict_impl(const V, const size_t) const;

  void ASSUME_INTERNAL_NODE() const {
    if (is_leaf()) {
      throw std::logic_error("this node is not a leaf");
    }
  }
};

// search a node who has the given node index
template <typename T>
std::shared_ptr<node<T>> node<T>::search_node(
  const size_t node_index
) const {
  if (get_id() == node_index) {
    // an explicit "this->" is required to call shared_from_this()
    // cf: https://gcc.gnu.org/wiki/VerboseDiagnostics#dependent_base
    return std::const_pointer_cast<node<T>>(this->shared_from_this());
  }

  nodeid_helper current_id(node_index);
  const size_t partial_depth = current_id.get_depth() - this->get_depth();
  tree_assert(partial_depth > 0);

  // construct a path from the target node to this node
  std::vector<bool> lefts;
  lefts.reserve(partial_depth);
  while (current_id.get_index() > this->get_id()) {
    lefts.push_back(current_id.is_left_child());
    const size_t parent_index = current_id.get_parent_index();
    current_id = nodeid_helper(parent_index);
  }
  tree_assert(lefts.size() == partial_depth);
  tree_assert(current_id.get_index() == get_id());

  // trace down from this node to the target node
  auto node_ptr = this->shared_from_this();
_Pragma(__novector__)
  for (auto left = lefts.crbegin(); left != lefts.crend(); left++) {
    if (node_ptr->is_leaf()) {
      throw std::runtime_error(
        std::string("no such node whose index is ")
        .append(std::to_string(node_index))
      );
    }

    // get the child
    if (*left) {
      node_ptr = node_ptr->get_left_node();
    } else {
      node_ptr = node_ptr->get_right_node();
    }
  }
  tree_assert(node_ptr->get_id() == node_index);

  return std::const_pointer_cast<node<T>>(node_ptr);
}

template <typename T>
T get_feature(const T* v, const size_t i) {
  return v[i];
}

template <typename T, typename I>
T get_feature(const sparse_vector<T, I>& spv, const size_t i) {
#if defined(_SX) || defined(__ve__)
  size_t j;
  const I* indices = spv.idx.data();
  for (j = 0; j < spv.idx.size(); j++) {
    if (i <= indices[j]) { break; }
  }

  tree_assert(j < spv.idx.size());
  tree_assert(j < spv.val.size());
  return (i == indices[j]) ? spv.val[j] : 0;
#else
  const auto itr = std::lower_bound(spv.idx.cbegin(), spv.idx.cend(), i);
  tree_assert(itr != spv.idx.cend());
  if (i == *itr) {
    const size_t j = std::distance(spv.idx.cbegin(), itr);
    tree_assert(j < spv.val.size());
    return spv.val[j];
  } else {
    return 0;
  }
#endif
}

template <typename T>
template <typename V>
const predict_pair<T>& node<T>::predict_impl(
  const V v, const size_t num_feats
) const {
  if (is_leaf()) { return prediction; }

  tree_assert(split_ptr->get_feature_index() < num_feats);
  const T fvalue = get_feature(v, split_ptr->get_feature_index());

  bool go_left = false;
  if (split_ptr->is_categorical()) {
#if defined(_SX) || defined(__ve__)
    const std::vector<T>& categories = split_ptr->get_categories();
    const size_t num_categories = categories.size();
    const T* src = categories.data();
    unsigned int temp = 0;
    for (size_t k = 0; k < num_categories; k++) {
      temp += static_cast<unsigned int>(fvalue == src[k]);
    }
    go_left = static_cast<bool>(temp);
#else
    for (const T category: split_ptr->get_categories()) {
      go_left = (fvalue == category);
      if (go_left) { break; }
    }
#endif
  } else if (split_ptr->is_continuous()) {
    go_left = (fvalue < split_ptr->get_threshold());
  } else {
    throw std::logic_error("no such feature continuity");
  }

  const auto child_ptr = go_left ? left_ptr : right_ptr;
  return child_ptr->template predict_impl<V>(v, num_feats);
}

template <typename T>
class metainfo {
  size_t height, num_nodes, maxnum_feats;
  SERIALIZE(height, num_nodes, maxnum_feats)

public:
  metainfo() {}
  metainfo(
    const size_t height, const size_t num_nodes,
    const size_t maxnum_features
  ) :
    height(height), num_nodes(num_nodes), maxnum_feats(maxnum_features)
  {}

  metainfo(const std::shared_ptr<node<T>>& top_node) {
    if (top_node) {
      *this = trace(top_node);
    } else {
      throw std::runtime_error("invalid top_node");
    }
  }

  size_t get_height() const { return height; }
  size_t get_num_nodes() const { return num_nodes; }
  size_t get_maxnum_features() const { return maxnum_feats; }

private:
  metainfo<T> trace(const std::shared_ptr<node<T>>& node_ptr) {
    tree_assert(node_ptr);
    if (node_ptr->is_leaf()) {
      return metainfo<T>(node_ptr->get_depth(), 1, 0);
    }

    // the max number of features for this node
    size_t currnum_feats = 1;
    const auto split_ptr = node_ptr->get_split();
    if (split_ptr->is_categorical()) {
      currnum_feats = split_ptr->get_categories().size();
      tree_assert(currnum_feats > 0);
    }

    // trace down recursively
    const auto left = trace(node_ptr->get_left_node());
    const auto right = trace(node_ptr->get_right_node());
    return metainfo<T>(
      std::max(left.height, right.height),
      left.num_nodes + right.num_nodes + 1,
      std::max({left.maxnum_feats, right.maxnum_feats, currnum_feats})
    );
  }
};

// shortcut functions
template <typename T, typename... Args>
std::shared_ptr<node<T>> make_node(Args&&... args) {
  return std::make_shared<node<T>>(std::forward<Args>(args)...);
}

template <typename T>
std::shared_ptr<node<T>> make_leaf(
  const size_t node_index,
  predict_pair<T> prediction,
  const T impurity,
  const bool leaf = true
) {
  return make_node<T>(node_index, std::move(prediction), impurity, leaf);
}

template <typename T, typename... Args>
std::shared_ptr<split<T>> make_split(Args&&... args) {
  return std::make_shared<split<T>>(std::forward<Args>(args)...);
}

template <typename T, typename... Args>
std::shared_ptr<information_gain_stats<T>> make_stats(Args&&... args) {
  return std::make_shared<information_gain_stats<T>>(
    std::forward<Args>(args)...
  );
}

} // end namespace tree

template <typename T>
class decision_tree_model {
  std::shared_ptr<tree::node<T>> root_ptr;
  size_t treeid;
  tree::algorithm algo;
  tree::metainfo<T> meta;

  SERIALIZE(root_ptr, treeid, algo, meta)

public:
  decision_tree_model() {}

  decision_tree_model(
    const std::shared_ptr<tree::node<T>>& top_node,
    const size_t treeid, const tree::algorithm algo
  ) :
    root_ptr(top_node), treeid(treeid), algo(algo), meta(top_node)
  {}

  decision_tree_model(
    const std::shared_ptr<tree::node<T>>& top_node,
    const tree::algorithm algo
  ) : decision_tree_model<T>(top_node, 0, algo) {}

  decision_tree_model(
    const std::shared_ptr<tree::node<T>>& top_node,
    const size_t treeid, const tree::algorithm algo,
    tree::metainfo<T> meta
  ) :
    root_ptr(top_node), treeid(treeid), algo(algo), meta(std::move(meta))
  {}

  size_t get_treeid() const { return treeid; }
  tree::algorithm get_algo() const { return algo; }
  size_t get_height() const { return meta.get_height(); }
  size_t get_num_nodes() const { return meta.get_num_nodes(); }
  size_t get_maxnum_features() const { return meta.get_maxnum_features(); }

  const tree::metainfo<T>& get_metainfo() const& { return meta; }
  tree::metainfo<T> get_metainfo() && { return std::move(meta); }

  std::shared_ptr<tree::node<T>> get_top_node() const {
    return root_ptr;
  }

  decision_tree_model<T> prune(const T min_info_gain) const {
    return decision_tree_model<T>(
      prune(root_ptr, min_info_gain), algo
    );
  }

  decision_tree_model<T>& inplace_prune(const T min_info_gain) {
    root_ptr = prune(root_ptr, min_info_gain);
    meta = tree::metainfo<T>(root_ptr);
    return *this;
  }

  // return a single predict if DATA is a single data point
  // return a std::vector<predict_pair<T>> if DATA is multi data points
  template <typename DATA>
  decltype(root_ptr->predict(std::declval<DATA>()))
  predict(DATA&& x) const {
    return root_ptr->predict(std::forward<DATA>(x));
  }

  // with probability versions
  template <typename DATA>
  decltype(root_ptr->predict_with_probability(std::declval<DATA>()))
  predict_with_probability(DATA&& x) const {
    return root_ptr->predict_with_probability(std::forward<DATA>(x));
  }

  // file IO
  void save(const std::string& path) const {
    tree::modelvector<T>(*this).saveline(path);
  }
  void load(const std::string& path) {
    tree::modelvector<T> vtree;
    vtree.loadline(path);
    *this = decision_tree_model<T>(vtree.restore());
  }
  void savebinary(const std::string& path) const {
    tree::modelvector<T>(*this).savebinary(path);
  }
  void loadbinary(const std::string& path) {
    tree::modelvector<T> vtree;
    vtree.loadbinary(path);
    *this = decision_tree_model<T>(vtree.restore());
  }

  node_local<decision_tree_model<T>> broadcast() const;

  // string output
  std::string to_string() const;
  void debug_print() const { std::cout << to_string(); }

  // shortcuts
  bool is_classification_tree() const {
    return algo == tree::algorithm::Classification;
  }
  bool is_regression_tree() const {
    return algo == tree::algorithm::Regression;
  }

private:
  std::shared_ptr<tree::node<T>> prune(
    const std::shared_ptr<tree::node<T>>&, const T
  ) const;
};

template <typename T>
decision_tree_model<T> tree_model_broadcast_helper(
  const tree::modelvector<T>& vtree
) {
  return vtree.restore();
}

template <typename T>
inline node_local<decision_tree_model<T>>
decision_tree_model<T>::broadcast() const {
  tree::modelvector<T> vtree(*this);
  auto bcas_vtree = vtree.broadcast();
  return bcas_vtree.map(tree_model_broadcast_helper<T>);
}

template <typename T>
std::shared_ptr<tree::node<T>> decision_tree_model<T>::prune(
  const std::shared_ptr<tree::node<T>>& node_ptr,
  const T min_info_gain
) const {
  if (node_ptr->is_leaf()) { return node_ptr; }

  const auto left_ptr = prune(
    node_ptr->get_left_node(), min_info_gain
  );
  const auto right_ptr = prune(
    node_ptr->get_right_node(), min_info_gain
  );

  if (
    left_ptr->is_leaf() && right_ptr->is_leaf() &&
    node_ptr->get_stats()->get_gain() <= min_info_gain
  ) {
    return tree::make_leaf<T>(
      node_ptr->get_id(),
      node_ptr->get_predict(),
      node_ptr->get_impurity()
    );
  } else {
    return tree::make_node<T>(
      node_ptr->get_id(), node_ptr->get_predict(),
      node_ptr->get_impurity(), node_ptr->is_leaf(),
      node_ptr->get_split(),
      left_ptr, right_ptr,
      node_ptr->get_stats()
    );
  }
}

template <typename T>
std::ostream& operator<<(
  std::ostream& os, const decision_tree_model<T>& model
) {
  return os << model.to_string();
}

template <typename T>
std::string decision_tree_model<T>::to_string() const {
  std::ostringstream sout;
  switch (algo) {
  case tree::algorithm::Classification:
    sout << "-------- Classification Tree:: --------" << std::endl;
    break;
  case tree::algorithm::Regression:
    sout << "-------- Regression Tree:: --------" << std::endl;
    break;
  }
  sout << "# of nodes: " << get_num_nodes() << ", ";
  sout << "height: " << get_height() << std::endl;
  sout << root_ptr->to_string();
  return sout.str();
}

template <typename T>
std::string tree::node<T>::to_string(const std::string& prefix) const {
  std::ostringstream sout;
  if (!is_root()) { sout << " \\_ "; }

  if (is_leaf()) {
    sout << "(" << get_id() << ") ";
    sout << "Predict: " << prediction.get_predict();
    const T probability = prediction.get_probability();
    if (probability > 0) {
      sout << " (" << probability * 100 << "%)";
    }
    sout << std::endl;
  } else {
    tree_assert(split_ptr && stats_ptr);

    sout << "<" << get_id() << "> ";

    sout << "Split: ";
    sout << "feature[" << split_ptr->get_feature_index() << "] ";
    if (split_ptr->is_categorical()) {
      const auto& categories = split_ptr->get_categories();
      sout << "in {";
      if (!categories.empty()) {
        sout << categories.front();
        for (size_t i = 1; i < categories.size(); i++) {
          sout << ", " << categories[i];
        }
      }
      sout << "}";
    } else if (split_ptr->is_continuous()) {
      sout << "< " << split_ptr->get_threshold();
    }
    sout << ", ";

    sout << "IG: " << stats_ptr->get_gain();
    sout << std::endl;

    sout << prefix << left_ptr->to_string(prefix + " | ");
    sout << prefix << right_ptr->to_string(prefix + "   ");
  }

  return sout.str();
}

// shortcut functions
template <typename T>
decision_tree_model<T> make_tree_by_load(const std::string& path) {
  decision_tree_model<T> model;
  model.load(path);
  return model;
}

template <typename T>
decision_tree_model<T> make_tree_by_loadbinary(const std::string& path) {
  decision_tree_model<T> model;
  model.loadbinary(path);
  return model;
}

/*
TODO: fix wrong specialization
// specialize for broadcast
template <typename T>
node_local<decision_tree_model<T>> make_node_local_broadcast(
  const decision_tree_model<T>& model
) {
  return model.broadcast();
}
*/

} // end namespace frovedis

// tree model helper
#include "tree_vector.hpp"

#endif
