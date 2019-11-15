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
}

#endif
