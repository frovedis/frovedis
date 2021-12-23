#ifndef MODEL_TRACKER_HPP
#define MODEL_TRACKER_HPP

#include <set>
#include <boost/lexical_cast.hpp>

#include "../exrpc/exrpc.hpp"
/*
#include "frovedis/ml/glm/linear_model.hpp"
#include "frovedis/ml/recommendation/matrix_factorization_model.hpp"
#include "frovedis/ml/clustering/kmeans.hpp"
#include "frovedis/ml/tree/tree_model.hpp"
#include "frovedis/ml/fm/model.hpp"
#include "frovedis/ml/nb/nb_model.hpp"
#include "frovedis/ml/fpm/fp_growth_model.hpp"
#include "frovedis/ml/clustering/spectral_clustering.hpp"
#include "frovedis/ml/clustering/agglomerative.hpp"
*/

using namespace frovedis;

enum MODEL_KIND {
  GLM = 0,
  LR,
  SVM,
  LNRM,
  MFM,
  KMEANS,
  DTM,
  NBM,
  FMM,
  FPM,
  FPR,
  ACM,
  SCM,
  SEM,
  SPARSE_CONV_INFO,
  W2V,
  DBSCAN,
  KNN,
  KNC,
  KNR,
  LDA,
  LDASP,
  RFM,
  GBT,
  SVR,
  KSVC,
  RR,
  LSR,
  GMM,
  STANDARDSCALER    
};

enum MAT_KIND {
  RMJR = 1,
  CMJR = 2,
  BCLC = 3,
  SCRS = 4,
  SCCS = 5,
  SELL = 6,
  SHYBRID = 7,
  RMJR_L = 101,
  CMJR_L = 102,
  BCLC_L = 103,
  SCRS_L = 104,
  SCCS_L = 105,
  SELL_L = 106,
  SHYBRID_L = 107
};

enum DTYPE {
  INT = 1,
  LONG = 2,
  FLOAT = 3,
  DOUBLE = 4,
  STRING = 5,
  BOOL = 6,
  ULONG = 7,
  WORDS = 8
};

enum OPTYPE {
  // --- conditional ---
  EQ = 1,
  NE = 2,
  GT = 3,
  GE = 4,
  LT = 5,
  LE = 6,
  // --- special conditional ---
  AND = 11,
  OR = 12,
  NOT = 13,
  LIKE = 14,
  NLIKE = 15,
  ISNULL = 16,
  ISNOTNULL = 17,
  // --- mathematical ---
  ADD = 21,
  SUB = 22,
  MUL = 23,
  IDIV = 24,
  FDIV = 25,
  MOD = 26,
  POW = 27,
  // --- aggregator ---
  aMAX = 41,
  aMIN = 42,
  aSUM = 43,
  aAVG = 44,
  aVAR = 45,
  aSEM = 46,
  aSTD = 47,
  aMAD = 48,
  aCNT = 49,
  aSIZE = 50,
  aDSUM = 51,
  aDCNT = 52,
  aFST  = 53,
  aLST  = 54
};

// [MODEL_ID] => [MODEL_KIND, MODEL_PTR]
extern std::map<int,std::pair<MODEL_KIND,exrpc_ptr_t>> model_table;
extern std::set<int> deleted_model_tracker;
extern std::set<int> under_training_model_tracker;

void register_model(int mid, MODEL_KIND m, exrpc_ptr_t mptr);
void register_for_train(int mid);
void unregister_from_train(int mid);
bool is_deleted(int mid);
bool is_registered_model(int mid);
bool is_under_training(int mid);

void finalize_model_table();
void finalize_trackers();
void cleanup_frovedis_server();
int get_numeric_dtype(const std::string& dt);
std::string get_string_dtype(short dt);

// retuns the model head for the requested registered model id
template <class M>
M* get_model_ptr(int mid) {
  //std::cout<<"inside get model ptr \n\n";
  if(model_table.find(mid) == model_table.end()) { // if not registered
 
  //std::cout<<"not registered \n\n";
    if(!is_under_training(mid)) { // if not under training
     // std::cout<<"not under training\n\n";
      std::string message = "request for either non-registered or deleted model: [";
      message += boost::lexical_cast<std::string>(mid) + " ]\n";
      REPORT_ERROR(USER_ERROR, message);
    }
    else while(is_under_training(mid)); // waits until training is completed
  }
  auto p = model_table[mid];
  return reinterpret_cast<M*>(p.second);
}

// convert a numeric string to number
template <class T>
T do_cast (const std::string& data) {
  T c_data = 0;
  try {
    c_data = boost::lexical_cast<T>(data);
  }
  catch (const boost::bad_lexical_cast &excpt) {
    REPORT_ERROR(USER_ERROR, "invalid type for casting: " + data);
  }
  return c_data;
}

template <>
bool do_cast<bool>(const std::string& data);

#endif
