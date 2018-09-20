#ifndef _TREE_VECTOR_HPP_
#define _TREE_VECTOR_HPP_

#include <algorithm>
#include <initializer_list>
#include <iterator>
#include <memory>
#include <stack>
#include <stdexcept>
#include <string>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

#include "../../../frovedis.hpp"
#include "../../core/type_utility.hpp"

#include "pragmas.hpp"
#include "tree_assert.hpp"
#include "tree_config.hpp"
#include "tree_model.hpp"

namespace frovedis {
namespace tree {

constexpr size_t VVID = 201802;
constexpr bool LEAF_NODE = true;
constexpr bool INTERNAL_NODE = !LEAF_NODE;

template <typename... Args>
inline void __expand(const Args&...) {}

template <typename T, enable_if_uint<T> = nullptr>
struct allreserve_helper {
  allreserve_helper(const T n) : n(n) {}

  template <typename Vec>
  T operator()(Vec& vec) const {
    vec.reserve(n);
    return 0;
  }

  T n;
};

template <typename... Vecs>
void allreserve(const size_t n, Vecs&... vecs) {
  allreserve_helper<size_t> reserver(n);
  __expand(reserver(vecs)...);
}

template <typename T>
struct allappend_helper {
  allappend_helper(const T val) : val(val) {}

  template <typename Vec>
  T operator()(Vec& vec) const {
    vec.push_back(static_cast<typename Vec::value_type>(val));
    return 0;
  }

  T val;
};

template <typename T, typename... Vecs>
void allappend(const T value, Vecs&... vecs) {
  allappend_helper<T> appender(value);
  __expand(appender(vecs)...);
}

template <typename Vec>
size_t get_size(const Vec& vec) { return vec.size(); }

template <typename... Vecs>
bool samesize(const size_t n, const Vecs&... vecs) {
  std::initializer_list<size_t> list = { get_size(vecs)... };
  return std::all_of(
    list.begin(), list.end(),
    [n] (const size_t vsize) { return vsize == n; }
  );
}

template <typename Vec, typename... Vecs>
bool samesize(const Vec& vec, const Vecs&... vecs) {
  return samesize(vec.size(), vecs...);
}

template <typename T>
dvector<T> make_dvector_local(const std::vector<T>& vec) {
  auto temp = make_node_local_allocate<std::vector<T>>();
  temp.put(0, vec);
  return temp.template moveto_dvector<T>();
}

template <typename... Paths>
class saveload_helper {
  std::string dir;
  std::tuple<Paths...> paths;

public:
  saveload_helper(const std::string& dir, Paths&&... paths) :
    dir(dir), paths(std::forward<Paths>(paths)...)
  {}

  template <typename... Vecs>
  void saveline(const Vecs&... vecs) const {
    make_directory(dir);
    _saveline<0, Vecs...>(vecs...);
  }

  template <typename... Vecs>
  void loadline(Vecs&... vecs) const {
    if (!directory_exists(dir)) {
      throw std::runtime_error("no such directory: " + dir);
    }
    _loadline<0, Vecs...>(vecs...);
  }

  template <typename... Vecs>
  void savebinary(const Vecs&... vecs) const {
    make_directory(dir);
    _savebinary<0, Vecs...>(vecs...);
  }

  template <typename... Vecs>
  void loadbinary(Vecs&... vecs) const {
    if (!directory_exists(dir)) {
      throw std::runtime_error("no such directory: " + dir);
    }
    _loadbinary<0, Vecs...>(vecs...);
  }

private:
  template <size_t N, typename Vec, typename... Vecs>
  void _saveline(const Vec& vec, const Vecs&... vecs) const {
    _saveline<N, Vec>(vec);
    _saveline<N + 1, Vecs...>(vecs...);
  }

  template <size_t N, typename Vec>
  void _saveline(const Vec& vec) const {
    make_dvector_local(vec).saveline(dir + "/" + std::get<N>(paths));
  }

  template <size_t N, typename Vec, typename... Vecs>
  void _loadline(Vec& vec, Vecs&... vecs) const {
    _loadline<N, Vec>(vec);
    _loadline<N + 1, Vecs...>(vecs...);
  }

  template <size_t N, typename Vec>
  void _loadline(Vec& vec) const {
    using T = typename Vec::value_type;
    const std::string path(dir + "/" + std::get<N>(paths));
    vec = make_dvector_loadline<T>(path).gather();
  }

  template <size_t N, typename Vec, typename... Vecs>
  void _savebinary(const Vec& vec, const Vecs&... vecs) const {
    _savebinary<N, Vec>(vec);
    _savebinary<N + 1, Vecs...>(vecs...);
  }

  template <size_t N, typename Vec>
  void _savebinary(const Vec& vec) const {
    make_dvector_local(vec).savebinary(dir + "/" + std::get<N>(paths));
  }

  template <size_t N, typename Vec, typename... Vecs>
  void _loadbinary(Vec& vec, Vecs&... vecs) const {
    _loadbinary<N, Vec>(vec);
    _loadbinary<N + 1, Vecs...>(vecs...);
  }

  template <size_t N, typename Vec>
  void _loadbinary(Vec& vec) const {
    using T = typename Vec::value_type;
    const std::string path(dir + "/" + std::get<N>(paths));
    vec = make_dvector_loadbinary<T>(path).gather();
  }
};

template <typename... Args>
saveload_helper<decltype(std::string(std::declval<Args>()))...>
make_saveload_helper(const std::string& dir, Args&&... args) {
  return saveload_helper<decltype(std::string(args))...>(
    dir, std::string(std::forward<Args>(args))...
  );
}

// ---------------------------------------------------------------------

// a forward declaration
template <typename T> class modelvector;

template <typename T>
class splitvector {
  friend class modelvector<T>;

  std::vector<size_t> indices;
  std::vector<int> disables, conties;
  std::vector<size_t> heads, tails;
  std::vector<T> fvalues;

  SERIALIZE(indices, disables, conties, heads, tails, fvalues)

public:
  splitvector() {}

  splitvector(
    std::vector<size_t>&& indices,
    std::vector<int>&& disables, std::vector<int>&& conties,
    std::vector<size_t>&& heads, std::vector<size_t>&& tails,
    std::vector<T>&& feature_values
  ) :
    indices(std::move(indices)),
    disables(std::move(disables)), conties(std::move(conties)),
    heads(std::move(heads)), tails(std::move(tails)),
    fvalues(std::move(feature_values))
  {
    ASSERT_SIZE();
  }

  size_t size() const {
    ASSERT_SIZE();
    return indices.size();
  }

  void reserve(const size_t n) {
    allreserve(n, indices, disables, conties, heads, tails, fvalues);
  }

  void push_back_empty() {
    allappend(0, indices, disables, conties, heads, tails);
  }

  void push_back(const split<T>& s) {
    // note: splitvector is a friend of split
    indices.push_back(s.findex);
    disables.push_back(0);
    conties.push_back(static_cast<int>(s.fconty));

    heads.push_back(fvalues.size());
    if (s.is_categorical()) {
      const std::vector<T>& categories = s.categories;
      const size_t num_categories = categories.size();
_Pragma(__novector__)
      for (size_t k = 0; k < num_categories; k++) {
        fvalues.push_back(categories[k]);
      }
    } else if (s.is_continuous()) {
      fvalues.push_back(s.threshold);
    } else {
      throw std::logic_error("invalid continuity");
    }
    tails.push_back(fvalues.size());
  }

  // pseudo emplace back
  template <typename... Args>
  void emplace_back(Args&&... args) {
    push_back(split<T>(std::forward<Args>(args)...));
  }

  std::shared_ptr<split<T>> get(const size_t j) const {
    if (is_categorical(j)) {
      return make_split<T>(get_feature_index(j), get_categories(j));
    } else if (is_continuous(j)) {
      return make_split<T>(get_feature_index(j), get_threshold(j));
    } else {
      throw std::logic_error("invalid continuity");
    }
  }

  size_t get_feature_index(const size_t j) const {
    tree_assert(j < size());
    return indices[j];
  }

  continuity get_feature_continuity(const size_t j) const {
    tree_assert(j < size());
    return static_cast<continuity>(conties[j]);
  }

  std::vector<T> get_categories(const size_t j) const {
    tree_assert(is_categorical(j));
    tree_assert(heads[j] < tails[j]);
    tree_assert(tails[j] <= fvalues.size());
#if defined(_SX) || defined(__ve__)
    const size_t head = heads[j];
    const size_t tail = tails[j];
    const size_t num_categories = tail - head;
    std::vector<T> ret(num_categories, 0);

    const T* src = fvalues.data() + head;
    T* dst = ret.data();
    for (size_t k = 0; k < num_categories; k++) { dst[k] = src[k]; }

    return ret;
#else
    auto origin = fvalues.cbegin();
    auto head = std::next(origin, heads[j]);
    auto tail = std::next(origin, tails[j]);
    return std::vector<T>(head, tail);
#endif
  }

  T get_threshold(const size_t j) const {
    tree_assert(is_continuous(j));
    tree_assert(heads[j] < fvalues.size());
    tree_assert(heads[j] + 1 == tails[j]);
    return fvalues[heads[j]];
  }

  bool is_categorical(const size_t j) const {
    return get_feature_continuity(j) == continuity::Categorical;
  }
  bool is_continuous(const size_t j) const {
    return get_feature_continuity(j) == continuity::Continuous;
  }

  node_local<splitvector<T>> broadcast() const;

private:
  void ASSERT_SIZE() const {
    tree_assert(samesize(indices, disables, conties, heads, tails));
  }
};

template <typename T>
struct splitvector_broadcast_helper {
  template <typename... Args>
  splitvector<T> operator()(Args&&... args) const {
    return splitvector<T>(std::move(args)...);
  }

  SERIALIZE_NONE
};

template <typename T>
inline node_local<splitvector<T>> splitvector<T>::broadcast() const {
  auto bcas_indices = make_node_local_broadcast(indices);
  return bcas_indices.map(
    splitvector_broadcast_helper<T>(),
    make_node_local_broadcast(disables),
    make_node_local_broadcast(conties),
    make_node_local_broadcast(heads),
    make_node_local_broadcast(tails),
    make_node_local_broadcast(fvalues)
  );
}

template <typename T>
class modelvector {
  algorithm algo;
  metainfo<T> meta;
  std::vector<size_t> ids;
  std::vector<T> preds, probs, imprs;
  std::vector<int> leaves;
  std::vector<T> gains;
  splitvector<T> splits;

  SERIALIZE(
    algo, meta, ids, preds, probs, imprs, leaves, gains, splits
  )

public:
  modelvector() {}

  modelvector(const decision_tree_model<T>& model) :
    algo(model.get_algo()),
    meta(model.get_height(), model.get_num_nodes()),
    ids(), preds(), probs(), imprs(), leaves(), gains(), splits()
  {
    reserve(model.get_num_nodes());
    _zip(model.get_top_node());
    ASSERT_SIZE();
  }

  modelvector(
    const algorithm algo, metainfo<T>&& meta,
    std::vector<size_t>&& ids,
    std::vector<T>&& preds, std::vector<T>&& probs,
    std::vector<T>&& imprs, std::vector<int>&& leaves,
    std::vector<T>&& gains, splitvector<T>&& splits
  ) :
    algo(algo), meta(std::move(meta)),
    ids(std::move(ids)),
    preds(std::move(preds)),
    probs(std::move(probs)),
    imprs(std::move(imprs)),
    leaves(std::move(leaves)),
    gains(std::move(gains)),
    splits(std::move(splits))
  {
    ASSERT_SIZE();
  }

  void reserve(const size_t n) {
    allreserve(n, ids, preds, probs, imprs, leaves, gains, splits);
  }

  std::vector<size_t> metavector() const {
    return {
      VVID, static_cast<size_t>(algo),
      meta.get_height(), meta.get_num_nodes()
    };
  }

  decision_tree_model<T> unzip() const;
  node_local<modelvector<T>> broadcast() const;

  void save(const std::string&) const;
  void load(const std::string&);
  void savebinary(const std::string&) const;
  void loadbinary(const std::string&);

private:
  void _zip(const std::shared_ptr<node<T>>&);

  void check_size() const {
    if (
      !samesize(
        meta.get_num_nodes(),
        ids, preds, probs, imprs, leaves, gains, splits
      )
    ) {
      throw std::runtime_error("invalid vector length");
    }
  }

  template <typename Cond>
  void check_structure(const Cond& condition) const {
    if (!condition) {
      throw std::runtime_error("invalid tree structre");
    }
  }

  void ASSERT_SIZE() const {
    tree_assert(meta.get_num_nodes() > 0);
    tree_assert(
      samesize(
        meta.get_num_nodes(),
        ids, preds, probs, imprs, leaves, gains, splits
      )
    );
  }
};

template <typename T>
struct modelvector_broadcast_helper {
  modelvector_broadcast_helper() {}
  modelvector_broadcast_helper(
    const algorithm algo, const metainfo<T>& meta
  ) :
    algo(algo), meta(meta)
  {}

  template <typename... Args>
  modelvector<T> operator()(Args&&... args) {
    return modelvector<T>(algo, std::move(meta), std::move(args)...);
  }

  algorithm algo;
  metainfo<T> meta;
  SERIALIZE(algo, meta)
};

template <typename T>
inline node_local<modelvector<T>> modelvector<T>::broadcast() const {
  auto bcas_ids = make_node_local_broadcast(ids);
  return bcas_ids.map(
    modelvector_broadcast_helper<T>(algo, meta),
    make_node_local_broadcast(preds),
    make_node_local_broadcast(probs),
    make_node_local_broadcast(imprs),
    make_node_local_broadcast(leaves),
    make_node_local_broadcast(gains),
    splits.broadcast()
  );
}

template <typename T>
void modelvector<T>::_zip(const std::shared_ptr<node<T>>& node_ptr) {
  tree_assert(node_ptr);
  if (!node_ptr->is_leaf()) {
    _zip(node_ptr->get_left_node());
    _zip(node_ptr->get_right_node());
  }

  ids.push_back(node_ptr->get_id());

  const auto& predprob = node_ptr->get_predict();
  preds.push_back(predprob.get_predict());
  probs.push_back(predprob.get_probability());
  imprs.push_back(node_ptr->get_impurity());

  if (node_ptr->is_leaf()) {
    leaves.push_back(LEAF_NODE);
    gains.push_back(0);
    splits.push_back_empty();
  } else {
    tree_assert(node_ptr->get_stats());
    tree_assert(node_ptr->get_split());
    leaves.push_back(INTERNAL_NODE);
    gains.push_back(node_ptr->get_stats()->get_gain());
    splits.push_back(*(node_ptr->get_split()));
  }
}

template <typename T>
inline decision_tree_model<T> modelvector<T>::unzip() const {
  ASSERT_SIZE();
  const size_t num_nodes = meta.get_num_nodes();
  std::stack<std::shared_ptr<node<T>>> left_ptrs;
  std::shared_ptr<node<T>> right_ptr;

  for (size_t i = 0; i < num_nodes; i++) {
    const nodeid_helper id(ids[i]);
    predict_pair<T> pred(preds[i], probs[i]);
    std::shared_ptr<node<T>> node_ptr;

    if (leaves[i]) {
      check_structure(!right_ptr);
      node_ptr = make_leaf<T>(
        id.get_index(), std::move(pred), imprs[i]
      );
    } else {
      check_structure(!left_ptrs.empty());
      check_structure(right_ptr);

      std::shared_ptr<node<T>> left_ptr = left_ptrs.top();
      node_ptr = make_node<T>(
        id.get_index(), std::move(pred), imprs[i],
        INTERNAL_NODE,
        splits.get(i), left_ptr, right_ptr,
        make_stats<T>(
          gains[i], imprs[i],
          left_ptr->get_impurity(), right_ptr->get_impurity(),
          left_ptr->get_predict(), right_ptr->get_predict()
        )
      );

      left_ptrs.pop();
      right_ptr.reset();
    }

    if (id.is_left_child()) {
      left_ptrs.push(std::move(node_ptr));
    } else if (id.is_right_child()) {
      check_structure(!right_ptr);
      right_ptr = std::move(node_ptr);
    } else if (id.is_root()) {
      check_structure(left_ptrs.empty());
      check_structure(!right_ptr);
      check_structure(i == num_nodes - 1);
      return decision_tree_model<T>(node_ptr, algo, meta);
    } else {
      throw std::logic_error("invalid node type");
    }
  }

  check_structure(false);
  throw std::logic_error("should not reach here");
}

template <typename T>
void modelvector<T>::save(const std::string& dir) const {
  make_saveload_helper(
    dir, "metainfo", "nodeid", "prediction", "probability",
    "impurity", "leaf", "infogain",
    "findex", "disable", "continuity", "head", "tail", "fvalue"
  ).saveline(
    metavector(), ids, preds, probs, imprs, leaves, gains,
    splits.indices, splits.disables, splits.conties,
    splits.heads, splits.tails, splits.fvalues
  );
}

template <typename T>
void modelvector<T>::load(const std::string& dir) {
  std::vector<size_t> metavec;
  make_saveload_helper(
    dir, "metainfo", "nodeid", "prediction", "probability",
    "impurity", "leaf", "infogain",
    "findex", "disable", "continuity", "head", "tail", "fvalue"
  ).loadline(
    metavec, ids, preds, probs, imprs, leaves, gains,
    splits.indices, splits.disables, splits.conties,
    splits.heads, splits.tails, splits.fvalues
  );

  if (metavec.at(0) != VVID) {
    throw std::runtime_error("vectorized version ID is different");
  }

  algo = static_cast<algorithm>(metavec.at(1));
  meta = metainfo<T>(metavec.at(2), metavec.at(3));

  check_size();
}

template <typename T>
void modelvector<T>::savebinary(const std::string& dir) const {
  make_saveload_helper(
    dir, "metainfo", "nodeid", "prediction", "probability",
    "impurity", "leaf", "infogain",
    "findex", "disable", "continuity", "head", "tail", "fvalue"
  ).savebinary(
    metavector(), ids, preds, probs, imprs, leaves, gains,
    splits.indices, splits.disables, splits.conties,
    splits.heads, splits.tails, splits.fvalues
  );
}

template <typename T>
void modelvector<T>::loadbinary(const std::string& dir) {
  std::vector<size_t> metavec;
  make_saveload_helper(
    dir, "metainfo", "nodeid", "prediction", "probability",
    "impurity", "leaf", "infogain",
    "findex", "disable", "continuity", "head", "tail", "fvalue"
  ).loadbinary(
    metavec, ids, preds, probs, imprs, leaves, gains,
    splits.indices, splits.disables, splits.conties,
    splits.heads, splits.tails, splits.fvalues
  );

  if (metavec.at(0) != VVID) {
    throw std::runtime_error("vectorized version ID is different");
  }

  algo = static_cast<algorithm>(metavec.at(1));
  meta = metainfo<T>(metavec.at(2), metavec.at(3));

  check_size();
}

} // end namespace tree

// specialize for broadcast
template <typename T>
node_local<tree::splitvector<T>> make_node_local_broadcast(
  const tree::splitvector<T>& splits
) {
  return splits.broadcast();
}

template <typename T>
node_local<tree::modelvector<T>> make_node_local_broadcast(
  const tree::modelvector<T>& vtree
) {
  return vtree.broadcast();
}

} // end namespace frovedis

#endif
