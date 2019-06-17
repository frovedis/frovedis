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
  double miniBatchFraction;
  SERIALIZE(miniBatchFraction)
};  

// -- improve vectorization...
template <class T>
T local_square_sum(T* src, size_t size) {
  T sum = 0;
  for(size_t i = 0; i < size; i++) {
    sum += src[i] * src[i];
  }
  return sum;
}

template <class T, class MODEL, class REGULARIZER>
bool update_model_local(double regParam, double convergenceTol,
                        size_t numFeatures, bool isIntercept,
                        time_spent& reduce_lap, time_spent& bcast_lap,
                        time_spent& conv_lap, MODEL& global, MODEL& local,
                        std::vector<size_t>& tbl,
                        shrink_vector_info<size_t>& info) {
  int self = get_selfid();
  REGULARIZER reg(regParam);
    
  time_spent t(TRACE);
  auto diff = global - local;
  if(self == 0) t.show("calc_diff: ");
  reduce_lap.lap_start();
  auto diff_sum = shrink_vector_sum_local(diff.weight, info);
  reduce_lap.lap_stop();
  if(self == 0) t.show("shrunk_vector_sum_local: ");
  bcast_lap.lap_start();
  auto bcast_sum = shrink_vector_bcast_local(diff_sum, info);
  bcast_lap.lap_stop();
  if(self == 0) t.show("shrunk_vector_bcast_local: ");
  MODEL totaldiff;
  totaldiff.weight.swap(bcast_sum);
  if(isIntercept) {
    reduce_lap.lap_start();
    typed_allreduce(&diff.intercept, &totaldiff.intercept, 1, MPI_SUM, 
                    MPI_COMM_WORLD);
    reduce_lap.lap_stop();
    if(self == 0) t.show("Allreduce intercept: ");
  }
  global -= totaldiff;
  reg.regularize(global.weight);
  if(self == 0) t.show("regularize: ");

#ifdef _ALLOW_CONV_RATE_CHECK_
  conv_lap.lap_start();
  T sqdiff = local_square_sum(diff_sum.data(), diff_sum.size());
  T sqdiff_sum = 0;
  typed_allreduce(&sqdiff, &sqdiff_sum, 1, MPI_SUM, MPI_COMM_WORLD);
  if(isIntercept) 
    sqdiff_sum += diff.intercept * diff.intercept;
  double RMSE = sqrt(sqdiff_sum/(numFeatures + 1));
  if(self == 0) {
    std::string msg = std::string("RMS error: ") + ITOS(RMSE) + ": ";
    t.show(msg);
  }
  conv_lap.lap_stop();
  if(RMSE < convergenceTol) return true;
  else return false;
#else
  return false;
#endif
}

template <class T, class DATA_MATRIX, class TRANS_MATRIX, 
          class MODEL, class GRADIENT, class REGULARIZER>
struct do_train_with_trans {
  do_train_with_trans() {}
  do_train_with_trans(size_t numIteration, double alpha, double regParam,
                      bool isIntercept, double convergenceTol) :
    numIteration(numIteration), alpha(alpha), regParam(regParam),
    isIntercept(isIntercept), convergenceTol(convergenceTol) {}
  void operator()(std::vector<DATA_MATRIX>& data,
                  std::vector<TRANS_MATRIX>& transData,
                  std::vector<std::vector<T>>& label,
                  std::vector<size_t>& tbl,
                  shrink_vector_info<size_t>& info,
                  MODEL& model) {
    frovedis::time_spent trace(TRACE), trace_iter(TRACE),
      bcast_lap(DEBUG), reduce_lap(DEBUG), dtrain_lap(DEBUG),
      update_lap(DEBUG), conv_lap(DEBUG);

    auto self = get_selfid();
    auto shrink_weight = frovedis::shrink_vector_local(model.weight, tbl);
    model.weight.swap(shrink_weight);
    auto numFeatures = model.get_num_features();
    GRADIENT loss;
    trace.show("initialize: ");

    // -------- main loop --------
    for(size_t i = 1; i <= numIteration; i++) {

      dtrain_lap.lap_start();
      auto updated_model =
        sgd_dtrain_with_trans<T>(i,alpha,isIntercept)
        (data,transData,label,model,loss);
      dtrain_lap.lap_stop();
      if(self == 0) trace.show("dtrain: ");

      update_lap.lap_start();
      bool conv = update_model_local<T,MODEL,REGULARIZER>
        (regParam, convergenceTol, numFeatures, isIntercept,
         reduce_lap, bcast_lap, conv_lap, model, updated_model, tbl, info);
      update_lap.lap_stop();
      if(self == 0) trace.show("update_model: ");

      if(self == 0) {
        std::string msg = "[Iteration: " + ITOS(i) + "] elapsed-time: ";
        trace_iter.show(msg);
      }
#ifdef _ALLOW_CONV_RATE_CHECK_
      if(conv) {
        if(self == 0) {
          RLOG(INFO) << std::string("Convergence achieved in ") + 
            ITOS(i) + " iterations.\n";
        }
        break;
      }
#endif
    }
    if(self == 0) {
      bcast_lap.show_lap("bcast time: ");
      reduce_lap.show_lap("reduce time: ");
      dtrain_lap.show_lap("dtrain time: ");
      update_lap.show_lap("update time: ");
#ifdef _ALLOW_CONV_RATE_CHECK_
      conv_lap.show_lap("check convergence time: ");
#endif
    }
  }
  size_t numIteration;
  double alpha;
  double regParam;
  bool isIntercept;
  double convergenceTol;
  SERIALIZE(numIteration,alpha,regParam,isIntercept,convergenceTol)
};

template <class T, class DATA_MATRIX,
          class MODEL, class GRADIENT, class REGULARIZER>
struct do_train_notrans {
  do_train_notrans() {}
  do_train_notrans(size_t numIteration, double alpha, double regParam,
                   bool isIntercept, double convergenceTol) :
    numIteration(numIteration), alpha(alpha), regParam(regParam),
    isIntercept(isIntercept), convergenceTol(convergenceTol) {}
  void operator()(std::vector<DATA_MATRIX>& data,
                  std::vector<std::vector<T>>& label,
                  std::vector<size_t>& tbl,
                  shrink_vector_info<size_t>& info,
                  MODEL& model) {
    frovedis::time_spent trace(TRACE), trace_iter(TRACE),
      bcast_lap(DEBUG), reduce_lap(DEBUG), dtrain_lap(DEBUG),
      update_lap(DEBUG), conv_lap(DEBUG);

    auto self = get_selfid();
    auto shrink_weight = frovedis::shrink_vector_local(model.weight, tbl);
    model.weight.swap(shrink_weight);
    auto numFeatures = model.get_num_features();
    GRADIENT loss;
    trace.show("initialize: ");

    // -------- main loop --------
    for(size_t i = 1; i <= numIteration; i++) {

      dtrain_lap.lap_start();
      auto updated_model =
        sgd_dtrain<T>(i,alpha,isIntercept)(data,label,model,loss);
      dtrain_lap.lap_stop();
      if(self == 0) trace.show("dtrain: ");

      update_lap.lap_start();
      bool conv = update_model_local<T,MODEL,REGULARIZER>
        (regParam, convergenceTol, numFeatures, isIntercept,
         reduce_lap, bcast_lap, conv_lap, model, updated_model, tbl, info);
      update_lap.lap_stop();
      if(self == 0) trace.show("update_model: ");

      if(self == 0) {
        std::string msg = "[Iteration: " + ITOS(i) + "] elapsed-time: ";
        trace_iter.show(msg);
      }
#ifdef _ALLOW_CONV_RATE_CHECK_
      if(conv) {
        if(self == 0) {
          RLOG(INFO) << std::string("Convergence achieved in ") + 
            ITOS(i) + " iterations.\n";
        }
        break;
      }
#endif
    }
    if(self == 0) {
      bcast_lap.show_lap("bcast time: ");
      reduce_lap.show_lap("reduce time: ");
      dtrain_lap.show_lap("dtrain time: ");
      update_lap.show_lap("update time: ");
#ifdef _ALLOW_CONV_RATE_CHECK_
      conv_lap.show_lap("check convergence time: ");
#endif
    }
  }
  size_t numIteration;
  double alpha;
  double regParam;
  bool isIntercept;
  double convergenceTol;
  SERIALIZE(numIteration,alpha,regParam,isIntercept,convergenceTol)
};

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

  auto trainedModel = broadcast(initModel);
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

  auto weight = make_node_local_allocate<std::vector<T>>();
  MODEL returnModel;

  // -------- selection of input matrix structure --------
  if (mType == CRS) {
    auto trans_crs_vec = div_data.map(to_trans_crs_vec<T,I,O>);
    t0.show("to trans crs: ");
    div_data.mapv(do_train_with_trans<T,
                  crs_matrix_local<T,I,O>, crs_matrix_local<T,I,O>,
                  MODEL,GRADIENT,REGULARIZER>
                  (numIteration,alpha,regParam,isIntercept,convergenceTol), 
                  trans_crs_vec, div_label, tbl, info, trainedModel);
    trainedModel.mapv(get_weight<T,MODEL>, weight);
    returnModel.weight = shrink_vector_merge(weight, info).gather();
    returnModel.intercept = trainedModel.map(get_intercept<T,MODEL>).get(0);
  }
  else if (mType == HYBRID) {
    auto jds_crs_vec = div_data.map(to_jds_crs_vec<T,I,O>);
    t0.show("to jds_crs: ");
    auto trans_jds_crs_vec = div_data.map(to_trans_jds_crs_vec<T,I,O>);
    t0.show("to trans jds_crs: ");
    div_data.mapv(clear_data_vector<T,I,O>);
    t0.show("clear div_data: ");
    jds_crs_vec.mapv(do_train_with_trans<T,
                     jds_crs_hybrid_local<T,I,O>,jds_crs_hybrid_local<T,I,O>,
                     MODEL,GRADIENT,REGULARIZER>
                     (numIteration,alpha,regParam,isIntercept,convergenceTol), 
                     trans_jds_crs_vec, div_label, tbl, info, trainedModel);
    trainedModel.mapv(get_weight<T,MODEL>, weight);
    returnModel.weight = shrink_vector_merge(weight, info).gather();
    returnModel.intercept = trainedModel.map(get_intercept<T,MODEL>).get(0);
  }
  else if (mType == ELL) {
    auto ell_vec = div_data.map(to_ell_vec<T,I,O>);
    t0.show("to ell: ");
    div_data.mapv(clear_data_vector<T,I,O>);
    t0.show("clear div_data: ");
    ell_vec.mapv(do_train_notrans<T, ell_matrix_local<T,I>,
                 MODEL,GRADIENT,REGULARIZER>
                 (numIteration,alpha,regParam,isIntercept,convergenceTol), 
                 div_label, tbl, info, trainedModel);
    trainedModel.mapv(get_weight<T,MODEL>, weight);
    returnModel.weight = shrink_vector_merge(weight, info).gather();
    returnModel.intercept = trainedModel.map(get_intercept<T,MODEL>).get(0);
  }
  else if (mType == JDS) {
    auto jds_vec = div_data.map(to_jds_vec<T,I,O>);
    t0.show("to jds: ");
    auto trans_jds_vec = div_data.map(to_trans_jds_vec<T,I,O>);
    t0.show("to trans jds: ");
    div_data.mapv(clear_data_vector<T,I,O>);
    t0.show("clear div_data: ");
    jds_vec.mapv(do_train_with_trans<T,
                 jds_matrix_local<T,I,O>,jds_matrix_local<T,I,O>,
                 MODEL,GRADIENT,REGULARIZER>
                 (numIteration,alpha,regParam,isIntercept,convergenceTol), 
                 trans_jds_vec, div_label, tbl, info, trainedModel);
    trainedModel.mapv(get_weight<T,MODEL>, weight);
    returnModel.weight = shrink_vector_merge(weight, info).gather();
    returnModel.intercept = trainedModel.map(get_intercept<T,MODEL>).get(0);
  }
  else
    REPORT_ERROR(USER_ERROR,"Unsupported input matrix type!!\n");
  return returnModel;
}

}

#endif
