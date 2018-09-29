#ifndef MODEL_TRACKER_HPP
#define MODEL_TRACKER_HPP

#include <set>
#include <boost/lexical_cast.hpp>

#include "../exrpc/exrpc.hpp"
#include "frovedis/ml/glm/linear_model.hpp"
#include "frovedis/ml/recommendation/matrix_factorization_model.hpp"
#include "frovedis/ml/clustering/kmeans.hpp"
#include "frovedis/ml/tree/tree_model.hpp"
#include "frovedis/ml/fm/model.hpp"
#include "frovedis/ml/nb/nb_model.hpp"

namespace frovedis {

enum { NONE = 0xDEAD }; 

enum MODEL_KIND {
  GLM = 0,
  LRM,
  SVM,
  LNRM,
  MFM,
  KMEANS,
  DTM,
  NBM,
  FMM,
  FPM
};

enum DMAT_KIND {
  RMJR = 1,
  CMJR = 2,
  BCLC = 3,
  RMJR_L = (20 + RMJR),
  CMJR_L = (20 + CMJR),
  BCLC_L = (20 + BCLC)
};

/*
enum SMAT_KIND {
  CRS = 1,
  CCS = 2,
  ELL = 3,
  HYBRID = 4
};
*/

enum DTYPE {
  INT = 1,
  LONG = 2,
  FLOAT = 3,
  DOUBLE = 4,
  STRING = 5,
  BOOL = 6
};

enum OPTYPE {
  EQ = 1,
  NE = 2,
  GT = 3,
  GE = 4,
  LT = 5,
  LE = 6,
  AND = 11,
  OR = 12
};

// [MODEL_ID] => [MODEL_KIND, MODEL_PTR]
extern std::map<int,std::pair<MODEL_KIND,exrpc_ptr_t>> model_table;
extern std::set<int> deleted_model_tracker;
extern std::set<int> under_training_model_tracker;

void register_model(int mid, MODEL_KIND m, exrpc_ptr_t mptr);
void register_for_train(int mid);
void unregister_from_train(int mid);
bool is_deleted(int mid);
bool is_under_training(int mid);

void finalize_model_table();
void finalize_trackers();
void cleanup_frovedis_server();

// retuns the model head for the requested registered model id
template <class M>
M* get_model_ptr(int mid) {
  if(model_table.find(mid) == model_table.end()) { // if not registered
    if(!is_under_training(mid)) { // if not under training
      std::string message = "request for either non-registered or deleted model: [";
      message += boost::lexical_cast<std::string>(mid) + " ]\n";
      REPORT_ERROR(USER_ERROR, message);
    }
    else while(is_under_training(mid)); // waits until training is completed
  }
  auto p = frovedis::model_table[mid];
  return reinterpret_cast<M*>(p.second);
}

}

#endif
