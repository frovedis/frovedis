#ifndef _SHRINK_SGD_PARALLELIZER_HPP_
#define _SHRINK_SGD_PARALLELIZER_HPP_

#include "common.hpp"
#include "../../matrix/shrink_matrix.hpp"

namespace frovedis {
namespace shrink {

template <class T>
struct sgd_dtrain {
  sgd_dtrain(): iterCount(1), alpha(0.01), isIntercept(false) {}
  sgd_dtrain(size_t i, double al, bool intercept): 
         iterCount(i), alpha(al), isIntercept(intercept) {}

  template <class DATA_MATRIX, class MODEL, class GRADIENT>
  MODEL operator()(std::vector<DATA_MATRIX>& data,
                   std::vector<std::vector<T>>& label,
                   std::vector<std::vector<T>>& sample_weight,
                   MODEL& gModel,
                   GRADIENT& grad,
                   double& loss) {
    // --- One time check ---
    if(iterCount == 1 && data.size() != label.size())
      REPORT_FATAL(INTERNAL_ERROR,
                   "Report bug: Problem in internal minibatch creation.\n");
    MODEL lModel = gModel;   
    gradient_descent gd(alpha, isIntercept);
    gd.optimize<T>(data,label,sample_weight,lModel,grad,iterCount,loss);
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
                   std::vector<std::vector<T>>& sample_weight,
                   MODEL& gModel,
                   GRADIENT& grad,
                   double& loss) {
    // --- One time check ---
    if(iterCount == 1 && data.size() != label.size())
      REPORT_FATAL(INTERNAL_ERROR,
                   "Report bug: Problem in internal minibatch creation.\n");
    MODEL lModel = gModel;    
    gradient_descent gd(alpha, isIntercept);
    gd.optimize<T>(data,trans,label,sample_weight,lModel,grad,iterCount,loss);
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
                    GRADIENT& grad,
                    REGULARIZER& rType,
                    std::vector<T>& sample_weight,
                    size_t& n_iter,
                    size_t numIteration,
                    double alpha,
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

template <class T, class I, class MODEL, class REGULARIZER>
bool update_model_local(double convergenceTol,
                        size_t numFeatures, bool isIntercept,
                        REGULARIZER& reg,
                        time_spent& reduce_lap, time_spent& bcast_lap,
                        time_spent& conv_lap, MODEL& global, MODEL& local,
                        std::vector<I>& tbl,
                        shrink_vector_info<I>& info) {
  int self = get_selfid();
    
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
                    frovedis_comm_rpc);
    reduce_lap.lap_stop();
    if(self == 0) t.show("Allreduce intercept: ");
  }
  global -= totaldiff;
  reg.regularize(global.weight);
  if(self == 0) t.show("regularize: ");

#ifdef _CONV_RATE_CHECK_
  conv_lap.lap_start();
  T sqdiff = local_square_sum(diff_sum.data(), diff_sum.size());
  T sqdiff_sum = 0;
  typed_allreduce(&sqdiff, &sqdiff_sum, 1, MPI_SUM, frovedis_comm_rpc);
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

template <class T, class I, class DATA_MATRIX, class TRANS_MATRIX,
          class MODEL, class GRADIENT, class REGULARIZER>
void do_train_with_trans(std::vector<DATA_MATRIX>& data,
                         std::vector<TRANS_MATRIX>& transData,
                         std::vector<std::vector<T>>& label,
                         std::vector<I>& tbl,
                         shrink_vector_info<I>& info,
                         MODEL& model,
                         std::vector<std::vector<T>>& sample_weight,
                         size_t& n_iter,
                         sgd_config<GRADIENT, REGULARIZER>& config) {

    frovedis::time_spent trace(TRACE), trace_iter(TRACE),
      bcast_lap(DEBUG), reduce_lap(DEBUG), dtrain_lap(DEBUG),
      update_lap(DEBUG), conv_lap(DEBUG);

    // --- extract hyper-parameters ---
    auto numIteration = config.numIteration;
    auto alpha = config.alpha;
    auto convergenceTol = config.convergenceTol;
    auto isIntercept = config.isIntercept;
    auto nsamples = config.numSamples;
    auto grad = config.grad;
    auto rType = config.rType;

    auto self = get_selfid();
    auto shrink_weight = frovedis::shrink_vector_local(model.weight, tbl);
    model.weight.swap(shrink_weight);
    auto numFeatures = model.get_num_features();
    trace.show("initialize: ");
    double l_loss = 0.0, sumloss = 0.0;
#ifdef _LOSS_CHECK_
    double best_loss = std::numeric_limits<double>::infinity();
    int no_improvement_count = 0;
#endif

    // -------- main loop --------
    size_t i;
    for(i = 1; i <= numIteration; i++) {
      dtrain_lap.lap_start();
      auto updated_model =
        sgd_dtrain_with_trans<T>(i,alpha,isIntercept)
        (data,transData,label,sample_weight,model,grad,l_loss);
      dtrain_lap.lap_stop();
      if(self == 0) trace.show("dtrain: ");

      update_lap.lap_start();
      bool conv = update_model_local<T,I,MODEL,REGULARIZER>
        (convergenceTol, numFeatures, isIntercept, rType,
         reduce_lap, bcast_lap, conv_lap, model, updated_model, tbl, info);
      typed_allreduce(&l_loss, &sumloss, 1, MPI_SUM, frovedis_comm_rpc);
      update_lap.lap_stop();

      if(self == 0) trace.show("update_model: ");

#ifdef _CONV_RATE_CHECK_
#ifdef _RMSE_CONV_RATE_CHECK_
      if(conv) {
        if(self == 0) {
          RLOG(INFO) << std::string("Convergence achieved in ") + 
            ITOS(i) + " iterations.\n";
        }
        break;
      }
//#elif _LOSS_CHECK_
#else
      if (i > 1) {
        if (sumloss > best_loss - convergenceTol * nsamples)
          no_improvement_count++;
        else
          no_improvement_count = 0;
        if (sumloss < best_loss) best_loss = sumloss;
        if (no_improvement_count >= NITER_NO_CHANGE) {
          if(self == 0) {
            RLOG(INFO) << "Convergence achieved in " << ITOS(i) << " iterations.\n";
          }
          break;
        }
      }
#endif
#endif

      if(self == 0) {
        auto msg = " --- Epoch: " + std::to_string(i) + " ---\n";
        msg += "  -> norm(w): "  + std::to_string(nrm2<T>(model.weight));
        msg += ", bias: " + std::to_string(model.intercept);
        msg += ", avg. loss: " + std::to_string(sumloss / nsamples);
        msg += ", elapsed-time: ";
        trace_iter.show(msg);
      }
    }

    if(self == 0) {
      n_iter = i;
      bcast_lap.show_lap("bcast time: ");
      reduce_lap.show_lap("reduce time: ");
      dtrain_lap.show_lap("dtrain time: ");
      update_lap.show_lap("update time: ");
#ifdef _CONV_RATE_CHECK_
      conv_lap.show_lap("check convergence time: ");
#endif
    }
  }

template <class T, class I, class DATA_MATRIX,
          class MODEL, class GRADIENT, class REGULARIZER>
void do_train_notrans(std::vector<DATA_MATRIX>& data,
                      std::vector<std::vector<T>>& label,
                       std::vector<I>& tbl,
                      shrink_vector_info<I>& info,
                      MODEL& model,
                      std::vector<std::vector<T>>& sample_weight,
                      size_t& n_iter,
                      sgd_config<GRADIENT, REGULARIZER>& config) {
    frovedis::time_spent trace(TRACE), trace_iter(TRACE),
      bcast_lap(DEBUG), reduce_lap(DEBUG), dtrain_lap(DEBUG),
      update_lap(DEBUG), conv_lap(DEBUG);

    // --- extract hyper-parameters ---
    auto numIteration = config.numIteration;
    auto alpha = config.alpha;
    auto convergenceTol = config.convergenceTol;
    auto isIntercept = config.isIntercept;
    auto nsamples = config.numSamples;
    auto grad = config.grad;
    auto rType = config.rType;

    auto self = get_selfid();
    auto shrink_weight = frovedis::shrink_vector_local(model.weight, tbl);
    model.weight.swap(shrink_weight);
    auto numFeatures = model.get_num_features();
    double l_loss = 0.0, sumloss = 0.0;
#ifdef _LOSS_CHECK_
    double best_loss = std::numeric_limits<double>::infinity();
    int no_improvement_count = 0;
#endif
    trace.show("initialize: ");

    // -------- main loop --------
    size_t i;
    for(i = 1; i <= numIteration; i++) {
      dtrain_lap.lap_start();
      auto updated_model =
        sgd_dtrain<T>(i,alpha,isIntercept)(data,label,sample_weight,model,grad,l_loss);
      dtrain_lap.lap_stop();
      if(self == 0) trace.show("dtrain: ");

      update_lap.lap_start();
      bool conv = update_model_local<T,I,MODEL,REGULARIZER>
        (convergenceTol, numFeatures, isIntercept, rType,
         reduce_lap, bcast_lap, conv_lap, model, updated_model, tbl, info);
      typed_allreduce(&l_loss, &sumloss, 1, MPI_SUM, frovedis_comm_rpc);
      update_lap.lap_stop();
      if(self == 0) trace.show("update_model: ");

#ifdef _CONV_RATE_CHECK_
#ifdef _RMSE_CONV_RATE_CHECK_
      if(conv) {
        if(self == 0) {
          RLOG(INFO) << std::string("Convergence achieved in ") + 
            ITOS(i) + " iterations.\n";
        }
        break;
      }
//#elif _LOSS_CHECK_
#else
      if (i > 1) {
        if (sumloss > best_loss - convergenceTol * nsamples)
          no_improvement_count++;
        else
          no_improvement_count = 0;
        if (sumloss < best_loss) best_loss = sumloss;
        if (no_improvement_count >= NITER_NO_CHANGE) {
          if(self == 0) {
            RLOG(INFO) << "Convergence achieved in " << ITOS(i) << " iterations.\n";
          }
          break;
        }
      }
#endif
#endif

      if(self == 0) {
        auto msg = " --- Epoch: " + std::to_string(i) + " ---\n";
        msg += "  -> norm(w): "  + std::to_string(nrm2<T>(model.weight));
        msg += ", bias: " + std::to_string(model.intercept);
        msg += ", avg. loss: " + std::to_string(sumloss / nsamples);
        msg += ", elapsed-time: ";
        trace_iter.show(msg);
      }
    }

    if(self == 0) {
      n_iter = i;
      bcast_lap.show_lap("bcast time: ");
      reduce_lap.show_lap("reduce time: ");
      dtrain_lap.show_lap("dtrain time: ");
      update_lap.show_lap("update time: ");
#ifdef _CONV_RATE_CHECK_
      conv_lap.show_lap("check convergence time: ");
#endif
    }
  }

template <class T, class I, class O, class MODEL, 
          class GRADIENT, class REGULARIZER>
MODEL sgd_parallelizer::parallelize(crs_matrix<T,I,O>& data,
                                    dvector<T>& label,
                                    MODEL& initModel,
                                    GRADIENT& grad,
                                    REGULARIZER& rType,
                                    std::vector<T>& sample_weight,
                                    size_t& n_iter,
                                    size_t numIteration,
                                    double alpha,
                                    bool isIntercept,
                                    double convergenceTol,
                                    MatType mType,
                                    bool inputMovable) {
  checkAssumption (numIteration > 0 && alpha > 0 && 
                   rType.regParam >= 0 && convergenceTol >= 0);

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

  auto nsample_weight = make_dvector_scatter(sample_weight, sizes).moveto_node_local();

  auto tbl = shrink_column(data);
  t0.show("shrink_column: ");

  auto info = prepare_shrink_comm(tbl, data);
  t0.show("prepare_shrink_comm: ");

  auto div_data  = data.data.map(divide_data_to_minibatch_crs<T,I,O>,
                                 broadcast(miniBatchFraction));
  auto div_label = nloc_label.map(divide_label_to_minibatch<T>,
                                  broadcast(miniBatchFraction));
  auto div_sample_weight = nsample_weight.map(divide_sample_weight_to_minibatch<T>,
                                              broadcast(miniBatchFraction));

  t0.show("divide minibatch: ");

  if(inputMovable) { // to free memory
    data.clear();
    t0.show("clear input contents: ");
  }

  auto weight = make_node_local_allocate<std::vector<T>>();
  auto nIter = make_node_local_allocate<size_t>();
  MODEL returnModel;

  sgd_config<GRADIENT, REGULARIZER> config(numIteration, alpha, convergenceTol,
                                    isIntercept, numSamples,grad, rType);

  // -------- selection of input matrix structure --------
  if (mType == CRS) {
    auto trans_crs_vec = div_data.map(to_trans_crs_vec<T,I,O>);
    t0.show("to trans crs: ");
    div_data.mapv(do_train_with_trans<T, I,
                  crs_matrix_local<T,I,O>, crs_matrix_local<T,I,O>, MODEL,
                  GRADIENT, REGULARIZER>, 
                  trans_crs_vec, div_label, tbl, info, trainedModel,
                  div_sample_weight, nIter, broadcast(config));
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
    jds_crs_vec.mapv(do_train_with_trans<T, I,
                  jds_crs_hybrid_local<T,I,O>, jds_crs_hybrid_local<T,I,O>, MODEL,
                  GRADIENT, REGULARIZER>,
                  trans_jds_crs_vec, div_label, tbl, info, trainedModel,
                  div_sample_weight, nIter, broadcast(config));
    trainedModel.mapv(get_weight<T,MODEL>, weight);
    returnModel.weight = shrink_vector_merge(weight, info).gather();
    returnModel.intercept = trainedModel.map(get_intercept<T,MODEL>).get(0);
  }
  else if (mType == ELL) {
    auto ell_vec = div_data.map(to_ell_vec<T,I,O>);
    t0.show("to ell: ");
    div_data.mapv(clear_data_vector<T,I,O>);
    t0.show("clear div_data: ");
    ell_vec.mapv(do_train_notrans<T, I, ell_matrix_local<T,I>, MODEL,
                  GRADIENT, REGULARIZER>,
                  div_label, tbl, info, trainedModel,
                  div_sample_weight, nIter, broadcast(config));

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
    jds_vec.mapv(do_train_with_trans<T, I,
                  jds_matrix_local<T,I,O>, jds_matrix_local<T,I,O>, MODEL, 
                  GRADIENT, REGULARIZER>,
                  trans_jds_vec, div_label, tbl, info, trainedModel,
                  div_sample_weight, nIter, broadcast(config));
    trainedModel.mapv(get_weight<T,MODEL>, weight);
    returnModel.weight = shrink_vector_merge(weight, info).gather();
    returnModel.intercept = trainedModel.map(get_intercept<T,MODEL>).get(0);
  }
  else
    REPORT_ERROR(USER_ERROR,"Unsupported input matrix type!!\n");
  n_iter = nIter.get(0);
  return returnModel;
}

}
}
#endif
