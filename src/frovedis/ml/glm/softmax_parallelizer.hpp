
#ifndef _SOFTMAX_PARALLELIZER_HPP_
#define _SOFTMAX_PARALLELIZER_HPP_

#include "common.hpp"
#include <algorithm>

namespace frovedis {

template <class T>
struct multinomial_dtrain {
  multinomial_dtrain(): isIntercept(false) {}
  multinomial_dtrain(bool icpt): isIntercept(icpt) {}

  template <class DATA_MATRIX, class MODEL>
  rowmajor_matrix_local<T> 
  operator()(DATA_MATRIX& data,
             std::vector<T>& label,
             std::vector<T>& sample_weight,
             MODEL& model) {
    softmax_gradient_descent gd(isIntercept);
    auto grad_mat = gd.compute_gradient<T>(data,label,sample_weight,
                    model.weight,model.intercept);
    return grad_mat;
  }

  bool isIntercept;
  SERIALIZE(isIntercept)
};

template <class T>
struct multinomial_dtrain_with_trans {
  multinomial_dtrain_with_trans(): isIntercept(false) {}
  multinomial_dtrain_with_trans(bool icpt): isIntercept(icpt) {}

  template <class DATA_MATRIX, class TRANS_MATRIX, class MODEL>
  rowmajor_matrix_local<T> 
  operator()(DATA_MATRIX& data,
             TRANS_MATRIX& trans,
             std::vector<T>& label,
             std::vector<T>& sample_weight,
             MODEL& model) {
    softmax_gradient_descent gd(isIntercept);
    auto grad_mat = gd.compute_gradient<T>(data,trans,label,sample_weight,
                    model.weight,model.intercept);
    return grad_mat;
  }

  bool isIntercept;
  SERIALIZE(isIntercept)
};

struct softmax_parallelizer {
  softmax_parallelizer() {}
 
  template <class T, class I, class O, 
            class MODEL, class REGULARIZER>
  MODEL parallelize(crs_matrix<T,I,O>& data,
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
  
  template <class T, class MODEL, class REGULARIZER>
  MODEL parallelize(colmajor_matrix<T>& data,
                    dvector<T>& label,
                    MODEL& initModel,
                    std::vector<T>& sample_weight,
                    size_t& n_iter,
                    size_t numIteration,
                    double alpha,
                    double regParam,
                    bool isIntercept,
                    double convergenceTol);


  private:
  template <class T, class DATA_MATRIX, 
            class MODEL, class REGULARIZER>
  void do_train(node_local<DATA_MATRIX>& data,
                node_local<std::vector<T>>& label,
                MODEL& initModel,
                node_local<std::vector<T>>& sample_weight,
                size_t& n_iter,
                size_t numIteration,
                double alpha,
                double regParam,
                bool isIntercept,
                double convergenceTol);

  template <class T, class DATA_MATRIX, class TRANS_MATRIX,
            class MODEL, class REGULARIZER>
  void do_train(node_local<DATA_MATRIX>& data,
                node_local<TRANS_MATRIX>& transData,
                node_local<std::vector<T>>& label,
                MODEL& initModel,
                node_local<std::vector<T>>& sample_weight,
                size_t& n_iter,
                size_t numIteration,
                double alpha,
                double regParam,
                bool isIntercept,
                double convergenceTol);

  template <class T, class MODEL, class REGULARIZER>
  bool update_global_model(MODEL& model,
                           rowmajor_matrix_local<T>& grad,
                           REGULARIZER& rType,
                           double alpha,
                           double convergenceTol,
                           bool isIntercept,
                           size_t iterCount);

  template <class T>
  rowmajor_matrix_local<T>
  get_global_gradient (node_local<rowmajor_matrix_local<T>>& loc_grad,
                       size_t nfeatures, size_t nclasses);

};  


template <class T>
std::vector<T>
get_grad_vector(rowmajor_matrix_local<T>& loc_grad) {
  return loc_grad.val;
}

template <class T>
rowmajor_matrix_local<T>
softmax_parallelizer::get_global_gradient (
     node_local<rowmajor_matrix_local<T>>& loc_grad,
     size_t nfeatures, size_t nclasses) {
  auto tot_grad = loc_grad.map(get_grad_vector<T>).vector_sum();
  rowmajor_matrix_local<T> global_grad;
  global_grad.val.swap(tot_grad);
  global_grad.set_local_num(nfeatures,nclasses);
  return global_grad;
}

template <class T, class MODEL, class REGULARIZER>
bool softmax_parallelizer::update_global_model(
                           MODEL& model,
                           rowmajor_matrix_local<T>& grad,
                           REGULARIZER& rType,
                           double alpha,
                           double convergenceTol,
                           bool isIntercept,
                           size_t iterCount) {
  bool conv = false;
  auto nfeatures = model.nfeatures;
  auto nclasses  = model.nclasses;

#ifdef _CONV_RATE_CHECK_
  MODEL prev_model = model;
#endif

  T reduced_alpha = alpha / sqrt(iterCount);

  T *weightp = &model.weight.val[0];
  T *gradp = &grad.val[0];
  for(size_t i=0; i<nfeatures*nclasses; ++i) 
    weightp[i] -= reduced_alpha * gradp[i];

  if(isIntercept) {
    auto bias = sum_of_rows(grad);
    T *biasp = &bias[0];
    T *icptp = &model.intercept[0];
    for(size_t i=0; i<nclasses; ++i) icptp[i] -= reduced_alpha * biasp[i]; 
  }

  rType.regularize(model.weight.val);

#ifdef _CONV_RATE_CHECK_
  auto diff = model - prev_model;
  T sum = 0;
  T *wp = &diff.weight.val[0];
  T *ip = &diff.intercept[0];
  auto nft = nfeatures;
  for(size_t i=0; i<diff.weight.val.size(); ++i) sum += wp[i] * wp[i];
  if(isIntercept) {
    for(size_t i=0; i<diff.nclasses; ++i) sum += ip[i] * ip[i];
    nft++;
  }
  auto rmse = sqrt(sum/(nft*nclasses));
  if(rmse <= convergenceTol) { 
    conv = true;
    std::string msg = "Convergence achieved in " + ITOS(iterCount) + 
                      " iterations.\nRMS Error: " + ITOS(rmse) + "\n";
    REPORT_INFO(msg);
  }
#endif

  return conv;
}                          

template <class T, class DATA_MATRIX, class TRANS_MATRIX,
          class MODEL, class REGULARIZER>
void softmax_parallelizer::do_train(node_local<DATA_MATRIX>& data,
                                node_local<TRANS_MATRIX>& transData,
                                node_local<std::vector<T>>& label,
                                MODEL& initModel,
                                node_local<std::vector<T>>& sample_weight,
                                size_t& n_iter,
                                size_t numIteration,
                                double alpha,
                                double regParam,
                                bool isIntercept,
                                double convergenceTol) {
  REGULARIZER rType(regParam);
  auto nfeatures = initModel.nfeatures;
  auto nclasses = initModel.nclasses;
  
  // -------- main loop --------
  size_t i;
  for(i = 1; i <= numIteration; i++) {
    auto distModel = initModel.broadcast();

    // work_at_worker
    auto locgrad = data.map(multinomial_dtrain_with_trans<T>(isIntercept),
                     transData,label,sample_weight,distModel);

    // work_at_master
    auto glob_grad = get_global_gradient(locgrad,nfeatures,nclasses);
    bool conv = update_global_model(initModel,glob_grad,rType,alpha,
                                    convergenceTol,isIntercept,i);

#ifdef _CONV_RATE_CHECK_
    if(conv) break;
#endif
  }
  n_iter = (i == numIteration + 1) ? numIteration : i;
}


template <class T, class DATA_MATRIX,
          class MODEL, class REGULARIZER>
void softmax_parallelizer::do_train(node_local<DATA_MATRIX>& data,
                                node_local<std::vector<T>>& label,
                                MODEL& initModel,
                                node_local<std::vector<T>>& sample_weight,
                                size_t& n_iter,
                                size_t numIteration,
                                double alpha,
                                double regParam,
                                bool isIntercept,
                                double convergenceTol) {
  REGULARIZER rType(regParam);
  auto nfeatures = initModel.nfeatures;
  auto nclasses = initModel.nclasses;
  
  // -------- main loop --------
  size_t i;
  for(i = 1; i <= numIteration; i++) {
    auto distModel = initModel.broadcast();

    // work_at_worker
    auto locgrad = data.map(multinomial_dtrain<T>(isIntercept),label,sample_weight,distModel);

    // work_at_master
    auto glob_grad = get_global_gradient(locgrad,nfeatures,nclasses);
    bool conv = update_global_model(initModel,glob_grad,rType,alpha,
                                    convergenceTol,isIntercept,i);

#ifdef _CONV_RATE_CHECK_
    if(conv) break;
#endif
  }
  n_iter = i;
}

template <class T, class I, class O, 
          class MODEL, class REGULARIZER>
MODEL softmax_parallelizer::parallelize(crs_matrix<T,I,O>& data,
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
  label.align_as(sizes);
  auto nloc_label = label.viewas_node_local();
  t0.show("label resize & nloc: ");

  auto nsample_weight = make_dvector_scatter(sample_weight, sizes).moveto_node_local();

  // -------- selection of input matrix structure --------
  if (mType == CRS) {
    auto trans_crs_vec = data.data.map(to_trans_crs_data<T,I,O>);
    t0.show("to trans crs: ");
    do_train<T,crs_matrix_local<T,I,O>,crs_matrix_local<T,I,O>,
             MODEL,REGULARIZER>
               (data.data,trans_crs_vec,nloc_label,trainedModel,nsample_weight,
                n_iter,numIteration,alpha,regParam,isIntercept,convergenceTol);
  }
  
  else if (mType == HYBRID) {
    auto jds_crs_vec = data.data.map(to_jds_crs_data<T,I,O>);
    t0.show("to jds_crs: ");
    auto trans_jds_crs_vec = data.data.map(to_trans_jds_crs_data<T,I,O>);
    t0.show("to trans jds_crs: ");
    if(inputMovable) data.clear();
    do_train<T,jds_crs_hybrid_local<T,I,O>,jds_crs_hybrid_local<T,I,O>,
             MODEL,REGULARIZER>
                (jds_crs_vec,trans_jds_crs_vec,nloc_label,trainedModel,
                 nsample_weight,n_iter,numIteration,alpha,regParam,
                 isIntercept,convergenceTol);
  }
 /*
  * TODO: ell_matrix_local<T,I> * rowmajor_matrix_local<T>
  * TODO: trans_mm(ell_matrix_local<T,I>, rowmajor_matrix_local<T>)
  * On presence of above two functionalities, ELL part can be uncommented...
  *
  else if (mType == ELL) {
    auto ell_vec = data.data.map(to_ell_data<T,I,O>);
    t0.show("to ell: ");
    if(inputMovable) data.clear();
    do_train<T,ell_matrix_local<T,I>,MODEL,REGULARIZER>
               (ell_vec,nloc_label,trainedModel,
                numIteration,alpha,regParam,isIntercept,convergenceTol);
  }
 */
  else REPORT_ERROR(USER_ERROR,"Unsupported input matrix type!!\n");

  return trainedModel;
}

template <class T>
rowmajor_matrix_local<T>
to_trans_rowmajor(rowmajor_matrix_local<T>& m) { return m.transpose(); }

template <class T, class MODEL, class REGULARIZER>
MODEL softmax_parallelizer::parallelize(colmajor_matrix<T>& data,
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
  label.align_as(sizes);
  auto nloc_label = label.viewas_node_local();
  t0.show("label resize & nloc: ");
 
  auto nsample_weight = make_dvector_scatter(sample_weight, sizes).moveto_node_local();

  auto rmat = data.to_rowmajor();
  auto t_rmat_data = rmat.data.map(to_trans_rowmajor<T>);
  do_train<T,rowmajor_matrix_local<T>,rowmajor_matrix_local<T>,
           MODEL,REGULARIZER>
         (rmat.data, t_rmat_data, nloc_label, trainedModel, nsample_weight,
          n_iter, numIteration, alpha, regParam, isIntercept, convergenceTol);

  return trainedModel;
}

}

#endif
