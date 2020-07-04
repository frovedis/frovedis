#ifndef _LDA_RESULT_HPP_
#define _LDA_RESULT_HPP_

#include "dummy_matrix.hpp"
#include "frovedis/core/exceptions.hpp"
#include "frovedis/core/radix_sort.hpp"
#include "frovedis/ml/lda/lda_cgs.hpp"

namespace frovedis {

// wrapper for frovedis::lda_model<TC>
// it includes the doc_topic_count matrix as well in the form of rowmajor_matrix<TC>
template <class TC>
struct lda_model_wrapper{
  lda_model_wrapper() {}
  lda_model_wrapper(lda_model<TC>& t_model, rowmajor_matrix<TC>& mat,
                    std::vector<long>& orig_doc_id):
                    model(t_model), doc_topic_count(mat), 
                    orig_doc_id(orig_doc_id) {
    num_docs = doc_topic_count.num_row; 
    num_topics = doc_topic_count.num_col; 
    num_words = model.word_topic_count.local_num_row; // vocab size in model
  }
  lda_model_wrapper(lda_model<TC>&& t_model, rowmajor_matrix<TC>&& mat,
                    std::vector<long>&& orig_doc_id):
                    model(t_model), doc_topic_count(mat),
                    orig_doc_id(orig_doc_id) {
    num_docs = doc_topic_count.num_row;
    num_topics = doc_topic_count.num_col;
    num_words = model.word_topic_count.local_num_row; // vocab size in model
  }
  void loadbinary (const std::string& path) {
    model.loadbinary(path); // for faster loading
    doc_topic_count = make_rowmajor_matrix_loadbinary<TC>(path + "/doc_topic");
    orig_doc_id = make_dvector_loadbinary<long>(path + "/doc_ids").gather();
  }
  void savebinary (const std::string& path) {
    model.savebinary(path); // for faster saving
    doc_topic_count.savebinary(path + "/doc_topic");
    make_dvector_scatter(orig_doc_id).savebinary(path + "/doc_ids");
  }
  lda_model<TC> model;
  rowmajor_matrix<TC> doc_topic_count;
  std::vector<long> orig_doc_id;
  size_t num_docs, num_topics, num_words;
  SERIALIZE(model, doc_topic_count, num_docs, num_topics, num_words, orig_doc_id);
};

template <class I, class T>
struct distMatrix {
  distMatrix() {}
  distMatrix(rowmajor_matrix_local<I>& ind,
             rowmajor_matrix_local<T>& dist) {
    indices = ind; 
    distances = dist;
  } 
  distMatrix(rowmajor_matrix_local<I>&& ind,
             rowmajor_matrix_local<T>&& dist) {
    indices = std::move(ind); 
    distances = std::move(dist);
  }
  rowmajor_matrix_local<I> indices;
  rowmajor_matrix_local<T> distances;
  SERIALIZE(indices, distances);
};

struct dummy_lda_result {
  dummy_lda_result() {}
  dummy_lda_result(dummy_matrix& dist_mat, double ppl, double llh):
    dist_mat(dist_mat), perplexity(ppl), likelihood(llh) {}
  dummy_matrix dist_mat;
  double perplexity, likelihood;
  SERIALIZE(dist_mat, perplexity, likelihood)
};

struct dummy_lda_model {
  dummy_lda_model() {}
  dummy_lda_model(size_t ndocs, size_t ntopics, size_t nwords): 
                  num_docs(ndocs), num_topics(ntopics), vocabsz(nwords) {}
  size_t num_docs, num_topics, vocabsz;
  SERIALIZE(num_docs, num_topics, vocabsz)
};

template <class T>
rowmajor_matrix_local <double>
get_distribution_matrix_local(rowmajor_matrix_local<T>& m, 
                              std::vector<T>& vec,
                              int axis = 1){
  auto nrow = m.local_num_row;
  auto ncol = m.local_num_col;
  std::vector<double> dbl_vec(vec.size());
  auto vptr = vec.data();
  auto dvptr = dbl_vec.data();
  for(size_t i=0; i < vec.size(); i++) {
    if(vptr[i]) dvptr[i] = 1.0 / vptr[i];
    else        dvptr[i] = 1.0;
  }
  rowmajor_matrix_local<double> ret(nrow, ncol);
  auto mvalptr = m.val.data();
  auto retvalptr = ret.val.data();
  if (axis) {
    for(size_t j = 0; j < ncol; ++j) {
      for(size_t i = 0; i < nrow; ++i) {
        retvalptr[i * ncol + j] = mvalptr[i * ncol + j] * dvptr[i];
      }
    }
  }
  else {
    for(size_t j = 0; j < ncol; ++j) {
      for(size_t i = 0; i < nrow; ++i) {
        retvalptr[i * ncol + j] = mvalptr[i * ncol + j] * dvptr[j];
      }
    }
  }
  return ret;
}

template <class T>
rowmajor_matrix <double>
get_distribution_matrix(rowmajor_matrix<T>& m,
                        int axis = 1) {
  rowmajor_matrix<double> ret;
  if(axis) {
    std::vector<T> (*sum_of_cols_T)(const rowmajor_matrix_local<T>&) =
                                    sum_of_cols<T>;
    auto sumcol = m.data.map(sum_of_cols_T);
    ret.data = m.data.map(get_distribution_matrix_local<T>,
                          sumcol, broadcast(axis));
  }
  else {
    std::vector<T> (*sum_of_rows_T)(const rowmajor_matrix_local<T>&) =
                                    sum_of_rows<T>;
    auto sumrow = m.data.map(sum_of_rows_T).vector_sum();
    ret.data = m.data.map(get_distribution_matrix_local<T>,
                          broadcast(sumrow), broadcast(axis));
  }
  ret.num_row = m.num_row;
  ret.num_col = m.num_col;
  return ret;
}

template <class T>
rowmajor_matrix_local<T>
extract_k_cols_local(rowmajor_matrix_local<T>& mat, int k) {
  auto nrow = mat.local_num_row;
  auto ncol = mat.local_num_col;
  rowmajor_matrix_local<T> ret;
  checkAssumption(k >= 0 && k <= ncol);
  if (k == ncol) ret = mat; // no need for extraction (quick case)
  if (k > 0) {
    ret.val.resize(nrow * k);
    ret.set_local_num(nrow, k);
    auto srcptr = mat.val.data();
    auto dstptr = ret.val.data();
    if (k > nrow) {
      for(size_t i = 0; i < nrow; ++i) {
        for(size_t j = 0; j < k; ++j) {
          dstptr[i * k + j] = srcptr[i * ncol + j];
        }
      }
    }
    else {
      for(size_t j = 0; j < k; ++j) {
        for(size_t i = 0; i < nrow; ++i) {
          dstptr[i * k + j] = srcptr[i * ncol + j];
        }
      }
    }
  }
  return ret; // returns empty matrix in case k = 0
}

template <class T>
rowmajor_matrix<T>
extract_k_cols(rowmajor_matrix<T>& mat, int k) {
  rowmajor_matrix<T> ret(mat.data.map(extract_k_cols_local<T>, broadcast(k)));
  ret.num_row = mat.num_row;
  ret.num_col = mat.num_col;
  return ret;
}

}
#endif
