#ifndef _SGD_PARALLELIZER_HPP_
#define _SGD_PARALLELIZER_HPP_

#include "common.hpp"
#include "../../matrix/shrink_matrix.hpp"

namespace frovedis {

template <class T>
struct sgd_dtrain {
  sgd_dtrain(): iterCount(1), alpha(0.01), isIntercept(false) {}
  sgd_dtrain(size_t i, double al, bool intercept): 
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
    gradient_descent gd(alpha, isIntercept);
    gd.optimize<T>(data,label,lModel,loss,iterCount);
    return lModel;
  }

  size_t iterCount;
  double alpha;
  bool isIntercept;
  SERIALIZE(iterCount, alpha, isIntercept)
};

template <class T>
struct sgd_dtrain_with_trans {
  sgd_dtrain_with_trans(): iterCount(1), alpha(0.01), isIntercept(false) {}
  sgd_dtrain_with_trans(size_t i, double al, bool intercept): 
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
    gradient_descent gd(alpha, isIntercept);
    gd.optimize<T>(data,trans,label,lModel,loss,iterCount);
    return lModel;
  }

  size_t iterCount;
  double alpha;
  bool isIntercept;
  SERIALIZE(iterCount, alpha, isIntercept)
};

struct sgd_parallelizer {
  sgd_parallelizer(): miniBatchFraction(1.0) {}
  sgd_parallelizer(double frac): miniBatchFraction(frac) 
    { checkAssumption(miniBatchFraction > 0.0 && miniBatchFraction <= 1.0); }
 
  template <class T, class I, class O, class MODEL, 
            class GRADIENT, class REGULARIZER>
  MODEL parallelize(crs_matrix<T,I,O>& data,
                    dvector<T>& label,
                    MODEL& initModel,
                    size_t numIteration,
                    double alpha,
                    double regParam,
                    bool isIntercept,
                    double convergenceTol,
                    MatType mType,
                    bool inputMovable);

  private:
  template <class T, class DATA_MATRIX, 
            class MODEL, class GRADIENT, class REGULARIZER>
  void do_train(lvec<size_t>& tbl,
                node_local<shrink_vector_info<size_t>>& info,
                lvec<DATA_MATRIX>& data,
                lvec<std::vector<T>>& label,
                MODEL& initModel,
                size_t numIteration,
                double alpha,
                double regParam,
                bool isIntercept,
                double convergenceTol);

  template <class T, class DATA_MATRIX, class TRANS_MATRIX,
            class MODEL, class GRADIENT, class REGULARIZER>
  void do_train(lvec<size_t>& tbl,
                node_local<shrink_vector_info<size_t>>& info,
                lvec<DATA_MATRIX>& data,
                lvec<TRANS_MATRIX>& transData,
                lvec<std::vector<T>>& label,
                MODEL& initModel,
                size_t numIteration,
                double alpha,
                double regParam,
                bool isIntercept,
                double convergenceTol);

  template <class T, class MODEL, class REGULARIZER>
  bool update_model(node_local<MODEL>& global,
                    node_local<MODEL>& local,
                    lvec<size_t>& tbl,
                    node_local<shrink_vector_info<size_t>>& info,
                    double regParam,
                    double convergenceTol,
                    size_t numFeatures,
                    node_local<bool>& conv_checker,
                    bool isIntercept,
                    time_spent& t,
                    time_spent& reduce_lap,
                    time_spent& bcast_lap);
  
  double miniBatchFraction;
  SERIALIZE(miniBatchFraction)
};  

template <class T, class MODEL>
MODEL create_model_from_data(std::vector<T>& weight, 
                             size_t intercept) {
  MODEL m;
  m.weight.swap(weight);
  m.intercept = intercept;
  return m;
}

template <class T, class MODEL, class REGULARIZER>
struct update_model_local {
  update_model_local() {}
  update_model_local(double regParam, double convergenceTol, size_t numFeatures, 
                     bool isIntercept,
                     intptr_t reduce_lap_ptr, intptr_t bcast_lap_ptr) :
    regParam(regParam), convergenceTol(convergenceTol), 
    numFeatures(numFeatures), isIntercept(isIntercept), 
    reduce_lap_ptr(reduce_lap_ptr), bcast_lap_ptr(bcast_lap_ptr) {}
  void operator()(MODEL& global, 
                  MODEL& local, 
                  std::vector<size_t>& tbl,
                  shrink_vector_info<size_t>& info, 
                  bool& conv_checker) {
    // only valid on rank 0
    time_spent& reduce_lap = *reinterpret_cast<time_spent*>(reduce_lap_ptr);
    time_spent& bcast_lap = *reinterpret_cast<time_spent*>(bcast_lap_ptr);
    int self = get_selfid();
    REGULARIZER reg(regParam);
    
    time_spent t(TRACE);
    auto diff = global - local;
    t.show("calc_diff: ");
    if(self == 0) reduce_lap.lap_start();
    auto diff_sum = shrink_vector_sum_local(diff.weight, info);
    if(self == 0) reduce_lap.lap_stop();
    t.show("shrunk_vector_sum_local: ");
    if(self == 0) bcast_lap.lap_start();
    auto bcast_sum = shrink_vector_bcast_local(diff_sum, info);
    if(self == 0) bcast_lap.lap_stop();
    t.show("shrunk_vector_bcast_local: ");
    MODEL totaldiff;
    totaldiff.weight.swap(bcast_sum);
    if(isIntercept) {
      if(self == 0) reduce_lap.lap_start();
      if(self == 0) bcast_lap.lap_start();
      typed_allreduce(&diff.intercept, &totaldiff.intercept, 1, MPI_SUM, 
                      MPI_COMM_WORLD);
      if(self == 0) reduce_lap.lap_stop();
      if(self == 0) bcast_lap.lap_stop();
      t.show("Allreduce intercept: ");
    }
    global -= totaldiff;
    reg.regularize(global.weight);
    t.show("regularize: ");
    
    double sqdiff = 0;
    T* weightp = &diff.weight[0];
    for(size_t i = 0; i < diff.weight.size(); i++) 
      sqdiff += weightp[i] * weightp[i];
    if(isIntercept) sqdiff += diff.intercept * diff.intercept;
    double sqdiff_root;
    MPI_Reduce(&sqdiff, &sqdiff_root, 1, MPI_DOUBLE, MPI_SUM, 0, 
               MPI_COMM_WORLD);

    double RMSE = sqrt(sqdiff_root/(numFeatures + 1));
    std::string msg = std::string("RMS error: ") + ITOS(RMSE) + ": ";
    t.show(msg);

    // makes sense only at root
    if(RMSE < convergenceTol) conv_checker = true;
  }

  double regParam; 
  double convergenceTol;
  size_t numFeatures;
  bool isIntercept;
  intptr_t reduce_lap_ptr;
  intptr_t bcast_lap_ptr;
  SERIALIZE(regParam, convergenceTol, numFeatures, isIntercept,
            reduce_lap_ptr, bcast_lap_ptr)
};


template <class T, class MODEL, class REGULARIZER>
inline bool 
sgd_parallelizer::update_model(node_local<MODEL>& global,
                               node_local<MODEL>& local,
                               lvec<size_t>& tbl,
                               node_local<shrink_vector_info<size_t>>& info,
                               double regParam,
                               double convergenceTol,
                               size_t numFeatures,
                               node_local<bool>& conv_checker,
                               bool isIntercept,
                               time_spent& t,
                               time_spent& reduce_lap,
                               time_spent& bcast_lap) {

  bool conv = false;
  intptr_t reduce_lap_ptr = reinterpret_cast<intptr_t>(&reduce_lap);
  intptr_t bcast_lap_ptr = reinterpret_cast<intptr_t>(&bcast_lap);
  global.mapv(update_model_local<T,MODEL,REGULARIZER>
              (regParam, convergenceTol, numFeatures, isIntercept,
               reduce_lap_ptr, bcast_lap_ptr),
              local, tbl, info, conv_checker);
  t.show("update and regularize: ");
#ifdef _ALLOW_CONV_RATE_CHECK_
  conv = conv_checker.get(0);
#endif

  return conv;
}                          

template <class T, class DATA_MATRIX, class TRANS_MATRIX, 
          class MODEL, class GRADIENT, class REGULARIZER>
void sgd_parallelizer::do_train(lvec<size_t>& tbl,
                                node_local<shrink_vector_info<size_t>>& info,
                                lvec<DATA_MATRIX>& data,
                                lvec<TRANS_MATRIX>& transData,
                                lvec<std::vector<T>>& label,
                                MODEL& initModel,
                                size_t numIteration,
                                double alpha,
                                double regParam,
                                bool isIntercept,
                                double convergenceTol) {
  frovedis::time_spent t(TRACE), t2(TRACE),
    bcast_lap(DEBUG), reduce_lap(DEBUG), dtrain_lap(DEBUG);

  auto weight = frovedis::make_node_local_broadcast(initModel.weight);
  auto shrink_weight = frovedis::shrink_vector(weight, tbl);
  auto intercept = frovedis::make_node_local_broadcast(initModel.intercept);
  auto distModel = shrink_weight.map(create_model_from_data<T, MODEL>, 
                                     intercept);
  auto numFeatures = initModel.get_num_features();
  auto conv_checker = frovedis::make_node_local_allocate<bool>();
  auto distLoss = make_node_local_allocate<GRADIENT>();
  t.show("initialize: ");

  // -------- main loop --------
  for(size_t i = 1; i <= numIteration; i++) {
    frovedis::time_spent t3(TRACE);

    dtrain_lap.lap_start();
    auto locModel = data.template map<MODEL>
                    (sgd_dtrain_with_trans<T>(i,alpha,isIntercept),
                     transData,label,distModel,distLoss);
    dtrain_lap.lap_stop();
    t3.show("Dtrain: ");

    bool conv = update_model<T,MODEL,REGULARIZER>(distModel,locModel,tbl,info,
                             regParam,convergenceTol,numFeatures,
                             conv_checker,isIntercept,t3,
                             reduce_lap, bcast_lap);
    t3.show("update_model: ");

    std::string msg = "[Iteration: " + ITOS(i) + "] elapsed-time: ";
    t.show(msg);
#ifdef _ALLOW_CONV_RATE_CHECK_
    if(conv) {
      RLOG(INFO) << std::string("Convergence achieved in ") + 
        ITOS(i) + " iterations.\n";
      break;
    }
#endif
  }
  bcast_lap.show_lap("bcast time: ");
  reduce_lap.show_lap("reduce time: ");
  dtrain_lap.show_lap("dtrain time: ");
  t2.show("whole iteration: ");
  distModel.mapv(get_weight<T,MODEL>, weight);
  initModel.weight = shrink_vector_merge(weight, info).gather();
  initModel.intercept = distModel.map(get_intercept<T,MODEL>).get(0);
}

template <class T, class DATA_MATRIX, 
          class MODEL, class GRADIENT, class REGULARIZER>
void sgd_parallelizer::do_train(lvec<size_t>& tbl,
                                node_local<shrink_vector_info<size_t>>& info,
                                lvec<DATA_MATRIX>& data,
                                lvec<std::vector<T>>& label,
                                MODEL& initModel,
                                size_t numIteration,
                                double alpha,
                                double regParam,
                                bool isIntercept,
                                double convergenceTol) {
  frovedis::time_spent t(TRACE), t2(TRACE),
    bcast_lap(DEBUG), reduce_lap(DEBUG), dtrain_lap(DEBUG);

  auto weight = frovedis::make_node_local_broadcast(initModel.weight);
  auto shrink_weight = frovedis::shrink_vector(weight, tbl);
  auto intercept = frovedis::make_node_local_broadcast(initModel.intercept);
  auto distModel = shrink_weight.map(create_model_from_data<T, MODEL>, 
                                     intercept);
  auto numFeatures = initModel.get_num_features();
  auto conv_checker = frovedis::make_node_local_allocate<bool>();
  auto distLoss = make_node_local_allocate<GRADIENT>();
  t.show("initialize: ");

  // -------- main loop --------
  for(size_t i = 1; i <= numIteration; i++) {
    frovedis::time_spent t3(TRACE);

    dtrain_lap.lap_start();
    auto locModel = data.template map<MODEL>
                    (sgd_dtrain<T>(i,alpha,isIntercept),
                     label,distModel,distLoss);
    dtrain_lap.lap_stop();
    t3.show("Dtrain: ");

    bool conv = update_model<T,MODEL,REGULARIZER>(distModel,locModel,tbl,info,
                             regParam,convergenceTol,numFeatures,
                             conv_checker, isIntercept, t3,
                             reduce_lap, bcast_lap);
    t3.show("update_model: ");

    std::string msg = "[Iteration: " + ITOS(i) + "] elapsed-time: ";
    t.show(msg);
#ifdef _ALLOW_CONV_RATE_CHECK_
    if(conv) {
      RLOG(INFO) << std::string("Convergence achieved in ") + 
        ITOS(i) + " iterations.\n";
      break;
    }
#endif
  }
  bcast_lap.show_lap("bcast time: ");
  reduce_lap.show_lap("reduce time: ");
  dtrain_lap.show_lap("dtrain time: ");
  t2.show("whole iteration: ");
  distModel.mapv(get_weight<T,MODEL>, weight);
  initModel.weight = shrink_vector_merge(weight, info).gather();
  initModel.intercept = distModel.map(get_intercept<T,MODEL>).get(0);
}

template <class T, class I, class O, class MODEL, 
          class GRADIENT, class REGULARIZER>
MODEL sgd_parallelizer::parallelize(crs_matrix<T,I,O>& data,
                                    dvector<T>& label,
                                    MODEL& initModel,
                                    size_t numIteration,
                                    double alpha,
                                    double regParam,
                                    bool isIntercept,
                                    double convergenceTol,
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
  auto sizes = data.get_local_num_rows();
  label.align_as(sizes);
  auto nloc_label = label.viewas_node_local();
  t0.show("label resize & nloc: ");

  auto tbl = shrink_column(data);
  t0.show("shrink_column: ");

  auto info = prepare_shrink_comm(tbl, data);
  t0.show("prepare_shrink_comm: ");

  auto div_data  = data.data.map(divide_data_to_minibatch_crs<T,I,O>,
                                 broadcast(miniBatchFraction));
  auto div_label = nloc_label.map(divide_label_to_minibatch<T>,
                                  broadcast(miniBatchFraction));
  t0.show("divide minibatch: ");

  if(inputMovable) { // to free memory
    data.clear();
    t0.show("clear input contents: ");
  }

  // -------- selection of input matrix structure --------
  if (mType == CRS) {
    auto trans_crs_vec = div_data.map(to_trans_crs_vec<T,I,O>);
    t0.show("to trans crs: ");
    do_train<T,crs_matrix_local<T,I,O>,crs_matrix_local<T,I,O>,
      MODEL,GRADIENT,REGULARIZER> 
      (tbl,info,div_data,trans_crs_vec,div_label,trainedModel,
       numIteration,alpha,regParam,isIntercept,convergenceTol);
  }
  else if (mType == HYBRID) {
    auto jds_crs_vec = div_data.map(to_jds_crs_vec<T,I,O>);
    t0.show("to jds_crs: ");
    auto trans_jds_crs_vec = div_data.map(to_trans_jds_crs_vec<T,I,O>);
    t0.show("to trans jds_crs: ");
    div_data.mapv(clear_data_vector<T,I,O>);
    t0.show("clear div_data: ");
    do_train<T,jds_crs_hybrid_local<T,I,O>,jds_crs_hybrid_local<T,I,O>,
             MODEL,GRADIENT,REGULARIZER>
      (tbl,info,jds_crs_vec,trans_jds_crs_vec,div_label,trainedModel,
       numIteration,alpha,regParam,isIntercept,convergenceTol);
  }
  else if (mType == ELL) {
    auto ell_vec = div_data.map(to_ell_vec<T,I,O>);
    t0.show("to ell: ");
    div_data.mapv(clear_data_vector<T,I,O>);
    t0.show("clear div_data: ");
    do_train<T,ell_matrix_local<T,I>,MODEL,GRADIENT,REGULARIZER>
      (tbl,info,ell_vec,div_label,trainedModel,
       numIteration,alpha,regParam,isIntercept,convergenceTol);
  }
  else if (mType == JDS) {
    auto jds_vec = div_data.map(to_jds_vec<T,I,O>);
    t0.show("to jds: ");
    auto trans_jds_vec = div_data.map(to_trans_jds_vec<T,I,O>);
    t0.show("to trans jds: ");
    div_data.mapv(clear_data_vector<T,I,O>);
    t0.show("clear div_data: ");
    do_train<T,jds_matrix_local<T,I,O>,jds_matrix_local<T,I,O>,
             MODEL,GRADIENT,REGULARIZER>
      (tbl,info,jds_vec,trans_jds_vec,div_label,trainedModel,
       numIteration,alpha,regParam,isIntercept,convergenceTol);
  }
  else
    REPORT_ERROR(USER_ERROR,"Unsupported input matrix type!!\n");
  return trainedModel;
}

}

#endif
