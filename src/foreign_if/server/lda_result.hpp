#ifndef _LDA_RESULT_HPP_
#define _LDA_RESULT_HPP_

#include "dummy_matrix.hpp"

namespace frovedis {

struct dummy_lda_result {
  dummy_lda_result() {}
  dummy_lda_result(dummy_matrix& dist_mat, double ppl, double llh):
    dist_mat(dist_mat), perplexity(ppl), likelihood(llh) {}
  dummy_matrix dist_mat;
  double perplexity, likelihood;
  SERIALIZE(dist_mat, perplexity, likelihood)
};

struct describeMatrix {
  describeMatrix() {}
  describeMatrix(rowmajor_matrix_local<int>& word_id,
                 rowmajor_matrix_local<double>& word_dist,
                 int max_terms) {
    auto nr = word_id.local_num_row;
    auto nc = word_id.local_num_col;
    wid_vec.resize(nr * max_terms);
    dist_vec.resize(nr * max_terms);
    auto iptr = word_id.val.data();
    auto dptr = word_dist.val.data();
    auto r_iptr = wid_vec.data();
    auto r_dptr = dist_vec.data();
    for(size_t i = 0; i < nr; ++i) {
      for(size_t j = 0; j < max_terms; ++j){
        r_iptr[j + i * max_terms] = iptr[j + i * nc];
        r_dptr[j + i * max_terms] = dptr[j + i * nc];
      }
    }
  }
  std::vector<int> wid_vec;
  std::vector<double> dist_vec;
  SERIALIZE(wid_vec, dist_vec)
};

struct dummy_lda_model {
  dummy_lda_model() {}
  dummy_lda_model(int k, int vocab): 
            num_topics(k),vocabsz(vocab) {}
  int num_topics, vocabsz;
  SERIALIZE(num_topics, vocabsz)
};


}

#endif
