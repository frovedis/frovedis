#ifndef _TREE_VECTOR_HPP_
#define _TREE_VECTOR_HPP_

#include <array>
#include <iostream>
#include <memory>
#include <queue>
#include <sstream>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include "../../../frovedis.hpp"

#include "tree_assert.hpp"
#include "tree_config.hpp"
#include "tree_model.hpp"
#include "tree_utility.hpp"

namespace frovedis {
namespace tree {

constexpr size_t VVID = 201902;

constexpr bool LEAF_NODE = true;
constexpr bool INTERNAL_NODE = !LEAF_NODE;

constexpr size_t LEN_METAVEC = 7;
constexpr size_t NUM_FIXFCOLS = 4;
constexpr size_t NUM_ICOLS = 7;

template <typename T>
dvector<T> make_dvector_local(const std::vector<T>& vec) {
  auto temp = make_node_local_allocate<std::vector<T>>();
  temp.put(0, vec);
  return temp.template moveto_dvector<T>();
}

template <typename T>
class modelvector {
  using U = decltype(f2uint(std::declval<T>()));

  size_t treeid;
  algorithm algo;

  size_t num_rows, num_fcols;
  std::vector<T> fvec;
  std::vector<size_t> ivec;

  SERIALIZE(treeid, algo, num_rows, num_fcols, fvec, ivec)

public:
  modelvector() {}
  modelvector(const decision_tree_model<T>&);

  modelvector(
    const size_t treeid, const algorithm algo,
    const size_t num_rows, const size_t num_fcols,
    const std::vector<T>& fvec, const std::vector<size_t>& ivec
  ) :
    treeid(treeid), algo(algo),
    num_rows(num_rows), num_fcols(num_fcols),
    fvec(fvec), ivec(ivec)
  {
    tree_assert(fvec.size() == num_rows * num_fcols);
    tree_assert(ivec.size() == num_rows * NUM_ICOLS);
  }

  decision_tree_model<T> restore() const;
  node_local<modelvector<T>> broadcast() const;

  void saveline(const std::string&) const;
  void loadline(const std::string&);
  void savebinary(const std::string&) const;
  void loadbinary(const std::string&);

  std::string to_string() const;
  void debug_print() const { std::cout << to_string(); }

private:
  std::array<size_t, LEN_METAVEC> metavector() const {
    constexpr size_t ONE = 1;
    return make_array<size_t>(
      ONE, ONE, VVID,
      treeid, static_cast<size_t>(algo),
      num_rows, num_fcols
    );
  }
};

template <typename T>
modelvector<T>::modelvector(const decision_tree_model<T>& model) :
  treeid(model.get_treeid()), algo(model.get_algo()),
  num_rows(model.get_num_nodes()),
  num_fcols(NUM_FIXFCOLS + model.get_maxnum_features()),
  fvec(num_rows * num_fcols, 0),
  ivec(num_rows * NUM_ICOLS, 0)
{
  tree_assert(NUM_FIXFCOLS < num_fcols);

  // breadth-first serialization
  std::queue<std::shared_ptr<node<T>>> ptrs;
  ptrs.push(model.get_top_node());

  T* forigin = fvec.data();
  size_t* iorigin = ivec.data();
  for (size_t rowid = 0; !ptrs.empty(); rowid++, ptrs.pop()) {
    const auto node_ptr = ptrs.front();
    tree_assert(node_ptr);
    tree_assert(rowid < num_rows);

    T* fdst = forigin + rowid * num_fcols;
    size_t* idst = iorigin + rowid * NUM_ICOLS;
    size_t j = 0, k = 0;

    const auto pred = node_ptr->get_predict();
    idst[j++] = node_ptr->get_id();
    fdst[k++] = pred.get_predict();
    fdst[k++] = pred.get_probability();
    fdst[k++] = node_ptr->get_impurity();

    if (node_ptr->is_leaf()) {
      idst[j++] = LEAF_NODE;
      //tree_assert(j == 2 && k == 3);
      continue;
    }

    tree_assert(node_ptr->get_stats());
    tree_assert(node_ptr->get_split());

    const auto split_ptr = node_ptr->get_split();
    idst[j++] = INTERNAL_NODE;
    idst[j++] = node_ptr->get_left_child_index();
    idst[j++] = node_ptr->get_right_child_index();
    fdst[k++] = node_ptr->get_stats()->get_gain();
    idst[j++] = split_ptr->get_feature_index();
    idst[j++] = static_cast<size_t>(split_ptr->get_feature_continuity());
    tree_assert(k == NUM_FIXFCOLS);

    if (split_ptr->is_categorical()) {
      const auto categories = split_ptr->get_categories();
      const size_t num_categories = categories.size();
      tree_assert(NUM_FIXFCOLS + num_categories <= num_cols);

      const T* catsrc = categories.data();
      T* catdst = fdst + NUM_FIXFCOLS;

      idst[j++] = num_categories;
      for (size_t c = 0; c < num_categories; c++) {
        catdst[c] = catsrc[c];
      }
    } else if (split_ptr->is_continuous()) {
      idst[j++] = 1;
      fdst[k++] = split_ptr->get_threshold();
    } else {
      throw std::logic_error("invalid continuity");
    }

    tree_assert(j == NUM_ICOLS);
    ptrs.push(node_ptr->get_left_node());
    ptrs.push(node_ptr->get_right_node());
  }
}

template <typename T>
decision_tree_model<T> modelvector<T>::restore() const {
  tree_assert(fvec.size() == num_rows * num_fcols);
  tree_assert(ivec.size() == num_rows * NUM_ICOLS);

  std::queue<std::shared_ptr<node<T>>> ptrs;

  const T* forigin = fvec.data();
  const size_t* iorigin = ivec.data();
  for (size_t i = num_rows; i > 0; i--) {
    const size_t rowid = i - 1;
    const T* fsrc = forigin + rowid * num_fcols;
    const size_t* isrc = iorigin + rowid * NUM_ICOLS;

    size_t j = 0, k = 0;
    const size_t id = isrc[j++];
    const T predict = fsrc[k++];
    const T probability = fsrc[k++];
    const T impurity = fsrc[k++];
    predict_pair<T> pred(predict, probability);

    if (probability < 0 || 1 < probability) {
      RLOG(WARNING) << "WARNING: " <<
        "<nodeid:" << id << "> wrong probability: " << probability <<
      std::endl;
    }

    if (isrc[j++] > 0) {
      // leaf node
      ptrs.push(make_leaf<T>(id, std::move(pred), impurity));
      //tree_assert(j == 2 && k == 3);
      continue;
    }

    // internal node
    tree_assert(ptrs.size() >= 2);
    std::shared_ptr<node<T>> right_ptr(ptrs.front());
    ptrs.pop();
    std::shared_ptr<node<T>> left_ptr(ptrs.front());
    ptrs.pop();

    if (!(left_ptr->is_left_child() && right_ptr->is_right_child())) {
      throw std::runtime_error("invalid tree structure");
    }

    if (
      (id != left_ptr->get_parent_index()) ||
      (id != right_ptr->get_parent_index())
    ) {
      std::ostringstream sout;
      sout << "invalid tree structure: {";
      sout << "nodeid: " << id << ", ";
      sout << "leftid: " << left_ptr->get_id() << ", ";
      sout << "rightid: " << right_ptr->get_id() << "}";
      throw std::runtime_error(sout.str());
    }

    const size_t leftid = isrc[j++];
    const size_t rightid = isrc[j++];

    if (
      (leftid != left_ptr->get_id()) || (rightid != right_ptr->get_id())
    ) {
      RLOG(WARNING) << "WARNING: " <<
        "inconsistent node ids are ignored: {" <<
        "nodeid: " << id << ", " <<
        "leftid: " << leftid << ", " <<
        "rightid: " << rightid << "}" <<
      std::endl;
    }

    const T gain = fsrc[k++];
    const size_t findex = isrc[j++];
    const auto fconty = static_cast<continuity>(isrc[j++]);
    tree_assert(k == NUM_FIXFCOLS);

    std::shared_ptr<split<T>> split_ptr;

    if (fconty == continuity::Categorical) {
      const size_t num_categories = isrc[j++];
      if (num_fcols < NUM_FIXFCOLS + num_categories) {
        std::ostringstream sout;
        sout << "<nodeid:" << id << "> ";
        sout << "the number of categories is invalid: " << num_categories;
        throw std::runtime_error(sout.str());
      }

      std::vector<T> categories(num_categories, 0);
      const T* catsrc = fsrc + NUM_FIXFCOLS;
      T* catdst = categories.data();

      for (size_t c = 0; c < num_categories; c++) {
        catdst[c] = catsrc[c];
      }

      split_ptr = make_split<T>(findex, std::move(categories));
    } else if (fconty == continuity::Continuous) {
      const size_t fnum = isrc[j++];
      const T threshold = fsrc[k++];

      if (fnum != 1) {
        RLOG(WARNING) << "WARNING: " <<
          "<nodeid:" << id << "> " <<
          "the inconsistent number of features is ignored: " << fnum <<
        std::endl;
      }

      split_ptr = make_split<T>(findex, threshold);
    } else {
      std::ostringstream sout;
      sout << "<nodeid:" << id << "> invalid continuity";
      throw std::runtime_error(sout.str());
    }

    tree_assert(j == NUM_ICOLS);
    ptrs.push(
      make_node<T>(
        id, std::move(pred), impurity, INTERNAL_NODE,
        std::move(split_ptr), std::move(left_ptr), std::move(right_ptr),
        make_stats<T>(
          gain, impurity,
          left_ptr->get_impurity(), right_ptr->get_impurity(),
          left_ptr->get_predict(), right_ptr->get_predict()
        )
      )
    );
  }

  if (ptrs.size() != 1) {
    throw std::runtime_error("there are too many nodes");
  } else if (!ptrs.front()->is_root()) {
    throw std::runtime_error("the first node is not a root");
  }

  return decision_tree_model<T>(ptrs.front(), algo);
}

template <typename T>
struct modelvector_broadcast_helper {
  modelvector_broadcast_helper() {}
  modelvector_broadcast_helper(
    const size_t treeid, const algorithm algo,
    const size_t num_rows, const size_t num_fcols
  ) :
    treeid(treeid), algo(algo), num_rows(num_rows), num_fcols(num_fcols)
  {}

  modelvector<T> operator()(
    const std::vector<T>& fvec, const std::vector<size_t>& ivec
  ) const {
    return modelvector<T>(treeid, algo, num_rows, num_fcols, fvec, ivec);
  }

  size_t treeid;
  algorithm algo;
  size_t num_rows, num_fcols;
  SERIALIZE(treeid, algo, num_rows, num_fcols)
};

template <typename T>
inline node_local<modelvector<T>> modelvector<T>::broadcast() const {
  return make_node_local_broadcast(fvec).map(
    modelvector_broadcast_helper<T>(treeid, algo, num_rows, num_fcols),
    make_node_local_broadcast(ivec)
  );
}

template <typename T>
void modelvector<T>::saveline(const std::string& path) const {
  const auto metavec = metavector();
  const size_t* msrc = metavec.data();
  const T* fsrc = fvec.data();
  const size_t* isrc = ivec.data();

  tree_assert(metavec.size() == LEN_METAVEC);
  tree_assert(fvec.size() == num_rows * num_fcols);
  tree_assert(ivec.size() == num_rows * NUM_ICOLS);

  std::vector<T> vec(metavec.size() + fvec.size() + ivec.size(), 0);
  T* mdst = vec.data();
  T* fdst = vec.data() + metavec.size();
  T* idst = vec.data() + metavec.size() + fvec.size();

  for (size_t i = 0; i < metavec.size(); i++) {
    mdst[i] = static_cast<T>(msrc[i]);
  }
  for (size_t i = 0; i < fvec.size(); i++) {
    fdst[i] = fsrc[i];
  }
  for (size_t i = 0; i < ivec.size(); i++) {
    idst[i] = static_cast<T>(isrc[i]);
  }

  make_dvector_local(vec).saveline(path);
}

template <typename T>
void modelvector<T>::loadline(const std::string& path) {
  const auto vec = make_dvector_loadline<T>(path).gather();
  if (vec.size() < LEN_METAVEC) {
    throw std::runtime_error("invalid input");
  }

  size_t i = 2;
  const size_t vvid = static_cast<size_t>(vec[i++]);
  treeid = static_cast<size_t>(vec[i++]);
  algo = static_cast<algorithm>(vec[i++]);
  num_rows = static_cast<size_t>(vec[i++]);
  num_fcols = static_cast<size_t>(vec[i++]);
  tree_assert(i == LEN_METAVEC);

  const size_t flen = num_rows * num_fcols;
  const size_t ilen = num_rows * NUM_ICOLS;

  if (vvid != VVID) {
    throw std::runtime_error("vectorized version ID is different");
  } else if (vec.size() != LEN_METAVEC + flen + ilen) {
    throw std::runtime_error("invalid input length");
  }

#if defined(_SX) || defined(__ve__)
  fvec = std::vector<T>(flen, 0);
  ivec = std::vector<size_t>(ilen, 0);
#else
  fvec.resize(flen, 0);
  ivec.resize(ilen, 0);
#endif

  const T* fsrc = vec.data() + LEN_METAVEC;
  const T* isrc = vec.data() + LEN_METAVEC + flen;
  T* fdst = fvec.data();
  size_t* idst = ivec.data();

  for (i = 0; i < flen; i++) { fdst[i] = fsrc[i]; }
  for (i = 0; i < ilen; i++) { idst[i] = static_cast<size_t>(isrc[i]); }
}

template <typename T>
void modelvector<T>::savebinary(const std::string& path) const {
  const auto metavec = metavector();
  const size_t* msrc = metavec.data();
  const T* fsrc = fvec.data();
  const size_t* isrc = ivec.data();

  tree_assert(metavec.size() == LEN_METAVEC);
  tree_assert(fvec.size() == num_rows * num_fcols);
  tree_assert(ivec.size() == num_rows * NUM_ICOLS);

  std::vector<T> vec(metavec.size() + fvec.size() + ivec.size(), 0);
  U* mdst = fp2uintp(vec.data());
  T* fdst = vec.data() + metavec.size();
  U* idst = fp2uintp(vec.data() + metavec.size() + fvec.size());

  for (size_t i = 0; i < metavec.size(); i++) {
    mdst[i] = static_cast<U>(msrc[i]);
  }
  for (size_t i = 0; i < fvec.size(); i++) {
    fdst[i] = fsrc[i];
  }
  for (size_t i = 0; i < ivec.size(); i++) {
    idst[i] = static_cast<U>(isrc[i]);
  }

  make_dvector_local(vec).savebinary(path);
}

template <typename T>
void modelvector<T>::loadbinary(const std::string& path) {
  const auto vec = make_dvector_loadbinary<T>(path).gather();
  if (vec.size() < LEN_METAVEC) {
    throw std::runtime_error("invalid input");
  }

  size_t i = 0;
  constexpr U ONE(1);
  const U* msrc = fp2uintp(vec.data());

  const U checker1 = msrc[i++];
  const U checker2 = msrc[i++];

  if ((checker1 != ONE) || (checker2 != ONE)) {
    throw std::runtime_error("invalid binary format");
  }

  const size_t vvid = static_cast<size_t>(msrc[i++]);
  treeid = static_cast<size_t>(msrc[i++]);
  algo = static_cast<algorithm>(msrc[i++]);
  num_rows = static_cast<size_t>(msrc[i++]);
  num_fcols = static_cast<size_t>(msrc[i++]);
  tree_assert(i == LEN_METAVEC);

  const size_t flen = num_rows * num_fcols;
  const size_t ilen = num_rows * NUM_ICOLS;

  if (vvid != VVID) {
    throw std::runtime_error("vectorized version ID is different");
  } else if (vec.size() != LEN_METAVEC + flen + ilen) {
    throw std::runtime_error("invalid input length");
  }

#if defined(_SX) || defined(__ve__)
  fvec = std::vector<T>(flen, 0);
  ivec = std::vector<size_t>(ilen, 0);
#else
  fvec.resize(flen, 0);
  ivec.resize(ilen, 0);
#endif

  const T* fsrc = vec.data() + LEN_METAVEC;
  const U* isrc = fp2uintp(vec.data() + LEN_METAVEC + flen);
  T* fdst = fvec.data();
  size_t* idst = ivec.data();

  for (i = 0; i < flen; i++) { fdst[i] = fsrc[i]; }
  for (i = 0; i < ilen; i++) { idst[i] = static_cast<size_t>(isrc[i]); }
}

template <typename T>
std::string modelvector<T>::to_string() const {
  std::ostringstream sout;
  sout << "treeid: " << treeid << std::endl;
  sout << "algo: " << static_cast<size_t>(algo) << std::endl;
  sout << "num_rows: " << num_rows << std::endl;
  sout << "num_fcols: " << num_fcols << std::endl;

  const T* fsrc = fvec.data();
  const size_t* isrc = ivec.data();
  for (size_t i = 0; i < num_rows; i++) {
    for (size_t j = 0; j < NUM_ICOLS; j++) {
      sout << isrc[i * NUM_ICOLS + j] << (j + 1 < NUM_ICOLS ? " " : "; ");
    }
    for (size_t j = 0; j < num_fcols; j++) {
      sout << fsrc[i * num_fcols + j] << (j + 1 < num_fcols ? " " : "");
    }
    sout << std::endl;
  }

  return sout.str();
}

} // end namespace tree

// specialize for broadcast
template <typename T>
node_local<tree::modelvector<T>> make_node_local_broadcast(
  const tree::modelvector<T>& vtree
) {
  return vtree.broadcast();
}

// for output stream
template <typename T>
std::ostream& operator<<(
  std::ostream& os, const tree::modelvector<T>& vtree
) {
  return os << vtree.to_string();
}

} // end namespace frovedis

#endif
