#ifndef GMM_HPP
#define GMM_HPP

#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>

#include <frovedis.hpp>
#include <frovedis/matrix/blas_wrapper.hpp>
#include <frovedis/matrix/colmajor_matrix.hpp>
#include <frovedis/matrix/lapack_wrapper.hpp>
#include <frovedis/ml/clustering/kmeans.hpp>
#include <fstream>
#include <iostream>
#include <random>

namespace frovedis {

template <typename T>
std::vector<T> init_step_mu(frovedis::colmajor_matrix_local<T> &Data, int CNT);

template <typename T>
std::vector<T> init_step_cov(frovedis::colmajor_matrix_local<T> &Data, int CNT,
                             frovedis::colmajor_matrix_local<T> &mu,
                             std::vector<T> &r_arr);

template <typename T>
std::vector<T> E_step(frovedis::colmajor_matrix_local<T> &Data, int K,
                      frovedis::colmajor_matrix_local<T> &mu,
                      frovedis::colmajor_matrix_local<T> &cov,
                      frovedis::colmajor_matrix_local<T> &Pi_arr,
                      frovedis::colmajor_matrix_local<T> &MD);

template <typename T>
std::vector<T> M_step_mu(frovedis::colmajor_matrix_local<T> &Data, int K,
                         std::vector<T> &r_arr);

template <typename T>
std::vector<T> M_step_cov(frovedis::colmajor_matrix_local<T> &Data, int K,
                          frovedis::colmajor_matrix_local<T> &mu,
                          std::vector<T> &r_arr);

template <typename T>
T likelihood(frovedis::colmajor_matrix_local<T> &Data,
             frovedis::colmajor_matrix_local<T> &MD, int K);

template <typename T>
frovedis::colmajor_matrix_local<T> make_gdp(
    const frovedis::colmajor_matrix_local<T> &data, int K) {
  frovedis::colmajor_matrix_local<T> ret(data.local_num_row, K);
  for (int i = 0; i < data.local_num_row * K; i++) {
    ret.val[i] = 0.0;
  }
  return ret;
}

template <typename T>
T my_sum(T a, T b) {
  return a + b;
}

template <typename T>
struct gmm_model {
  frovedis::rowmajor_matrix_local<T> cluster;
  frovedis::rowmajor_matrix_local<T> model;
  frovedis::rowmajor_matrix_local<T> covariance;
  frovedis::rowmajor_matrix_local<T> pi;
  T likelihood;
  int loops;
  bool converged;  
  int Number;
  void save(const std::string &path) {
    frovedis::make_directory(path);
    //model.transpose().save(path + "/model_means");
    model.save(path + "/model_means");
    covariance.save(path + "/model_cov");
    pi.save(path + "/model_pi");
  }
    
  void savebinary(const std::string &path) {
    frovedis::make_directory(path);
    //model.transpose().savebinary(path + "/model_means");
    model.savebinary(path + "/model_means");
    covariance.savebinary(path + "/model_cov");
    pi.savebinary(path + "/model_pi");
  }
    
  void load(const std::string &path) {
    model = make_rowmajor_matrix_local_load<T>(path + "/model_means");
    covariance = make_rowmajor_matrix_local_load<T>(path + "/model_cov");
    pi = make_rowmajor_matrix_local_load<T>(path + "/model_pi");          
  }
    
  void loadbinary(const std::string &path) {
    model = make_rowmajor_matrix_local_loadbinary<T>(path + "/model_means");
    covariance = make_rowmajor_matrix_local_loadbinary<T>(path + "/model_cov");
    pi = make_rowmajor_matrix_local_loadbinary<T>(path + "/model_pi");      
  }
  
  void debug_print() {
    std::cout << "Means:\n";    
    model.debug_print();
    std::cout << "\nWeights:\n";    
    pi.debug_print();
    std::cout << "\nCovariances:\n";    
    covariance.debug_print();      
  }    
};

template <typename T>
struct gmm_cluster {
  frovedis::rowmajor_matrix_local<int> predict;
  frovedis::rowmajor_matrix_local<T> predict_prob;
  void save(const std::string &path) {
    frovedis::make_directory(path);
    predict.transpose().save(path + "/predict");
    predict_prob.transpose().save(path + "/predict_prob");
  }
  void savebinary(const std::string &path) {
    frovedis::make_directory(path);
    predict.transpose().savebinary(path + "/predict");
    predict_prob.transpose().savebinary(path + "/predict_prob");
  }
};

template <typename T>
double gmm_score(frovedis::rowmajor_matrix<T> Data, int CNT) {
  frovedis::colmajor_matrix<T> X1(Data);
  auto p_CNT = frovedis::make_node_local_broadcast(CNT);
  int Num = Data.num_row;
  auto p_MD = X1.data.map(make_gdp<T>, p_CNT);
  auto partial_log = X1.data.map(likelihood<T>, p_MD, p_CNT);
  auto ret = partial_log.reduce(my_sum<T>);
  return ret / static_cast<double>(Num);
}

template <typename T>
gmm_model<T> gmm(frovedis::rowmajor_matrix<T> Data, int CNT, int loopmax,
                 double epsilon, bool switch_init = 0, long seed = 123) {
  int Dim = Data.num_col;
  int Num = Data.num_row;
  frovedis::colmajor_matrix<T> X1(Data);
  auto &X = X1.data;
  auto p_CNT = frovedis::make_node_local_broadcast(CNT);
  frovedis::colmajor_matrix_local<T> ml(Dim, CNT);
  frovedis::colmajor_matrix_local<T> covl(Dim * Dim, CNT);
  frovedis::colmajor_matrix_local<T> Pi_arr(CNT, 1);
  std::vector<T> r_arrl(Num * CNT);
  frovedis::colmajor_matrix_local<T> cll(Num, CNT);
  T lognew = 0.0;
  gmm_model<T> ret;
  ret.Number = Num;
  int loop;
  if (switch_init == 0) {
    int num_iteration = 100;
    T eps = 0.01;
    int kmeans_iteration = 10;
    auto mu_kmeans =
        frovedis::kmeans(Data, CNT, num_iteration, eps, seed, kmeans_iteration);
    frovedis::colmajor_matrix_local<T> mu_kmean(mu_kmeans);
    for (int k = 0; k < CNT; k++) {
      for (int d1 = 0; d1 < Dim; d1++) {
        ml.val[k * Dim + d1] = mu_kmean.val[k * Dim + d1];
      }
    }
    auto assigned = parallel_kmeans_assign_cluster(Data, mu_kmeans);
    for (int n = 0; n < Num; n++) {
      Pi_arr.val[assigned[n]] += 1; // TODO: use vector_bincount here to vectorize this count
      r_arrl[n * CNT + assigned[n]] = 1;
    }
    for (int k = 0; k < CNT; k++) {
      Pi_arr.val[k] = Pi_arr.val[k] / Num;
    }
  } else if (switch_init == 1) {
    auto partial_init_mu = X.map(init_step_mu<T>, p_CNT);
    auto init_mu = partial_init_mu.vector_sum();
    T *sum_for_mu = init_mu.data();
    for (int k = 0; k < CNT; k++) {
      for (int d = 0; d < Dim; d++) {
        ml.val[k * Dim + d] = sum_for_mu[k * Dim + d];
      }
      for (int n = 0; n < Num; n++) {
        r_arrl[n * CNT + k] = 1.0 / CNT;
      }
      Pi_arr.val[k] = 1.0 / CNT;
    }
  } else {
    throw std::runtime_error("incorrect switch_init: " +
                             std::to_string(switch_init));
  }
  auto mu_init = broadcast(ml);
  auto k_means_mu = make_dvector_scatter(r_arrl);
  auto num_rows = Data.get_local_num_rows();
  size_t nodesize = get_nodesize();
  std::vector<size_t> sizes(nodesize);
  for (size_t i = 0; i < nodesize; i++) sizes[i] = num_rows[i] * CNT;
  k_means_mu.align_as(sizes);
  auto k_means_mu2 = k_means_mu.moveto_node_local();
  auto partial_init = X.map(init_step_cov<T>, p_CNT, mu_init, k_means_mu2);
  auto total_init = partial_init.vector_sum();
  T *i_cov = total_init.data();
  for (int k = 0; k < CNT; k++) {
    for (int d1 = 0; d1 < Dim; d1++) {
      for (int d2 = d1 + 1; d2 < Dim; d2++) {
        covl.val[k * Dim * Dim + d1 * Dim + d2] =
            i_cov[k * Dim * Dim + d1 * Dim + d2] / (Num * Pi_arr.val[k]);
        covl.val[k * Dim * Dim + d2 * Dim + d1] =
            covl.val[k * Dim * Dim + d1 * Dim + d2];
      }
      covl.val[k * Dim * Dim + d1 * Dim + d1] =
          i_cov[k * Dim * Dim + d1 * Dim + d1] / (Num * Pi_arr.val[k]);
    }
  }
  auto p_MD = X.map(make_gdp<T>, p_CNT);
  T logold = 1.0;
  for (loop = 1; loop < loopmax; loop++) {
    auto mu = broadcast(ml);
    auto cov = broadcast(covl);
    auto V_Pi = broadcast(Pi_arr);
    auto r_arr = X.map(E_step<T>, p_CNT, mu, cov, V_Pi, p_MD);
    auto partial_log = X.map(likelihood<T>, p_MD, p_CNT);
    lognew = partial_log.reduce(my_sum<T>);
    lognew /= Num;
    if (abs(logold - lognew) < epsilon) {
      auto r_arrval = r_arr.template as_dvector<T>().gather();
      frovedis::rowmajor_matrix_local<T> clu_save(CNT, Num);
      for (int k = 0; k < CNT; k++) {
        for (int n = 0; n < Num; n++) {
          clu_save.val[k * Num + n] = r_arrval[n * CNT + k];
        }
      }
      ret.cluster = clu_save;
      ret.converged = true;  
      break;
    } else {
      logold = lognew;
      if (loop == loopmax - 1) {
        auto r_arrval = r_arr.template as_dvector<T>().gather();
        frovedis::rowmajor_matrix_local<T> clu_save(CNT, Num);
        for (int k = 0; k < CNT; k++) {
          for (int n = 0; n < Num; n++) {
            clu_save.val[k * Num + n] = r_arrval[n * CNT + k];
          }
        }
        ret.cluster = clu_save;
      }
    }
    auto partial_sum = X.map(M_step_mu<T>, p_CNT, r_arr);
    auto total_sum = partial_sum.vector_sum();
    T *sum_for_mu = total_sum.data();
    T *sum_for_n_k_arr = sum_for_mu + CNT * Dim;
    for (int k = 0; k < CNT; k++) {
      for (int d = 0; d < Dim; d++) {
        ml.val[k * Dim + d] = sum_for_mu[k * Dim + d] / sum_for_n_k_arr[k];
      }
      Pi_arr.val[k] = sum_for_n_k_arr[k] / Num;
    }
    auto mu2 = broadcast(ml);
    auto partial_cov = X.map(M_step_cov<T>, p_CNT, mu2, r_arr);
    auto total_cov = partial_cov.vector_sum();
    T *s_cov = total_cov.data();
    for (int k = 0; k < CNT; k++) {
      for (int d1 = 0; d1 < Dim; d1++) {
        for (int d2 = d1 + 1; d2 < Dim; d2++) {
          covl.val[k * Dim * Dim + d1 * Dim + d2] =
              s_cov[k * Dim * Dim + d1 * Dim + d2] / sum_for_n_k_arr[k];
          covl.val[k * Dim * Dim + d2 * Dim + d1] =
              covl.val[k * Dim * Dim + d1 * Dim + d2];
        }
        covl.val[k * Dim * Dim + d1 * Dim + d1] =
            s_cov[k * Dim * Dim + d1 * Dim + d1] / sum_for_n_k_arr[k];
      }
    }
  }
/*
  frovedis::rowmajor_matrix_local<T> mu_save(Dim, CNT);
  for (int k = 0; k < CNT; k++) {
    for (int d = 0; d < Dim; d++) {
      mu_save.val[d * CNT + k] = ml.val[k * Dim + d];
    }
  }
*/
  frovedis::rowmajor_matrix_local<T> mu_save;    
  mu_save.val.swap(ml.val);
  mu_save.set_local_num(CNT, Dim);

  frovedis::rowmajor_matrix_local<T> cov_save(CNT, Dim * Dim);
  for (int k = 0; k < CNT; k++) {
    for (int d0 = 0; d0 < Dim; d0++) {
      for (int d1 = 0; d1 < Dim; d1++) {
        cov_save.val[k * Dim * Dim + d0 * Dim + d1] =
            covl.val[k * Dim * Dim + d0 * Dim + d1];
      }
    }
  }
  frovedis::rowmajor_matrix_local<T> pi_save;
  pi_save.val.swap(Pi_arr.val);
  pi_save.set_local_num(CNT, 1);

  ret.model = mu_save;
  ret.covariance = cov_save;
  ret.pi = pi_save;
  ret.likelihood = lognew;
  ret.loops = loop;
  return ret;
}

template <typename T>
std::vector<T> init_step_mu(frovedis::colmajor_matrix_local<T> &Data, int CNT) {
  int Dim = Data.local_num_col;
  int Num = Data.local_num_row;
  std::vector<T> ret(CNT * Dim * Dim);
  T *sum_for_mu = ret.data();
  std::random_device rnd;
  std::mt19937 mt(rnd());
  std::uniform_int_distribution<> rand(0, Num);
  std::vector<int> random_arr;
  for (int k = 0; k < CNT; k++) {
    for (int d = 0; d < Dim; d++) {
      sum_for_mu[k * Dim + d] = Data.val[rand(mt) * Dim + d];
    }
  }
  return ret;
}

template <typename T>
std::vector<T> init_step_cov(frovedis::colmajor_matrix_local<T> &Data, int CNT,
                             frovedis::colmajor_matrix_local<T> &mu,
                             std::vector<T> &r_arr) {
  int Dim = Data.local_num_col;
  int Num = Data.local_num_row;
  std::vector<T> ret(CNT * Dim * Dim);
  T *s_cov = ret.data();
  for (int k = 0; k < CNT; k++) {
    for (int d1 = 0; d1 < Dim; d1++) {
      for (int d2 = d1; d2 < Dim; d2++) {
#pragma _NEC select_vector
        for (int n = 0; n < Num; n++) {
          s_cov[k * Dim * Dim + d1 * Dim + d2] +=
              r_arr[n * CNT + k] *
              (Data.val[d1 * Num + n] - mu.val[k * Dim + d1]) *
              (Data.val[d2 * Num + n] - mu.val[k * Dim + d2]);
        }
      }
    }
  }
  return ret;
}

template <typename T>
std::vector<T> E_step(frovedis::colmajor_matrix_local<T> &Data, int K,
                      frovedis::colmajor_matrix_local<T> &mu,
                      frovedis::colmajor_matrix_local<T> &cov,
                      frovedis::colmajor_matrix_local<T> &Pi_arr,
                      frovedis::colmajor_matrix_local<T> &MD) {
  int CNT = K;
  int Dim = Data.local_num_col;
  int Num = Data.local_num_row;
  std::vector<int> ipiv(Dim);
  std::vector<T> Vec(Dim * Num);
  std::vector<T> r_arr(CNT * Num);
  std::vector<T> IP(CNT * Num);
  std::vector<T> Pi_arr_Sum(Num);
  std::vector<T> MND(CNT * Num);
  T Det;
  const T PI = 4 * atan(1);
  const T Log2PI = log(2 * PI);
  frovedis::colmajor_matrix_local<T> covl(Dim, Dim);
  for (int k = 0; k < CNT; k++) {
    for (int dd = 0; dd < Dim * Dim; dd++) {
      covl.val[dd] = cov.val[Dim * Dim * k + dd];
    }
    for (int dd = 0; dd < Dim * Dim; dd = dd + 1 + Dim) {
      covl.val[dd] += std::pow(10, -6);
    }
    frovedis::getrf<T>(covl, ipiv);
    Det = 1.0;
    for (int dd = 0; dd < Dim * Dim; dd = dd + 1 + Dim) {
      Det *= covl.val[dd];
    }
    Det = fabs(Det);
    frovedis::getri<T>(covl, ipiv);
    auto covp = covl.val.data();
    auto Vecp = Vec.data();
    auto Datap = Data.val.data();
    auto muvalp = mu.val.data();
    auto IPp = IP.data();

    for (int d = 0; d < Dim; d++) {
#pragma _NEC select_vector
      for (int n = 0; n < Num; n++) {
        Vecp[d * Num + n] = Datap[d * Num + n] - muvalp[k * Dim + d];
      }
    }
    for (int d = 0; d < Dim; d++) {
      for (int n = 0; n < Num; n++) {
        IPp[k * Num + n] +=
            covp[Dim * d + d] * Vecp[d * Num + n] * Vecp[d * Num + n];
      }
    }
    for (int d0 = 0; d0 < Dim - 1; d0++) {
      for (int d = d0 + 1; d < Dim; d++) {
#pragma _NEC select_vector
        for (int n = 0; n < Num; n++) {
          IPp[k * Num + n] +=
              2 * covp[Dim * d + d0] * Vecp[d0 * Num + n] * Vecp[d * Num + n];
        }
      }
    }
    for (int n1 = 0; n1 < Num; n1++) {
      MND[k * Num + n1] = Pi_arr.val[k] * exp(-IP[k * Num + n1] / 2.0) /
                          (std::pow(sqrt(2 * PI), Dim)) / sqrt(Det);
      MD.val[k * Num + n1] = -IP[k * Num + n1] / 2.0 + log(Pi_arr.val[k]) -
                             (Dim * Log2PI + log(Det)) / 2.0;
    }
  }
  for (int k = 0; k < CNT; k++) {
    for (int n = 0; n < Num; n++) {
      Pi_arr_Sum[n] += MND[k * Num + n];
    }
  }
  for (int k = 0; k < CNT; k++) {
    for (int n = 0; n < Num; n++) {
      r_arr[n * CNT + k] = MND[k * Num + n] / Pi_arr_Sum[n];
    }
  }
  return r_arr;
}

template <typename T>
T likelihood(frovedis::colmajor_matrix_local<T> &Data,
             frovedis::colmajor_matrix_local<T> &MD, int CNT) {
  int Num = Data.local_num_row;
  T likelihoodsum;
  T expmax;
  T logsum = 0.0;
  for (int n = 0; n < Num; n++) {
    expmax = MD.val[0 * Num + n];
    for (int k = 1; k < CNT; k++) {
      expmax = std::max(expmax, MD.val[k * Num + n]);
    }
    likelihoodsum = 0.0;
    for (int k = 0; k < CNT; k++) {
      likelihoodsum += exp(MD.val[k * Num + n] - expmax);
    }
    logsum += log(likelihoodsum) + expmax;
  }
  return logsum;
}

template <typename T>
std::vector<T> M_step_mu(frovedis::colmajor_matrix_local<T> &Data, int CNT,
                         std::vector<T> &r_arr) {
  int Dim = Data.local_num_col;
  int Num = Data.local_num_row;
  std::vector<T> ret(CNT * (Dim + 1));
  T *sum_for_mu = ret.data();
  T *sum_for_n_k_arr = sum_for_mu + CNT * Dim;
  for (int k = 0; k < CNT; k++) {
#pragma _NEC select_vector
    for (int n = 0; n < Num; n++) {
      sum_for_n_k_arr[k] += r_arr[n * CNT + k];
    }
    for (int d = 0; d < Dim; d++) {
#pragma _NEC select_vector
      for (int n = 0; n < Num; n++) {
        sum_for_mu[k * Dim + d] += Data.val[d * Num + n] * r_arr[n * CNT + k];
      }
    }
  }
  return ret;
}

template <typename T>
std::vector<T> M_step_cov(frovedis::colmajor_matrix_local<T> &Data, int CNT,
                          frovedis::colmajor_matrix_local<T> &mu,
                          std::vector<T> &r_arr) {
  int Dim = Data.local_num_col;
  int Num = Data.local_num_row;
  std::vector<T> ret(CNT * Dim * Dim);
  T *s_cov = ret.data();
  for (int k = 0; k < CNT; k++) {
    for (int d1 = 0; d1 < Dim; d1++) {
      for (int d2 = d1; d2 < Dim; d2++) {
#pragma _NEC select_vector
        for (int n = 0; n < Num; n++) {
          s_cov[k * Dim * Dim + d1 * Dim + d2] +=
              r_arr[n * CNT + k] *
              (Data.val[d1 * Num + n] - mu.val[k * Dim + d1]) *
              (Data.val[d2 * Num + n] - mu.val[k * Dim + d2]);
        }
      }
    }
  }
  return ret;
}

template <typename T>
gmm_cluster<T> gmm_assign_cluster(frovedis::rowmajor_matrix_local<T> NewData, int K,
                                  frovedis::rowmajor_matrix_local<T> input_mu, 
                                  frovedis::rowmajor_matrix_local<T> input_cov,
                                  frovedis::rowmajor_matrix_local<T> input_pi) {
  
  frovedis::colmajor_matrix_local<T> Data(NewData);
  frovedis::colmajor_matrix_local<T> mul(input_mu);
  frovedis::colmajor_matrix_local<T> covll(input_cov);
  frovedis::colmajor_matrix_local<T> Pi_arr(input_pi);
    
  gmm_cluster<T> ret;
  int CNT = K;
  int Dim = Data.local_num_col;
  int Num = Data.local_num_row;
  std::vector<int> ipiv(Dim);
  std::vector<T> Vec(Dim * Num);
  frovedis::colmajor_matrix_local<T> mu(Dim, CNT);

  for (int k = 0; k < CNT; k++) {
    for (int d = 0; d < Dim; d++) {
      mu.val[k * Dim + d] = mul.val[d * CNT + k];
    }
  }
  frovedis::colmajor_matrix_local<T> cov(Dim * Dim, CNT);
  for (int k = 0; k < CNT; k++) {
    for (int d1 = 0; d1 < Dim; d1++) {
      for (int d2 = 0; d2 < Dim; d2++) {
        cov.val[k * Dim * Dim + d1 * Dim + d2] =
            covll.val[d2 * Dim * CNT + d1 * CNT + k];
      }
    }
  }
  std::vector<T> r_arr(CNT * Num);
  std::vector<T> IP(CNT * Num);
  std::vector<T> Pi_arr_Sum(Num);
  std::vector<T> MND(CNT * Num);
  T Det;
  const T PI = 4 * atan(1);
  frovedis::colmajor_matrix_local<T> covl(Dim, Dim);
  for (int k = 0; k < CNT; k++) {
    for (int dd = 0; dd < Dim * Dim; dd++) {
      covl.val[dd] = cov.val[Dim * Dim * k + dd];
    }
    for (int dd = 0; dd < Dim * Dim; dd = dd + 1 + Dim) {
      covl.val[dd] += std::pow(10, -6);
    }
    frovedis::getrf<T>(covl, ipiv);
    Det = 1.0;
    for (int dd = 0; dd < Dim * Dim; dd = dd + 1 + Dim) {
      Det *= covl.val[dd];
    }
    Det = fabs(Det);
    frovedis::getri<T>(covl, ipiv);
    auto covp = covl.val.data();
    auto Vecp = Vec.data();
    auto Datap = Data.val.data();
    auto muvalp = mu.val.data();
    auto IPp = IP.data();

    for (int d = 0; d < Dim; d++) {
#pragma _NEC select_vector
      for (int n = 0; n < Num; n++) {
        Vecp[d * Num + n] = Datap[d * Num + n] - muvalp[k * Dim + d];
      }
    }
    for (int d = 0; d < Dim; d++) {
      for (int n = 0; n < Num; n++) {
        IPp[k * Num + n] +=
            covp[Dim * d + d] * Vecp[d * Num + n] * Vecp[d * Num + n];
      }
    }
    for (int d0 = 0; d0 < Dim - 1; d0++) {
      for (int d = d0 + 1; d < Dim; d++) {
#pragma _NEC select_vector
        for (int n = 0; n < Num; n++) {
          IPp[k * Num + n] +=
              2 * covp[Dim * d + d0] * Vecp[d0 * Num + n] * Vecp[d * Num + n];
        }
      }
    }
    for (int n1 = 0; n1 < Num; n1++) {
      MND[k * Num + n1] = Pi_arr.val[k] * exp(-IP[k * Num + n1] / 2.0) /
                          (std::pow(sqrt(2 * PI), Dim)) / sqrt(Det);
    }
  }

  for (int k = 0; k < CNT; k++) {
    for (int n = 0; n < Num; n++) {
      Pi_arr_Sum[n] += MND[k * Num + n];
    }
  }

  for (int k = 0; k < CNT; k++) {
    for (int n = 0; n < Num; n++) {
      r_arr[n * CNT + k] = MND[k * Num + n] / Pi_arr_Sum[n];
    }
  }
  frovedis::rowmajor_matrix_local<T> predict_pro(CNT, Num);
  for (int k = 0; k < CNT; k++) {
    for (int n = 0; n < Num; n++) {
      predict_pro.val[k * Num + n] = r_arr[n * CNT + k];
    }
  }
  ret.predict_prob = predict_pro;
  int cnt;
  frovedis::rowmajor_matrix_local<int> assign_cluster(1, Num);
  for (int n = 0; n < Num; n++) {
    cnt = 0;
    auto now = r_arr[n * CNT];
    for (int k = 1; k < CNT; k++) {
      if (now < r_arr[n * CNT + k]) {
        now = r_arr[n * CNT + k];
        cnt = k;
      }
    }
    assign_cluster.val[n] = cnt;
  }
  ret.predict_prob = predict_pro;
  ret.predict = assign_cluster;
  return ret;
}    
}  // namespace frovedis
#endif
