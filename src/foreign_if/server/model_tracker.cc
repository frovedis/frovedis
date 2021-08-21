#include "model_tracker.hpp"

// should get activated after type check (float/double) support added in model_table
#if 0 
#include "frovedis/ml/glm/linear_model.hpp"
#include "frovedis/ml/recommendation/matrix_factorization_model.hpp"
#include "frovedis/ml/clustering/kmeans.hpp"
#include "frovedis/ml/tree/tree_model.hpp"
#include "frovedis/ml/fm/model.hpp"
#include "frovedis/ml/nb/nb_model.hpp"
#include "frovedis/ml/fpm/fp_growth_model.hpp"
#include "frovedis/ml/clustering/spectral_clustering_model.hpp"
#include "frovedis/ml/clustering/spectral_embedding_model.hpp"
#include "frovedis/ml/clustering/agglomerative_model.hpp"
#endif

using namespace frovedis;

std::map<int,std::pair<MODEL_KIND,exrpc_ptr_t>> model_table;
std::set<int> deleted_model_tracker;
std::set<int> under_training_model_tracker;

// ------------------------------------------------------
// Note: C++ std::set is implemented using red-black tree data structure.
// Therefore insertion(insert), deletion(erase), search(find) 
// operation would have time-complexity is O(logn)
// ------------------------------------------------------
void register_for_train(int mid) {
  auto r = under_training_model_tracker.insert(mid);
  if(!r.second) REPORT_WARNING(WARNING_MESSAGE,"model is already under training!\n");
}

// unregisters a model from training 
void unregister_from_train(int mid) {
  auto r = under_training_model_tracker.erase(mid);
  if(!r) REPORT_WARNING(WARNING_MESSAGE,"model is not under training!\n"); 
}

// checks whether a model is currently under training
bool is_under_training(int mid) {
  return (under_training_model_tracker.find(mid) !=
          under_training_model_tracker.end()); 
}

// registers a trained model in frovedis::model_table
void register_model(int mid, MODEL_KIND m, exrpc_ptr_t mptr) {
  model_table[mid] = std::make_pair(m,mptr);
}

// checks whether a model is already deleted
bool is_deleted(int mid) {
  return (deleted_model_tracker.find(mid) != deleted_model_tracker.end());
}

// checks if  model with given mid is alreday registered (a pre-trained model)
bool is_registered_model(int mid) {
  return (model_table.find(mid) != model_table.end());
}

// deletes all in-memory Frovedis models 
void finalize_model_table() {
/*
  TODO: <--- correct this case --->
  with float data support in case of python, these typecasting would become erroneous

  for(auto m: model_table) {
    switch(m.second.first) {
      case LR:     delete reinterpret_cast<logistic_regression<double>*>(m.second.second); break;
      case SVM:    delete reinterpret_cast<svm_model<double>*>(m.second.second); break;
      case LNRM:   delete reinterpret_cast<linear_regression_model<double>*>(m.second.second); break;
      case MFM:    delete reinterpret_cast<matrix_factorization_model<double>*>(m.second.second); break;
      case KMEANS: delete reinterpret_cast<rowmajor_matrix_local<double>*>(m.second.second); break;
      case DTM: delete reinterpret_cast<decision_tree_model<double>*>(m.second.second); break;
      case NBM: delete reinterpret_cast<naive_bayes_model<double>*>(m.second.second); break;
      case FMM: delete reinterpret_cast<fm::fm_model<double>*>(m.second.second); break;
      case FPM: delete reinterpret_cast<fp_growth_model*>(m.second.second); break;
      default: REPORT_ERROR(INTERNAL_ERROR,"Unknown Model Kind is encountered!\n");
    }
  }
*/
  model_table.erase(model_table.begin(), model_table.end()); 
}

// deletes all trackers
void finalize_trackers() {
  std::set<int> tmp;
  tmp = std::move(deleted_model_tracker);
  tmp = std::move(under_training_model_tracker);
}

void cleanup_frovedis_server() {
  finalize_model_table();
  finalize_trackers();
}

int get_numeric_dtype(const std::string& dt) {
  if (dt == "int" || dt == "int32") return INT; 
  else if (dt == "boolean") return BOOL; 
  else if (dt == "long" || dt == "int64") return LONG; 
  else if (dt == "float" || dt == "float32") return FLOAT; 
  else if (dt == "double" || dt == "float64") return DOUBLE; 
  else if (dt == "dic_string") return STRING; 
  else if (dt == "unsigned long" || dt == "uint64") return ULONG;
  else throw std::runtime_error(dt + ": unsupported dtype encountered!\n");
}

std::string get_string_dtype(short dt) {
  if (dt == INT) return "int"; 
  else if (dt == BOOL) return "boolean"; 
  else if (dt == LONG) return "long"; 
  else if (dt == FLOAT) return "float"; 
  else if (dt == DOUBLE) return "double"; 
  else if (dt == STRING) return "dic_string"; 
  else if (dt == ULONG) return "unsigned long";
  else throw std::runtime_error(STR(dt) + ": unsupported dtype encountered!\n");
}

template <>
bool do_cast<bool>(const std::string& data) {
  int ret = 0;
  if (data == "True") ret = 1;
  else if (data == "False") ret = 0;
  else REPORT_ERROR(USER_ERROR, 
       "do_cast<bool>: Unknown boolean string is detected!\n");
  return ret;
}
