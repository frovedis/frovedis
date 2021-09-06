#ifndef _LBFGS_PARALLELIZER_HPP_
#define _LBFGS_PARALLELIZER_HPP_

#include "common.hpp"
#include "lbfgs.hpp"

#define LBFGS_MAX_STEP 1e+20 
#define LBFGS_MAX_SEARCH_ITER 10
#define LBFGS_SEARCH_TOL 1e-4
#define LBFGS_PRINT_INTERVAL 10

namespace frovedis {

template <class T>
struct lbfgs_dtrain {
  lbfgs_dtrain(): alpha(0.01), isIntercept(false) {}
  lbfgs_dtrain(double al, bool intercept): 
    alpha(al), isIntercept(intercept) {}

  template <class DATA_MATRIX, class MODEL, class GRADIENT>
  std::vector<T> 
  operator()(DATA_MATRIX& data,
             std::vector<T>& label,
             std::vector<T>& sample_weight,
             MODEL& model,
             GRADIENT& grad_obj,
             double& loss) {
    gradient_descent gd(alpha, isIntercept);
    return gd.compute_gradient<T>(data,model,label,sample_weight,grad_obj,loss);
  }
  double alpha;
  bool isIntercept;
  SERIALIZE(alpha, isIntercept)
};

template <class T>
struct lbfgs_dtrain_with_trans {
  lbfgs_dtrain_with_trans(): alpha(0.01), isIntercept(false) {}
  lbfgs_dtrain_with_trans(double al, bool intercept): 
    alpha(al), isIntercept(intercept) {}

  template <class DATA_MATRIX, class TRANS_MATRIX, class MODEL, class GRADIENT>
  std::vector<T>
  operator()(DATA_MATRIX& data,
             TRANS_MATRIX& trans,
             std::vector<T>& label,
             std::vector<T>& sample_weight,
             MODEL& model,
             GRADIENT& grad_obj,
             double& loss) {
    gradient_descent gd(alpha, isIntercept);
    return gd.compute_gradient<T>(data,trans,model,label,sample_weight,grad_obj,loss);
  }
  double alpha;
  bool isIntercept;
  SERIALIZE(alpha, isIntercept)
};

struct lbfgs_parallelizer {
  lbfgs_parallelizer(): hist_size(10) {}
  lbfgs_parallelizer(size_t hsize): hist_size(hsize) {}

  template <class T, class MODEL, class GRADIENT, class REGULARIZER>
  MODEL parallelize (colmajor_matrix<T>& data,
                     dvector<T>& label,
                     MODEL& initModel,
                     std::vector<T>& sample_weight,
                     size_t& n_iter,
                     size_t numIteration,
                     double alpha,
                     double regParam,
                     bool isIntercept,
                     double convergenceTol);

  template <class T, class I, class O, class MODEL, 
            class GRADIENT, class REGULARIZER>
  MODEL parallelize (crs_matrix<T,I,O>& data,
                     dvector<T>& label,
                     MODEL& initModel,
                     std::vector<T>& sample_weight,
                     size_t& n_iter,
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
  void do_train (node_local<DATA_MATRIX>& data,
                 lvec<T>& label,
                 MODEL& initModel,
                 lvec<T>& sample_weight,
                 size_t& n_iter,
                 size_t numIteration,
                 double alpha,
                 double regParam,
                 bool isIntercept,
                 double convergenceTol);

  template <class T, class DATA_MATRIX, class TRANS_MATRIX,
            class MODEL, class GRADIENT, class REGULARIZER>
  void do_train (node_local<DATA_MATRIX>& data,
                 node_local<TRANS_MATRIX>& transData,
                 lvec<T>& label,
                 MODEL& initModel,
                 lvec<T>& sample_weight,
                 size_t& n_iter,
                 size_t numIteration,
                 double alpha,
                 double regParam,
                 bool isIntercept,
                 double convergenceTol);
  size_t hist_size;
  SERIALIZE(hist_size)
};

template <class T, class DATA_MATRIX, class TRANS_MATRIX, 
          class MODEL, class GRADIENT, class REGULARIZER>
void lbfgs_parallelizer::do_train(node_local<DATA_MATRIX>& data,
                                  node_local<TRANS_MATRIX>& transData,
                                  lvec<T>& label,
                                  MODEL& initModel,
                                  lvec<T>& sample_weight,
                                  size_t& n_iter,
                                  size_t numIteration,
                                  double alpha, // no need
                                  double regParam,
                                  bool isIntercept,
                                  double convergenceTol) {
  frovedis::time_spent t(DEBUG);
  lbfgs<T> opt(alpha,hist_size,isIntercept);
  REGULARIZER rType(regParam); // should be L2 always
  auto distGrad = make_node_local_allocate<GRADIENT>();
  auto dloss = make_node_local_allocate<double>();

  auto distModel = initModel.broadcast();
  auto grad_vector = data.template map<std::vector<T>>
                            (lbfgs_dtrain_with_trans<T>(alpha,isIntercept),
                             transData,label,sample_weight,distModel,distGrad,dloss)
                         .vector_sum();
  auto loss = dloss.reduce(add<double>);
  auto weight = initModel.weight;
  weight.push_back(initModel.intercept);
  opt.old_model = weight;
  opt.old_gradient = grad_vector;

  // -------- main loop --------
  size_t i = 1;
  for(; i <= numIteration; i++) {
    auto step = opt.compute_hkgk(grad_vector);

    // --- line search ---
    // https://github.com/js850/lbfgs_cpp/blob/master/lbfgs.cpp#L216
    // if the step is pointing uphill, invert it
    if (vector_dot(grad_vector, step) > 0) step = step * static_cast<T>(-1.0); 
    T factor = 1.0; // alpha
    T stepsize = vector_norm(step);
    if (factor * stepsize > LBFGS_MAX_STEP) factor = LBFGS_MAX_STEP / stepsize;
    auto bias_step = step.back();
    step.pop_back();
    MODEL new_model;
    lvec<T> new_grad_vector_part;
    auto new_dloss = make_node_local_allocate<double>();
    double newloss = 0;
    size_t j = 1;
    for(; j <= LBFGS_MAX_SEARCH_ITER; ++j) {
      new_model.weight = initModel.weight + (factor * step);
      new_model.intercept = initModel.intercept + (factor * bias_step);
      rType.regularize(new_model.weight);
      auto dist_newmodel = new_model.broadcast();
      new_grad_vector_part = data.template map<std::vector<T>>
                                    (lbfgs_dtrain_with_trans<T>(alpha,isIntercept),
                                    transData,label,sample_weight,dist_newmodel,distGrad,new_dloss);
      newloss = new_dloss.reduce(add<double>);
      auto diff = newloss - loss;
      //std::cout << "   loss: " << loss << "; newloss: " << newloss 
      //          << "; diff: " << diff << "; factor: " << factor <<"\n";
      if (diff < LBFGS_SEARCH_TOL) {
        RLOG(DEBUG) << "line-search completed in " << j << " iterations!\n";
        break;
      }
      else factor /= static_cast<T>(10.0);
    }

    if (j > LBFGS_MAX_SEARCH_ITER) {
      RLOG(DEBUG) << "LBFGS line search exceeded maximum search step!\n";
    }
    loss = newloss;
    grad_vector = new_grad_vector_part.vector_sum();
    initModel = std::move(new_model);
    auto rms = vector_norm(grad_vector) / sqrt(grad_vector.size());
    opt.update_history(initModel, grad_vector);

    if (i % LBFGS_PRINT_INTERVAL == 0) {
      RLOG(INFO) << "[Iteration: " << i << "]"
                 << " norm(w): " << vector_norm(initModel.weight)
                 << "; norm(g): " << vector_norm(grad_vector)
                 << "; rms: " << rms
                 << "; loss: " << loss
                 << "; stepsize: " << stepsize * factor << std::endl;
    }

    if (rms <= convergenceTol) {
      REPORT_INFO("Convergence achieved in " + ITOS(i) + " iterations!\n");
      break;
    }
  }
  n_iter = (i == numIteration + 1) ? numIteration : i;
  t.show("whole iteration: ");
}

template <class T, class DATA_MATRIX, 
          class MODEL, class GRADIENT, class REGULARIZER>
void lbfgs_parallelizer::do_train(node_local<DATA_MATRIX>& data,
                                  lvec<T>& label,
                                  MODEL& initModel,
                                  lvec<T>& sample_weight,
                                  size_t& n_iter,
                                  size_t numIteration,
                                  double alpha,
                                  double regParam,
                                  bool isIntercept,
                                  double convergenceTol) {
  frovedis::time_spent t(DEBUG);
  lbfgs<T> opt(alpha,hist_size,isIntercept);
  REGULARIZER rType(regParam); // should be L2 always
  auto distGrad = make_node_local_allocate<GRADIENT>();
  auto dloss = make_node_local_allocate<double>();

  auto distModel = initModel.broadcast();
  auto grad_vector = data.template map<std::vector<T>>
                            (lbfgs_dtrain<T>(alpha,isIntercept),
                             label,sample_weight,distModel,distGrad,dloss)
                         .vector_sum();
  auto loss = dloss.reduce(add<double>);
  auto weight = initModel.weight;
  weight.push_back(initModel.intercept);
  opt.old_model = weight;
  opt.old_gradient = grad_vector;

  // -------- main loop --------
  size_t i = 1;
  for(; i <= numIteration; i++) {
    auto step = opt.compute_hkgk(grad_vector);

    // --- line search ---
    // https://github.com/js850/lbfgs_cpp/blob/master/lbfgs.cpp#L216
    // if the step is pointing uphill, invert it
    if (vector_dot(grad_vector, step) > 0) step = step * static_cast<T>(-1.0);
    T factor = 1.0; // alpha
    T stepsize = vector_norm(step);
    if (factor * stepsize > LBFGS_MAX_STEP) factor = LBFGS_MAX_STEP / stepsize;
    auto bias_step = step.back();
    step.pop_back();
    MODEL new_model;
    lvec<T> new_grad_vector_part;
    auto new_dloss = make_node_local_allocate<double>();
    double newloss = 0;
    size_t j = 1;
    for(; j <= LBFGS_MAX_SEARCH_ITER; ++j) {
      new_model.weight = initModel.weight + (factor * step);
      new_model.intercept = initModel.intercept + (factor * bias_step);
      rType.regularize(new_model.weight);
      auto dist_newmodel = new_model.broadcast();
      new_grad_vector_part = data.template map<std::vector<T>>
                                    (lbfgs_dtrain<T>(alpha,isIntercept),
                                    label,sample_weight,dist_newmodel,distGrad,new_dloss);
      newloss = new_dloss.reduce(add<double>);
      auto diff = newloss - loss;
      //std::cout << "   loss: " << loss << "; newloss: " << newloss
      //          << "; diff: " << diff << "; factor: " << factor <<"\n";
      if (diff < LBFGS_SEARCH_TOL) {
        RLOG(DEBUG) << "line-search completed in " << j << " iterations!\n";
        break;
      }
      else factor /= static_cast<T>(10.0);
    }

    if (j > LBFGS_MAX_SEARCH_ITER) {
      RLOG(DEBUG) << "LBFGS line search exceeded maximum search step!\n";
    }
    loss = newloss;
    grad_vector = new_grad_vector_part.vector_sum();
    initModel = std::move(new_model);
    auto rms = vector_norm(grad_vector) / sqrt(grad_vector.size());
    opt.update_history(initModel, grad_vector);

    if (i % LBFGS_PRINT_INTERVAL == 0) {
      RLOG(INFO) << "[Iteration: " << i << "]"
                 << " norm(w): " << vector_norm(initModel.weight)
                 << "; norm(g): " << vector_norm(grad_vector)
                 << "; rms: " << rms
                 << "; loss: " << loss
                 << "; stepsize: " << stepsize * factor << std::endl;
    }

    if (rms <= convergenceTol) {
      REPORT_INFO("Convergence achieved in " + ITOS(i) + " iterations!\n");
      break;
    }
  }
  n_iter = (i == numIteration + 1) ? numIteration : i;
  t.show("whole iteration: ");
}

// --- dense support ---
template <class T, class MODEL, class GRADIENT, class REGULARIZER>
MODEL lbfgs_parallelizer::parallelize(colmajor_matrix<T>& data,
                                      dvector<T>& label,
                                      MODEL& initModel,
                                      std::vector<T>& sample_weight,
                                      size_t& n_iter,
                                      size_t numIteration,
                                      double alpha,
                                      double regParam,
                                      bool isIntercept,
                                      double convergenceTol) {
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
  if(label.sizes() != sizes) label.align_as(sizes);
  auto nloc_label = label.viewas_node_local();
  t0.show("label resize & nloc: ");

  auto nsample_weight = make_dvector_scatter(sample_weight, sizes).moveto_node_local();

  do_train<T,colmajor_matrix_local<T>,MODEL,GRADIENT,REGULARIZER> 
            (data.data,nloc_label,trainedModel,nsample_weight,n_iter,
             numIteration,alpha,regParam,isIntercept,convergenceTol);
  return trainedModel;
}

template <class T, class I, class O, class MODEL, 
          class GRADIENT, class REGULARIZER>
MODEL lbfgs_parallelizer::parallelize(crs_matrix<T,I,O>& data,
                                      dvector<T>& label,
                                      MODEL& initModel,
                                      std::vector<T>& sample_weight,
                                      size_t& n_iter,
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
  if(label.sizes() != sizes) label.align_as(sizes);
  auto nloc_label = label.viewas_node_local();
  t0.show("label resize & nloc: ");

  auto nsample_weight = make_dvector_scatter(sample_weight, sizes).moveto_node_local();

  // -------- selection of input matrix structure --------
  if (mType == CRS) {
    auto trans_crs = data.data.map(to_trans_crs_data<T,I,O>);
    t0.show("to trans crs: ");
    do_train<T,crs_matrix_local<T,I,O>,crs_matrix_local<T,I,O>,
             MODEL,GRADIENT,REGULARIZER> 
            (data.data,trans_crs,nloc_label,trainedModel,nsample_weight,n_iter,
             numIteration,alpha,regParam,isIntercept,convergenceTol);
  }
  else if (mType == HYBRID) {
    auto jds_crs = data.data.map(to_jds_crs_data<T,I,O>);
    t0.show("to jds_crs: ");
    auto trans_jds_crs = data.data.map(to_trans_jds_crs_data<T,I,O>);
    t0.show("to trans jds_crs: ");
    if(inputMovable) { // to free memory
      data.clear();
      t0.show("clear crs data: ");
    }
    do_train<T,jds_crs_hybrid_local<T,I,O>,jds_crs_hybrid_local<T,I,O>,
             MODEL,GRADIENT,REGULARIZER>
            (jds_crs,trans_jds_crs,nloc_label,trainedModel,nsample_weight,n_iter,
             numIteration,alpha,regParam,isIntercept,convergenceTol);
  }
  else if (mType == ELL) {
    auto ell = data.data.map(to_ell_data<T,I,O>);
    t0.show("to ell: ");
    if(inputMovable) { // to free memory
      data.clear();
      t0.show("clear crs data: ");
    }
    do_train<T,ell_matrix_local<T,I>,MODEL,GRADIENT,REGULARIZER>
            (ell,nloc_label,trainedModel,nsample_weight,n_iter,
             numIteration,alpha,regParam,isIntercept,convergenceTol);
  }
  else
    REPORT_ERROR(USER_ERROR,"Unsupported input matrix type!!\n");

  return trainedModel;
}

}
#endif
