#include "exrpc_ml.hpp"
#include "short_hand_dense_type.hpp"
#include "short_hand_sparse_type.hpp"

using namespace frovedis;

std::vector<float> 
frovedis_w2v_train(std::string& encode,
                   std::string& count,
                   w2v::train_config& config) {
  return w2v::train_each_impl(encode, count, config);
}

void expose_frovedis_NL_functions() {
  // (1) word2vector
  expose(frovedis_w2v<DT2>);          // spark case
  expose(frovedis_w2v_train);         // python case
  // (2) Latent Dirichlet Allocation
  expose((frovedis_lda_train<DT4,S_MAT45>));
  expose((frovedis_lda_train<DT3,S_MAT35>));
  /* currently frovedis lda does not accept I for input csr matrix */
  /*
  expose((frovedis_lda_train<DT4,S_MAT44>));
  expose((frovedis_lda_train<DT3,S_MAT34>));
  */
  // for spark (changed datatype internally S_MAT15 -> S_MAT35)
  expose((frovedis_lda_train_for_spark<DT3,S_MAT15>)); 
}

