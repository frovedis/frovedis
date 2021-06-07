#ifndef _SGD_PARALLELIZER_HPP_
#define _SGD_PARALLELIZER_HPP_

#include "common.hpp"

namespace frovedis {

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

  template <class DATA_MATRIX, class TRANS_MATRIX,
            class MODEL, class GRADIENT>
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
 
  template <class T, class I, class O, 
            class MODEL, class GRADIENT, class REGULARIZER>
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

  template <class T, class MODEL, class GRADIENT, class REGULARIZER>
  MODEL parallelize(colmajor_matrix<T>& data,
                    dvector<T>& label,
                    MODEL& initModel,
                    GRADIENT& grad,
                    REGULARIZER& rType,
                    std::vector<T>& sample_weight,
                    size_t& n_iter,
                    size_t numIteration,
                    double alpha,
                    bool isIntercept,
                    double convergenceTol);

  double miniBatchFraction;
  SERIALIZE(miniBatchFraction)
};  

// -- to improve vectorization...
template <class T>
void copy_weight(T* dst, T* src, size_t size) {
  for(size_t i = 0; i < size; i++) dst[i] = src[i];
}

template <class T>
void add_weight(T* dst, T* src, size_t size) {
  for(size_t i = 0; i < size; i++) dst[i] += src[i];
}

template <class T>
T local_diff_square_sum(T* prev_model_weightp, T* model_weightp, size_t weight_size) {
  T sum = 0;
  auto self = get_selfid();
  auto each = ceil_div(weight_size, size_t(get_nodesize()));
  auto start = each * self;
  auto end = std::min(each * (self + 1), weight_size);
  for(size_t i = start; i < end; i++) {
    auto diff = prev_model_weightp[i] - model_weightp[i];
    sum += diff * diff;
  }
  return sum;
}
// ----

template <class T, class DATA_MATRIX, class TRANS_MATRIX,
          class MODEL, class GRADIENT, class REGULARIZER>
void do_train_with_trans(std::vector<DATA_MATRIX>& data,
                         std::vector<TRANS_MATRIX>& transData,
                         std::vector<std::vector<T>>& label,
                         MODEL& model,
                         std::vector<std::vector<T>>& sample_weight,
                         size_t& n_iter,
                         sgd_config<GRADIENT, REGULARIZER>& config) {
  frovedis::time_spent trace(TRACE), trace_iter(DEBUG),
    reduce_lap(DEBUG), dtrain_lap(DEBUG),
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
  size_t weight_size = model.weight.size();
  std::vector<T> weight_intercept(weight_size + 1);
  auto weight_interceptp = weight_intercept.data();
  std::vector<T> totaldiffvec(weight_size + 1);
  auto totaldiffvecp = totaldiffvec.data();
  double l_loss = 0.0, sumloss = 0.0;
#ifdef _LOSS_CHECK_
  double best_loss = std::numeric_limits<double>::infinity();
  int no_improvement_count = 0;
#endif
  
  // -------- main loop --------
  size_t i;
  for(i = 1; i <= numIteration; i++) {
    dtrain_lap.lap_start();
    auto updated_model = sgd_dtrain_with_trans<T>(i,alpha,isIntercept)
      (data,transData,label,sample_weight,model,grad,l_loss);
    dtrain_lap.lap_stop();
    if(self == 0) trace.show("dtrain: ");

    reduce_lap.lap_start();
#ifdef _RMSE_CONV_RATE_CHECK_
    auto prev_model = model;
#endif
    // updated_model = updated_model - model
    calc_diff_inplace<MODEL>(updated_model, model); 
    auto updated_model_weightp = updated_model.weight.data();
    copy_weight(weight_interceptp, updated_model_weightp, weight_size);
    weight_interceptp[weight_size] = updated_model.intercept;
    // assume weight_size can be represented as int
    typed_allreduce(weight_interceptp, totaldiffvecp, weight_size + 1,
                    MPI_SUM, frovedis_comm_rpc);
    typed_allreduce(&l_loss, &sumloss, 1, MPI_SUM, frovedis_comm_rpc);
    reduce_lap.lap_stop();
    if(self == 0) trace.show("allreduce: ");

    update_lap.lap_start();
    auto model_weightp = model.weight.data();
    add_weight(model_weightp, totaldiffvecp, weight_size);
    model.intercept += totaldiffvecp[weight_size];
    rType.regularize(model.weight, alpha / std::sqrt(i));
    update_lap.lap_stop();
    if(self == 0) trace.show("update and regularize: ");

#ifdef _CONV_RATE_CHECK_
    conv_lap.lap_start();
#ifdef _RMSE_CONV_RATE_CHECK_
    auto prev_model_weightp = prev_model.weight.data();
    T sum = local_diff_square_sum(prev_model_weightp,
                                  model_weightp, weight_size);
    T reduced_sum;
    typed_allreduce(&sum, &reduced_sum, 1, MPI_SUM, frovedis_comm_rpc);
    reduced_sum +=
      (prev_model.intercept - model.intercept) *
      (prev_model.intercept - model.intercept);
    T RMSE = sqrt(reduced_sum/(weight_size + 1));
    if(self == 0) trace.show("RMS error: " + std::to_string(RMSE) + ": ");
    if(RMSE < convergenceTol) {
      if(self == 0) { 
        RLOG(INFO) << "Convergence achieved in " << ITOS(i) << " iterations.\n";
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
    conv_lap.lap_stop();
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
    n_iter = (i == numIteration + 1) ? numIteration : i;
    reduce_lap.show_lap("allreduce time: ");
    dtrain_lap.show_lap("dtrain time: ");
    update_lap.show_lap("update time: ");
#ifdef _CONV_RATE_CHECK_
    conv_lap.show_lap("check convergence time: ");
#endif
  }
}

template <class T, class DATA_MATRIX,
          class MODEL, class GRADIENT, class REGULARIZER>
void do_train_notrans(std::vector<DATA_MATRIX>& data,
                      std::vector<std::vector<T>>& label,
                      MODEL& model,
                      std::vector<std::vector<T>>& sample_weight,
                      size_t& n_iter,
                      sgd_config<GRADIENT, REGULARIZER>& config) {
  frovedis::time_spent trace(TRACE), trace_iter(DEBUG),
    reduce_lap(DEBUG), dtrain_lap(DEBUG),
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
  size_t weight_size = model.weight.size();
  std::vector<T> weight_intercept(weight_size + 1);
  auto weight_interceptp = weight_intercept.data();
  std::vector<T> totaldiffvec(weight_size + 1);
  auto totaldiffvecp = totaldiffvec.data();
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
      sgd_dtrain<T>(i,alpha,isIntercept)(data,label,sample_weight,model,grad,l_loss);
    dtrain_lap.lap_stop();
    if(self == 0) trace.show("dtrain: ");

    reduce_lap.lap_start();
#ifdef _RMSE_CONV_RATE_CHECK_
    auto prev_model = model;
#endif
    // updated_model = updated_model - model
    calc_diff_inplace<MODEL>(updated_model, model); 
    auto updated_model_weightp = updated_model.weight.data();
    copy_weight(weight_interceptp, updated_model_weightp, weight_size);
    weight_interceptp[weight_size] = updated_model.intercept;
    // assume weight_size can be represented as int
    typed_allreduce(weight_interceptp, totaldiffvecp, weight_size + 1,
                    MPI_SUM, frovedis_comm_rpc);
    typed_allreduce(&l_loss, &sumloss, 1, MPI_SUM, frovedis_comm_rpc);
    reduce_lap.lap_stop();
    if(self == 0) trace.show("allreduce: ");

    update_lap.lap_start();
    auto model_weightp = model.weight.data();
    add_weight(model_weightp, totaldiffvecp, weight_size);
    model.intercept += totaldiffvecp[weight_size];
    rType.regularize(model.weight, alpha / std::sqrt(i));
    update_lap.lap_stop();
    if(self == 0) trace.show("update and regularize: ");

#ifdef _CONV_RATE_CHECK_
    conv_lap.lap_start();
#ifdef _RMSE_CONV_RATE_CHECK_
    auto prev_model_weightp = prev_model.weight.data();
    T sum = local_diff_square_sum(prev_model_weightp,
                                  model_weightp, weight_size);
    T reduced_sum;
    typed_allreduce(&sum, &reduced_sum, 1, MPI_SUM, frovedis_comm_rpc);
    reduced_sum +=
      (prev_model.intercept - model.intercept) *
      (prev_model.intercept - model.intercept);
    T RMSE = sqrt(reduced_sum/(weight_size + 1));
    if(self == 0) trace.show("RMS error: " + std::to_string(RMSE) + ": ");
    if(RMSE < convergenceTol) {
      if(self == 0) { 
        RLOG(INFO) << "Convergence achieved in " << ITOS(i) << " iterations.\n";
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
    conv_lap.lap_stop();
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
    reduce_lap.show_lap("allreduce time: ");
    dtrain_lap.show_lap("dtrain time: ");
    update_lap.show_lap("update time: ");
#ifdef _CONV_RATE_CHECK_
    conv_lap.show_lap("check convergence time: ");
#endif
  }
}

// --- dense support ---
template <class T, class MODEL, 
          class GRADIENT, class REGULARIZER>
MODEL sgd_parallelizer::parallelize(colmajor_matrix<T>& data,
                                    dvector<T>& label,
                                    MODEL& initModel,
                                    GRADIENT& grad,
                                    REGULARIZER& rType,
                                    std::vector<T>& sample_weight,
                                    size_t& n_iter,
                                    size_t numIteration,
                                    double alpha,
                                    bool isIntercept,
                                    double convergenceTol) {
  checkAssumption (numIteration > 0 && alpha > 0 &&
                   rType.regParam >= 0 && convergenceTol >= 0);
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
  
  // just being sliced, no copy
  auto div_data  = data.data.map(divide_data_to_minibatch_colmajor<T>,
                                 broadcast(miniBatchFraction));
  auto div_label = nloc_label.map(divide_label_to_minibatch<T>,
                                  broadcast(miniBatchFraction));
  auto div_sample_weight = nsample_weight.map(divide_sample_weight_to_minibatch<T>,
                                              broadcast(miniBatchFraction));

  t0.show("divide minibatch: ");

  auto trainedModel = broadcast(initModel);
  sgd_config<GRADIENT, REGULARIZER> config(numIteration, alpha, convergenceTol,
                                    isIntercept, numSamples, grad, rType);
  auto nIter = make_node_local_allocate<size_t>();
  div_data.mapv(do_train_notrans<T,sliced_colmajor_matrix_local<T>,
                MODEL,GRADIENT,REGULARIZER>,
                div_label, trainedModel, 
                div_sample_weight, nIter,
                broadcast(config));
  n_iter = nIter.get(0);
  return trainedModel.get(0);
}

template <class T, class I, class O, 
          class MODEL, class GRADIENT, class REGULARIZER>
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

  auto nIter = make_node_local_allocate<size_t>();
  auto trainedModel = broadcast(initModel);
  sgd_config<GRADIENT, REGULARIZER> config(numIteration, alpha, convergenceTol,
                                    isIntercept, numSamples, grad, rType);

  // -------- selection of input matrix structure --------
  if (mType == CRS) {
    auto trans_crs_vec = div_data.map(to_trans_crs_vec<T,I,O>);
    t0.show("to trans crs: ");
    div_data.mapv(do_train_with_trans<T,crs_matrix_local<T,I,O>,
                  crs_matrix_local<T,I,O>,MODEL,GRADIENT,REGULARIZER>,
                  trans_crs_vec, div_label, trainedModel, 
                  div_sample_weight, nIter,
                  broadcast(config));
    t0.show("training loop: ");
  }
  else if (mType == HYBRID) {
    auto jds_crs_vec = div_data.map(to_jds_crs_vec<T,I,O>);
    t0.show("to jds_crs: ");
    auto trans_jds_crs_vec = div_data.map(to_trans_jds_crs_vec<T,I,O>);
    t0.show("to trans jds_crs: ");
    div_data.mapv(clear_data_vector<T,I,O>);
    t0.show("clear div_data: ");
    jds_crs_vec.mapv(do_train_with_trans<T,jds_crs_hybrid_local<T,I,O>,
                     jds_crs_hybrid_local<T,I,O>,MODEL,GRADIENT,REGULARIZER>,
                     trans_jds_crs_vec, div_label, trainedModel,
                     div_sample_weight, nIter,
                     broadcast(config));
    t0.show("training loop: ");
  } else  if (mType == JDS) {
    auto jds_vec = div_data.map(to_jds_vec<T,I,O>);
    t0.show("to jds: ");
    auto trans_jds_vec = div_data.map(to_trans_jds_vec<T,I,O>);
    t0.show("to trans jds: ");
    div_data.mapv(clear_data_vector<T,I,O>);
    t0.show("clear div_data: ");
    jds_vec.mapv(do_train_with_trans<T,jds_matrix_local<T,I,O>,
                 jds_matrix_local<T,I,O>,MODEL,GRADIENT,REGULARIZER>,
                 trans_jds_vec, div_label, trainedModel,
                 div_sample_weight, nIter,
                 broadcast(config));
    t0.show("training loop: ");
  } else if (mType == ELL) {
    auto ell_vec = div_data.map(to_ell_vec<T,I,O>);
    t0.show("to ell: ");
    div_data.mapv(clear_data_vector<T,I,O>);
    t0.show("clear div_data: ");
    ell_vec.mapv(do_train_notrans<T,ell_matrix_local<T,I>,
                 MODEL,GRADIENT,REGULARIZER>,
                 div_label, trainedModel,
                 div_sample_weight, nIter,
                 broadcast(config));
    t0.show("training loop: ");
  }
  else
    REPORT_ERROR(USER_ERROR,"Unsupported input matrix type!!\n");
  n_iter = nIter.get(0);
  return trainedModel.get(0);
}

}

#endif
