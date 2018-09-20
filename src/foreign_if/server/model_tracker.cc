#include "model_tracker.hpp"

namespace frovedis {

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


// deletes all in-memory Frovedis models 
void finalize_model_table() {
/*
  TODO: <--- correct this case --->
  with float data support in case of python, these typecasting would become erroneous

  for(auto m: model_table) {
    switch(m.second.first) {
      case LRM:    delete reinterpret_cast<logistic_regression_model<double>*>(m.second.second); break;
      case SVM:    delete reinterpret_cast<svm_model<double>*>(m.second.second); break;
      case LNRM:   delete reinterpret_cast<linear_regression_model<double>*>(m.second.second); break;
      case MFM:    delete reinterpret_cast<matrix_factorization_model<double>*>(m.second.second); break;
      case KMEANS: delete reinterpret_cast<rowmajor_matrix_local<double>*>(m.second.second); break;
      case DTM: delete reinterpret_cast<decision_tree_model<double>*>(m.second.second); break;
      case NBM: delete reinterpret_cast<naive_bayes_model<double>*>(m.second.second); break;
      case FMM: delete reinterpret_cast<fm::fm_model<double>*>(m.second.second); break;
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
}
