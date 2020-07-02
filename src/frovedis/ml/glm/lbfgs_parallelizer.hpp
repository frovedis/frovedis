#ifndef _LBFGS_PARALLELIZER_HPP_
#define _LBFGS_PARALLELIZER_HPP_

#include "common.hpp"
#include "lbfgs.hpp"

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
             MODEL& model,
             GRADIENT& loss) {
    gradient_descent gd(alpha, isIntercept);
    return gd.compute_gradient<T>(data,model,label,loss);
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
             MODEL& model,
             GRADIENT& loss) {
    gradient_descent gd(alpha, isIntercept);
    return gd.compute_gradient<T>(data,trans,model,label,loss);
  }

  double alpha;
  bool isIntercept;
  SERIALIZE(alpha, isIntercept)
};

struct lbfgs_parallelizer {
  lbfgs_parallelizer(): hist_size(10) {}
  lbfgs_parallelizer(size_t hsize): hist_size(hsize)
    { checkAssumption(hsize > 0); }

  template <class T, class MODEL, class GRADIENT, class REGULARIZER>
  MODEL parallelize (colmajor_matrix<T>& data,
                     dvector<T>& label,
                     MODEL& initModel,
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
                 size_t numIteration,
                 double alpha,
                 double regParam,
                 bool isIntercept,
                 double convergenceTol);

  template <class T, class MODEL, class REGULARIZER>
  bool update_global_model(MODEL& initModel,
                           lvec<T>& grad_vector_part,
                           lbfgs<T>& lbfgs,
                           REGULARIZER& rType,
                           double convergenceTol,
                           size_t iterCount,
                           time_spent t);

  size_t hist_size;
  SERIALIZE(hist_size)
};

template <class T, class MODEL, class REGULARIZER>
inline bool
lbfgs_parallelizer::update_global_model(MODEL& initModel, 
                                        lvec<T>& grad_vector_part,
                                        lbfgs<T>& opt,
                                        REGULARIZER& rType,
                                        double convergenceTol,
                                        size_t iterCount,
                                        time_spent t) {
  bool conv = false;
  auto grad_vector = grad_vector_part.vector_sum(); // reduce grad-vectors
  t.show("reduce: ");

#ifdef _CONV_RATE_CHECK_
  MODEL prev_model = initModel;
#endif

  opt.optimize(grad_vector,initModel,iterCount,rType);
  t.show("update and regularize: ");

#ifdef _CONV_RATE_CHECK_
  MODEL& cur_model = initModel; // updated initModel
  if(is_converged(cur_model,prev_model,convergenceTol,iterCount)) conv = true;
#endif

  return conv;
}

template <class T, class DATA_MATRIX, class TRANS_MATRIX, 
          class MODEL, class GRADIENT, class REGULARIZER>
void lbfgs_parallelizer::do_train(node_local<DATA_MATRIX>& data,
                                  node_local<TRANS_MATRIX>& transData,
                                  lvec<T>& label,
                                  MODEL& initModel,
                                  size_t numIteration,
                                  double alpha,
                                  double regParam,
                                  bool isIntercept,
                                  double convergenceTol) {
  frovedis::time_spent t(DEBUG), t2(DEBUG);
  lbfgs<T> opt(alpha,hist_size,isIntercept);
  REGULARIZER rType(regParam);
  auto distLoss = make_node_local_allocate<GRADIENT>();

  // -------- main loop --------
  for(size_t i = 1; i <= numIteration; i++) {
    frovedis::time_spent t3(TRACE);
    auto distModel = initModel.broadcast();
    t3.show("broadcast: ");

    // work_at_worker
    auto grad_vector_part = data.template map<std::vector<T>>
                            (lbfgs_dtrain_with_trans<T>(alpha,isIntercept),
                             transData,label,distModel,distLoss);
    t3.show("Dtrain: ");

    // work_at_master
    bool conv = update_global_model<T>(initModel,grad_vector_part,opt,
                                       rType,convergenceTol,i,t3);
#ifdef _CONV_RATE_CHECK_
    if(conv) break;
#endif

    std::string msg = "[Iteration: " + ITOS(i) + "] elapsed-time: ";
    t.show(msg);
  }
  t2.show("whole iteration: ");
}

template <class T, class DATA_MATRIX, 
          class MODEL, class GRADIENT, class REGULARIZER>
void lbfgs_parallelizer::do_train(node_local<DATA_MATRIX>& data,
                                  lvec<T>& label,
                                  MODEL& initModel,
                                  size_t numIteration,
                                  double alpha,
                                  double regParam,
                                  bool isIntercept,
                                  double convergenceTol) {
  frovedis::time_spent t(DEBUG), t2(DEBUG);
  lbfgs<T> opt(alpha,hist_size,isIntercept);
  REGULARIZER rType(regParam);
  auto distLoss = make_node_local_allocate<GRADIENT>();

  // -------- main loop --------
  for(size_t i = 1; i <= numIteration; i++) {
    frovedis::time_spent t3(TRACE);
    auto distModel = initModel.broadcast();
    t3.show("broadcast: ");

    // work_at_worker
    auto grad_vector_part = data.template map<std::vector<T>>
                            (lbfgs_dtrain<T>(alpha,isIntercept),
                             label,distModel,distLoss);
    t3.show("Dtrain: ");

    // work_at_master
    bool conv = update_global_model<T>(initModel,grad_vector_part,opt,
                                       rType,convergenceTol,i,t3);
#ifdef _CONV_RATE_CHECK_
    if(conv) break;
#endif

    std::string msg = "[Iteration: " + ITOS(i) + "] elapsed-time: ";
    t.show(msg);
  }
  t2.show("whole iteration: ");
}

// --- dense support ---
template <class T, class MODEL, class GRADIENT, class REGULARIZER>
MODEL lbfgs_parallelizer::parallelize(colmajor_matrix<T>& data,
                                      dvector<T>& label,
                                      MODEL& initModel,
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
  label.align_as(sizes);
  auto nloc_label = label.viewas_node_local();
  t0.show("label resize & nloc: ");

  do_train<T,colmajor_matrix_local<T>,MODEL,GRADIENT,REGULARIZER> 
            (data.data,nloc_label,trainedModel,
             numIteration,alpha,regParam,isIntercept,convergenceTol);
  return trainedModel;
}

template <class T, class I, class O, class MODEL, 
          class GRADIENT, class REGULARIZER>
MODEL lbfgs_parallelizer::parallelize(crs_matrix<T,I,O>& data,
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

  // -------- selection of input matrix structure --------
  if (mType == CRS) {
    auto trans_crs = data.data.map(to_trans_crs_data<T,I,O>);
    t0.show("to trans crs: ");
    do_train<T,crs_matrix_local<T,I,O>,crs_matrix_local<T,I,O>,
             MODEL,GRADIENT,REGULARIZER> 
            (data.data,trans_crs,nloc_label,trainedModel,
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
            (jds_crs,trans_jds_crs,nloc_label,trainedModel,
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
            (ell,nloc_label,trainedModel,
             numIteration,alpha,regParam,isIntercept,convergenceTol);
  }
  else
    REPORT_ERROR(USER_ERROR,"Unsupported input matrix type!!\n");

  return trainedModel;
}

}

#endif
