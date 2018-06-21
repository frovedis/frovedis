#ifndef _SGD_PARALLELIZER_HPP_
#define _SGD_PARALLELIZER_HPP_

#include "common.hpp"

namespace frovedis {

template <class T>
struct sgd_dtrain {
  sgd_dtrain(): iterCount(1), alpha(0.01), isIntercept(false) {}
  sgd_dtrain(size_t i, T al, bool intercept): 
         iterCount(i), alpha(al), isIntercept(intercept) {}

  template <class DATA_MATRIX, class MODEL, class GRADIENT>
  MODEL operator()(std::vector<DATA_MATRIX>& data,
                   std::vector<std::vector<T>>& label,
                   MODEL& gModel,
                   GRADIENT& loss) {
    // --- One time check ---
    if(iterCount == 1 && data.size() != label.size())
      REPORT_FATAL(INTERNAL_ERROR,
                   "Report bug: Problem in internal minibatch creation.\n");
    MODEL lModel = gModel;   
    gradient_descent<T> gd(alpha, isIntercept);
    gd.optimize(data,label,lModel,loss,iterCount);
    return lModel;
  }

  size_t iterCount;
  T alpha;
  bool isIntercept;
  SERIALIZE(iterCount, alpha, isIntercept)
};

template <class T>
struct sgd_dtrain_with_trans {
  sgd_dtrain_with_trans(): iterCount(1), alpha(0.01), isIntercept(false) {}
  sgd_dtrain_with_trans(size_t i, T al, bool intercept): 
                  iterCount(i), alpha(al), isIntercept(intercept) {}

  template <class DATA_MATRIX, class TRANS_MATRIX, class MODEL, class GRADIENT>
  MODEL operator()(std::vector<DATA_MATRIX>& data,
                   std::vector<TRANS_MATRIX>& trans,
                   std::vector<std::vector<T>>& label,
                   MODEL& gModel,
                   GRADIENT& loss) {
    // --- One time check ---
    if(iterCount == 1 && data.size() != label.size())
      REPORT_FATAL(INTERNAL_ERROR,
                   "Report bug: Problem in internal minibatch creation.\n");
    MODEL lModel = gModel;   
    gradient_descent<T> gd(alpha, isIntercept);
    gd.optimize(data,trans,label,lModel,loss,iterCount);
    return lModel;
  }

  size_t iterCount;
  T alpha;
  bool isIntercept;
  SERIALIZE(iterCount, alpha, isIntercept)
};

template <class T>
struct sgd_parallelizer {
  sgd_parallelizer(): miniBatchFraction(1.0) {}
  sgd_parallelizer(T frac): miniBatchFraction(frac) 
    { checkAssumption(miniBatchFraction > 0.0 && miniBatchFraction <= 1.0); }
 
  template <class MODEL, class GRADIENT, class REGULARIZER>
  MODEL parallelize(crs_matrix<T>& data,
                    dvector<T>& label,
                    MODEL& initModel,
                    size_t numIteration,
                    T alpha,
                    T regParam,
                    bool isIntercept,
                    T convergenceTol,
                    MatType mType,
                    bool inputMovable);

  private:
  template <class DATA_MATRIX, 
            class MODEL, class GRADIENT, class REGULARIZER>
  void do_train(lvec<DATA_MATRIX>& data,
                lvec<std::vector<T>>& label,
                MODEL& initModel,
                size_t numIteration,
                T alpha,
                T regParam,
                bool isIntercept,
                T convergenceTol);

  template <class DATA_MATRIX, class TRANS_MATRIX,
            class MODEL, class GRADIENT, class REGULARIZER>
  void do_train(lvec<DATA_MATRIX>& data,
                lvec<TRANS_MATRIX>& transData,
                lvec<std::vector<T>>& label,
                MODEL& initModel,
                size_t numIteration,
                T alpha,
                T regParam,
                bool isIntercept,
                T convergenceTol);

  template <class MODEL, class REGULARIZER>
  bool update_global_model(node_local<MODEL>& global,
                           node_local<MODEL>& local,
                           MODEL& initModel,
                           REGULARIZER& rType,
                           T convergenceTol,
                           size_t iterCount,
                           time_spent& t,
                           time_spent& reduce_lap,
                           time_spent& update_lap);

  template <class MODEL, class REGULARIZER>
  void update_and_regularize_model(MODEL& target,
                                   MODEL& diff,
                                   REGULARIZER& rType);
  
  template <class MODEL>
  MODEL reduce_local_models(node_local<MODEL>& global,
                            node_local<MODEL>& local);

  T miniBatchFraction;
  SERIALIZE(miniBatchFraction)
};  

template <class T>
template <class MODEL>
inline MODEL 
sgd_parallelizer<T>::reduce_local_models(node_local<MODEL>& global, 
                                         node_local<MODEL>& local) {
  // global will no longer be needed. Thus calculating difference in-place.
  global.mapv(calc_diff_inplace<MODEL>,local); // global := global - local
  auto weights = make_node_local_allocate<std::vector<T>>();
  global.mapv(get_weight_intercept<MODEL,T>, weights); 
  auto totaldiffvec = weights.vector_sum();
  auto totaldiffintercept = totaldiffvec[totaldiffvec.size() - 1];
  totaldiffvec.pop_back();
  MODEL totalDiff;
  totalDiff.weight.swap(totaldiffvec);
  totalDiff.intercept = totaldiffintercept;
  return totalDiff;
}

template <class T>
template <class MODEL, class REGULARIZER>
inline void 
sgd_parallelizer<T>::update_and_regularize_model(MODEL& target, 
                                                 MODEL& diff, 
                                                 REGULARIZER& rType) {
  target -= diff;
  rType.regularize(target.weight);
}

template <class T>
template <class MODEL, class REGULARIZER>
inline bool 
sgd_parallelizer<T>::update_global_model(node_local<MODEL>& global,
                                         node_local<MODEL>& local,
                                         MODEL& initModel,
                                         REGULARIZER& rType,
                                         T convergenceTol,
                                         size_t iterCount,
                                         time_spent& t,
                                         time_spent& reduce_lap,
                                         time_spent& update_lap) {
  bool conv = false;
  reduce_lap.lap_start();
  MODEL totaldiff = reduce_local_models(global, local);
  reduce_lap.lap_stop();
  t.show("reduce: ");

#ifdef _ALLOW_CONV_RATE_CHECK_
  MODEL prev_model = initModel;
#endif

  update_lap.lap_start();
  update_and_regularize_model(initModel,totaldiff,rType);
  update_lap.lap_stop();
  t.show("update and regularize: ");

#ifdef _ALLOW_CONV_RATE_CHECK_
  MODEL &cur_model = initModel;
  if(is_converged(cur_model,prev_model,convergenceTol,iterCount)) conv = true;
#endif

  return conv;
}                          

template <class T>
template <class DATA_MATRIX, class TRANS_MATRIX, 
          class MODEL, class GRADIENT, class REGULARIZER>
void sgd_parallelizer<T>::do_train(lvec<DATA_MATRIX>& data,
                                   lvec<TRANS_MATRIX>& transData,
                                   lvec<std::vector<T>>& label,
                                   MODEL& initModel,
                                   size_t numIteration,
                                   T alpha,
                                   T regParam,
                                   bool isIntercept,
                                   T convergenceTol) {
  frovedis::time_spent t(TRACE), t2(TRACE),
    bcast_lap(DEBUG), reduce_lap(DEBUG), dtrain_lap(DEBUG),
    update_lap(DEBUG);

  REGULARIZER rType(regParam);
  auto distLoss = make_node_local_allocate<GRADIENT>();
  
  // -------- main loop --------
  for(size_t i = 1; i <= numIteration; i++) {
    frovedis::time_spent t3(TRACE);
    bcast_lap.lap_start();
    auto distModel = initModel.broadcast();
    bcast_lap.lap_stop();
    t3.show("broadcast: ");

    // work_at_worker
    dtrain_lap.lap_start();
    auto locModel = data.template map<MODEL>
                    (sgd_dtrain_with_trans<T>(i,alpha,isIntercept),
                     transData,label,distModel,distLoss);
    dtrain_lap.lap_stop();
    t3.show("Dtrain: ");

    // work_at_master
    bool conv = update_global_model(distModel,locModel,initModel,
                                    rType,convergenceTol,i,t3,
                                    reduce_lap,update_lap);
#ifdef _ALLOW_CONV_RATE_CHECK_
    if(conv) break;
#endif

    std::string msg = "[Iteration: " + ITOS(i) + "] elapsed-time: ";
    t.show(msg);
  }
  bcast_lap.show_lap("bcast time: ");
  reduce_lap.show_lap("reduce time: ");
  dtrain_lap.show_lap("dtrain time: ");
  update_lap.show_lap("local update time: ");
  t2.show("whole iteration: ");
}

template <class T>
template <class DATA_MATRIX, 
          class MODEL, class GRADIENT, class REGULARIZER>
void sgd_parallelizer<T>::do_train(lvec<DATA_MATRIX>& data,
                                   lvec<std::vector<T>>& label,
                                   MODEL& initModel,
                                   size_t numIteration,
                                   T alpha,
                                   T regParam,
                                   bool isIntercept,
                                   T convergenceTol) {
  frovedis::time_spent t(TRACE), t2(TRACE),
    bcast_lap(DEBUG), reduce_lap(DEBUG), dtrain_lap(DEBUG),
    update_lap(DEBUG);

  REGULARIZER rType(regParam);
  auto distLoss = make_node_local_allocate<GRADIENT>();

  // -------- main loop --------
  for(size_t i = 1; i <= numIteration; i++) {
    frovedis::time_spent t3(TRACE);
    bcast_lap.lap_start();
    auto distModel = initModel.broadcast();
    bcast_lap.lap_stop();
    t3.show("broadcast: ");

    // work_at_worker
    dtrain_lap.lap_start();
    auto locModel = data.template map<MODEL>
                    (sgd_dtrain<T>(i,alpha,isIntercept),
                     label,distModel,distLoss);
    dtrain_lap.lap_stop();
    t3.show("Dtrain: ");

    // work_at_master
    bool conv = update_global_model(distModel,locModel,initModel,
                                    rType,convergenceTol,i,t3,
                                    reduce_lap,update_lap);
#ifdef _ALLOW_CONV_RATE_CHECK_
    if(conv) break;
#endif

    std::string msg = "[Iteration: " + ITOS(i) + "] elapsed-time: ";
    t.show(msg);
  }
  bcast_lap.show_lap("bcast time: ");
  reduce_lap.show_lap("reduce time: ");
  dtrain_lap.show_lap("dtrain time: ");
  update_lap.show_lap("local update time: ");
  t2.show("whole iteration: ");
}

template <class T>
template <class MODEL, class GRADIENT, class REGULARIZER>
MODEL sgd_parallelizer<T>::parallelize(crs_matrix<T>& data,
                                       dvector<T>& label,
                                       MODEL& initModel,
                                       size_t numIteration,
                                       T alpha,
                                       T regParam,
                                       bool isIntercept,
                                       T convergenceTol,
                                       MatType mType,
                                       bool inputMovable) {
  checkAssumption (numIteration > 0 && alpha > 0 && 
                   regParam >= 0 && convergenceTol >= 0);

  MODEL trainedModel = initModel;
  size_t numFeatures = data.num_col;
  size_t numSamples  = data.num_row;

  if(!numFeatures || !numSamples)
    REPORT_ERROR(USER_ERROR,"Empty training data\n");

  if(numFeatures != initModel.get_num_features())
    REPORT_ERROR
      (USER_ERROR,"Incompatible Test Vector with Provided Initial Model\n");

  if(numSamples != label.size())
    REPORT_ERROR
      (USER_ERROR,"Number of label and data are different\n");

  time_spent t0(DEBUG);
  // crs_get_local_num_row is defined in crs_matrix.hpp
  auto sizes = data.data.map(crs_get_local_num_row<T>).gather();
  label.align_as(sizes);
  auto nloc_label = label.viewas_node_local();
  t0.show("label resize & nloc: ");

  auto div_data  = data.data.map(divide_data_to_minibatch<T>,
                                 broadcast(miniBatchFraction));
  auto div_label = nloc_label.map(divide_label_to_minibatch<T>,
                                  broadcast(miniBatchFraction));
  t0.show("divide minibatch: ");

  if(inputMovable) { // to free memory
    data.data.mapv(clear_data<T>);
    t0.show("clear input contents: ");
  }

  // -------- selection of input matrix structure --------
  if (mType == CRS) {
    auto trans_crs_vec = div_data.map(to_trans_crs_vec<T>);
    t0.show("to trans crs: ");
    do_train<crs_matrix_local<T>,crs_matrix_local<T>,
             MODEL,GRADIENT,REGULARIZER> 
                (div_data,trans_crs_vec,div_label,trainedModel,
                 numIteration,alpha,regParam,isIntercept,convergenceTol);
  }
  else if (mType == HYBRID) {
    auto jds_crs_vec = div_data.map(to_jds_crs_vec<T>);
    t0.show("to jds_crs: ");
    auto trans_jds_crs_vec = div_data.map(to_trans_jds_crs_vec<T>);
    t0.show("to trans jds_crs: ");
    div_data.mapv(clear_data_vector<T>);
    t0.show("clear div_data: ");
    do_train<jds_crs_hybrid_local<T>,jds_crs_hybrid_local<T>,
             MODEL,GRADIENT,REGULARIZER>
                (jds_crs_vec,trans_jds_crs_vec,div_label,trainedModel,
                 numIteration,alpha,regParam,isIntercept,convergenceTol);
  } else  if (mType == JDS) {
    auto jds_vec = div_data.map(to_jds_vec<T>);
    t0.show("to jds: ");
    auto trans_jds_vec = div_data.map(to_trans_jds_vec<T>);
    t0.show("to trans jds: ");
    div_data.mapv(clear_data_vector<T>);
    t0.show("clear div_data: ");
    do_train<jds_matrix_local<T>,jds_matrix_local<T>,
             MODEL,GRADIENT,REGULARIZER>
                (jds_vec,trans_jds_vec,div_label,trainedModel,
                 numIteration,alpha,regParam,isIntercept,convergenceTol);
  } else if (mType == ELL) {
    auto ell_vec = div_data.map(to_ell_vec<T>);
    t0.show("to ell: ");
    div_data.mapv(clear_data_vector<T>);
    t0.show("clear div_data: ");
    do_train<ell_matrix_local<T>,MODEL,GRADIENT,REGULARIZER>
               (ell_vec,div_label,trainedModel,
                numIteration,alpha,regParam,isIntercept,convergenceTol);
  }
  else
    REPORT_ERROR(USER_ERROR,"Unsupported input matrix type!!\n");

  return trainedModel;
}

}

#endif
