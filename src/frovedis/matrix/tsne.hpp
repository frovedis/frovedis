#ifndef _TSNE_HPP_
#define _TSNE_HPP_

#if defined(_SX) || defined(__ve__)
#include <asl.h>
#else
#include <random>
#endif

#include <frovedis/matrix/rowmajor_matrix.hpp>
#include <frovedis/ml/clustering/common.hpp>

#define TOLERANCE 1e-5
#define INITIAL_MOMENTUM 0.5
#define FINAL_MOMENTUM 0.8
#define GRAD_NORM 4.0
#define PGAIN 0.2
#define NGAIN 0.8
#define MINGAIN 0.01
#define MEAN 0.0
#define VAR 0.0001
#define GK_N_ITER 256
#define N_ITER_CHECK 50
#define EXPLORATION_N_ITER 250
#define DESIRED_PRECISION 1e10

namespace frovedis {

template <class T>
T sum(T& a, T& b) { return a + b; }

template <class T>
std::vector<T>
get_row_max(const rowmajor_matrix_local<T>& mat) {
  size_t nrow = mat.local_num_row;
  size_t ncol = mat.local_num_col;
  auto m_valp = mat.val.data();
  std::vector<T> row_max(nrow, std::numeric_limits<T>::min());
  auto rmax_valp = row_max.data();
  if (nrow > ncol) {
#pragma _NEC nointerchange
    for(size_t c = 0; c < ncol; c++) {
      for(size_t r = 0; r < nrow; r++) {
        if (m_valp[r * ncol + c] > rmax_valp[r]) {
          rmax_valp[r] = m_valp[r * ncol + c];
        }
      }
    }
  }
  else {
#pragma _NEC nointerchange
    for(size_t r = 0; r < nrow; r++) {
      for(size_t c = 0; c < ncol; c++) {
        if (m_valp[r * ncol + c] > rmax_valp[r]) {
          rmax_valp[r] = m_valp[r * ncol + c];
        }
      }
    }
  }
  return row_max;
}

template <class T>
rowmajor_matrix_local<T>
compute_probability_matrix (
  const rowmajor_matrix_local<T>& exp_matrix) {
  auto sum_vec = sum_of_cols(exp_matrix);
  diag_matrix_local<T> one_by_tot(sum_vec.size());
  auto diagp = one_by_tot.val.data();
  auto sum_vecp = sum_vec.data();
  for(size_t i = 0; i < sum_vec.size(); ++i) {
    if(sum_vecp[i] != 0.0) diagp[i] = 1.0 / sum_vecp[i];
    else                   diagp[i] = 1.0;
  }
  return one_by_tot * exp_matrix;
}

template <class T>
rowmajor_matrix_local<T> 
compute_gaussian_kernel(const rowmajor_matrix_local<T>& mat, 
                        const std::vector<T>& row_max,
                        double perplexity) {
  size_t nrow = mat.local_num_row;
  size_t ncol = mat.local_num_col;
  rowmajor_matrix_local<T> ret(nrow, ncol);
  auto retp = ret.val.data();
  auto mvalp = mat.val.data();
  auto row_max_valp = row_max.data();

  // Compute the Gaussian kernel row by row
  for(size_t r = 0; r < nrow; r++) {
    // Initialize some variables
    double beta = 1.0;
    double min_beta = -DBL_MAX;
    double max_beta =  DBL_MAX;
    double sum_P = 0.0;
    // Iterate until we found a good perplexity
    for(size_t iter = 0; iter < GK_N_ITER; iter++) {
      // Compute Gaussian kernel row
      sum_P = 0.0;
      double entropy = 0.0;
      for(size_t c = 0; c < ncol; c++) {
        size_t ind = r * ncol + c;
        if (mvalp[ind] != 0.0) {
          retp[ind] = exp(-beta * mvalp[ind] - row_max_valp[r]);
        }
        sum_P += retp[ind];
        entropy += beta * (mvalp[ind] * retp[ind]);
      }
      if (sum_P != 0.0) {
	    entropy = (entropy / sum_P) + log(sum_P);
      }
      // Evaluate whether the entropy is within the tolerance level
      double entropy_diff = entropy - log(perplexity);
      if(entropy_diff < TOLERANCE && -entropy_diff < TOLERANCE) {
        break;
      }
      else {
        if(entropy_diff > 0) {
          min_beta = beta;
          if(max_beta == DBL_MAX || max_beta == -DBL_MAX)
            beta *= 2.0;
          else
            beta = (beta + max_beta) / 2.0;
        }
        else {
          max_beta = beta;
          if(min_beta == -DBL_MAX || min_beta == DBL_MAX)
            beta /= 2.0;
          else
            beta = (beta + min_beta) / 2.0;
        }
      }
    }
  }
  // Row normalize probability
  return compute_probability_matrix(ret);
}

template <class T>
void compute_low_dimensional_affinity(rowmajor_matrix_local<T>& mat) {
  auto size = mat.val.size();
  auto m_valp = mat.val.data();
  for(size_t r = 0; r < size; r++) {
    if (m_valp[r] == 0.0)       m_valp[r] = 0.0;
    else if (m_valp[r] == -1.0) m_valp[r] = 1.0;
    else                        m_valp[r] = 1.0 / (1.0 + m_valp[r]);
  }
}

template <class T>
void scale_matrix_in_place(rowmajor_matrix_local<T>& mat,
                           double scalar) {
  scal<T>(mat.val, scalar);
}

template <class T>
T sum_of_elements(const rowmajor_matrix_local<T>& mat) {
  auto size = mat.val.size();
  auto m_valp = mat.val.data();
  T sum = 0;
  for(size_t r = 0; r < size; r++) sum += m_valp[r];
  return sum;
}

template <class T>
rowmajor_matrix_local<T> 
compute_gradient_mat(const rowmajor_matrix_local<T>& P_mat,
                     const rowmajor_matrix_local<T>& Q_mat,
                     double sum_Q) { 
  size_t nrow = P_mat.local_num_row;
  size_t ncol = P_mat.local_num_col;
  auto size = P_mat.val.size();
  rowmajor_matrix_local<T> ret(nrow, ncol);
  auto retp = ret.val.data();
  auto p_valp = P_mat.val.data();
  auto q_valp = Q_mat.val.data();
  for(size_t r = 0; r < size; r++) {
    retp[r] = (p_valp[r] - (q_valp[r] * sum_Q)) * q_valp[r];
  }
  return ret;
}

template <class T>
void performs_DminusA_inplace(rowmajor_matrix_local<T>& mat,
                              size_t myst) {
  size_t nrow = mat.local_num_row;
  size_t ncol = mat.local_num_col;
  auto size = mat.val.size();
  auto mptr = mat.val.data();
  auto D = sum_of_cols(mat);
  auto dptr = D.data();
  // perfoms mat = D - mat on distributed local matrix mat
  for(size_t r = 0; r < size; ++r) {
    mptr[r] *= -1.0;
  }
  for(size_t r = 0; r < nrow; ++r) {
    size_t k = r + myst;
    mptr[r * ncol + k] += dptr[r];
  }
}

template <class T>
void update_Y_mat(rowmajor_matrix_local<T>& y_mat,
                  rowmajor_matrix_local<T>& uY_mat,
                  rowmajor_matrix_local<T>& gains_mat,
                  const rowmajor_matrix_local<T>& grad_mat,    
                  double momentum,
                  double learning_rate) {
  auto size = y_mat.val.size();
  auto gain_valp = gains_mat.val.data();
  auto uY_valp = uY_mat.val.data();
  auto grad_valp = grad_mat.val.data();
  auto y_valp = y_mat.val.data();

  for(size_t r = 0; r < size; r++) {
    // ** update_gains **
    auto tmp = uY_valp[r] * grad_valp[r];
    if (tmp < 0.0) gain_valp[r] += PGAIN;
    else           gain_valp[r] *= NGAIN;
    if (gain_valp[r] < MINGAIN) gain_valp[r] = MINGAIN; // clipping gain 

    // ** update uY **
    uY_valp[r] = (momentum * uY_valp[r]) - 
                 (learning_rate * gain_valp[r] * grad_valp[r]);

    // ** update Y **
    y_valp[r] += uY_valp[r];

    // ** Normalize Y values to handle double precision error
    y_valp[r] = (trunc(y_valp[r] * DESIRED_PRECISION)) / DESIRED_PRECISION;
  }
}

template <class T>
rowmajor_matrix_local<T> 
compute_error(const rowmajor_matrix_local<T>& P_mat, 
              const rowmajor_matrix_local<T>& Q_mat,
              double sum_Q) {
  size_t nrow = P_mat.local_num_row;
  size_t ncol = P_mat.local_num_col;
  auto size = P_mat.val.size();
  auto p_valp = P_mat.val.data();
  auto q_valp = Q_mat.val.data();
  rowmajor_matrix_local<T> ret(nrow, ncol);
  auto retp = ret.val.data();
  for(size_t r = 0; r < size; r++) {
    if (p_valp[r] != 0) 
      retp[r] = p_valp[r] * (log(p_valp[r] / (q_valp[r] * sum_Q)));
    else
      retp[r] = 0;
  }
  return ret;
}

#if defined(_SX) || defined(__ve__)
// --- asl wrapper for random generator with normal distribution ---
template <class T>
std::vector<T>
gen_random(size_t N, double mean, double stddev);

template <>
std::vector<double>
gen_random<double>(size_t N, double mean, double stddev) {
  asl_random_t rng;
  asl_library_initialize();

  asl_random_create(&rng, ASL_RANDOMMETHOD_MT19937_64);
  asl_random_distribute_normal(rng, mean, stddev);

  // Value Generation
  std::vector<double> y(N);
  asl_random_generate_d(rng, N, y.data());

  asl_random_destroy(rng);
  asl_library_finalize();
  return y;
}

template <>
std::vector<float>
gen_random<float>(size_t N, double mean, double stddev) {
  asl_random_t rng;
  asl_library_initialize();

  asl_random_create(&rng, ASL_RANDOMMETHOD_MT19937_64);
  asl_random_distribute_normal(rng, mean, stddev);

  // Value Generation
  std::vector<float> y(N);
  asl_random_generate_s(rng, N, y.data());

  asl_random_destroy(rng);
  asl_library_finalize();
  return y;
}
#else
template <class T>
std::vector<T>
gen_random(size_t N, double mean, double stddev) {
  std::vector<T> y(N);
  auto yp = y.data();
  std::default_random_engine generator;
  std::normal_distribution<T> distribution(mean, stddev);
  for(size_t i = 0; i < N; ++i) {
    yp[i] = distribution(generator);
  }
  return y;
}
#endif

template <class T>
rowmajor_matrix<T>
compute_conditional_probability(rowmajor_matrix<T>& mat, 
                                double perplexity) {
  // ** Compute Euclidean distances
  auto gdata = get_global_data(mat);
  auto affinity = construct_distance_matrix(gdata, true, true);
  // ** Get max value of each row
  auto row_max = affinity.data.map(get_row_max<T>);
  // ** Compute Gaussian Kernel for each row
  rowmajor_matrix<T> probability_mat(affinity.data.map(
                                     compute_gaussian_kernel<T>, 
                                     row_max, 
                                     broadcast(perplexity)));
  probability_mat.num_row = affinity.num_row;
  probability_mat.num_col = affinity.num_col;

  // ** Symmetrizing Probability Matrix
  auto inv_row_cnt = 1.0 / (2 * probability_mat.num_row);
  probability_mat.data.mapv(scale_matrix_in_place<T>, 
                            broadcast(2 * inv_row_cnt));
  return probability_mat;
}

template <class T>
rowmajor_matrix<T>
initialize_y(size_t nrow, size_t ncol) {
  auto y = gen_random<T>(nrow * ncol, MEAN, VAR);
  rowmajor_matrix_local<T> y_mat_local;
  y_mat_local.val.swap(y);
  y_mat_local.set_local_num(nrow, ncol);
  // TODO: For VE, Use asl_random_parallel_create() to generate random 
  //       number in distributed way to avoid this scatter
  auto Y_mat = make_rowmajor_matrix_scatter(y_mat_local);
  return Y_mat;
}

template <class T>
rowmajor_matrix_local<T>
init_local_matrix(size_t nrow, size_t ncol,
                  T init_val) { 
  std::vector<T> retval(nrow * ncol, init_val);
  rowmajor_matrix_local<T> ret;
  ret.val.swap(retval);
  ret.set_local_num(nrow, ncol);
  return ret;
}

template <class T>
rowmajor_matrix<T>
init_matrix(const std::vector<size_t>& nrows,
            size_t num_row, size_t num_col,
            T init_val) {
  auto l_nrow = make_node_local_scatter(nrows);
  auto b_ncol = broadcast(num_col);
  rowmajor_matrix<T> ret(l_nrow.map(init_local_matrix<T>, 
                         b_ncol, broadcast(init_val)));
  ret.num_row = num_row;
  ret.num_col = num_col;
  return ret;
}

template <class T>
rowmajor_matrix<T>
compute_low_dimensional_probabilities(rowmajor_matrix<T>& Y_mat) {
  auto y_local_data = get_global_data(Y_mat);
  auto Q_mat = construct_distance_matrix(y_local_data, true, true);
  Q_mat.data.mapv(compute_low_dimensional_affinity<T>);
  return Q_mat;
}

template <class T>
rowmajor_matrix<T>
compute_gradient(rowmajor_matrix<T>& P_mat, 
                 const rowmajor_matrix<T>& Q_mat,
                 rowmajor_matrix<T>& Y_mat, // TODO: why not const?
                 double inv_sumq) {
  rowmajor_matrix<T> grad_mat(P_mat.data.map(compute_gradient_mat<T>, 
                                             Q_mat.data, 
                                             broadcast(inv_sumq)));
  grad_mat.num_row = P_mat.num_row;
  grad_mat.num_col = P_mat.num_col;
  auto myst = get_start_indices(grad_mat);
  grad_mat.data.mapv(performs_DminusA_inplace<T>, myst);
  // TODO: improve operator* here...
  auto grad_local = get_global_data(grad_mat) * get_global_data(Y_mat);
  grad_mat = make_rowmajor_matrix_scatter(grad_local);
  grad_mat.data.mapv(scale_matrix_in_place<T>, broadcast(GRAD_NORM));
  return grad_mat;
}

template <class T>
T compute_grad_error(rowmajor_matrix<T>& P_mat,
                     const rowmajor_matrix<T>& Q_mat,
                     double inv_sumq) {
  rowmajor_matrix<T> error_mat(P_mat.data.map(compute_error<T>, 
                               Q_mat.data,
                               broadcast(inv_sumq)));
  return error_mat.data.map(sum_of_elements<T>).reduce(sum<T>);
}

template <class T>
T calc_squared_sum_of_elements(const rowmajor_matrix_local<T>& mat) {
  auto N = mat.val.size();
  if (N == 0) return static_cast<T>(0);
  auto mptr = mat.val.data();
  // overflow handling
  auto maxval = std::abs(mptr[0]);
  T zero = static_cast<T>(0);
  for(size_t i = 0; i < N; ++i) {
    auto absval = mptr[i] * ((mptr[i] >= zero) - (mptr[i] < zero));
    if (absval > maxval) maxval = absval;
  }
  auto one_by_max = static_cast<T>(1.0) / maxval;
  T sqsum = 0.0;
  for(size_t i = 0; i < N; ++i) {
    auto tmp = mptr[i] * one_by_max; // dividing with max to avoid overflow!
    sqsum += tmp * tmp;
  }
  return sqsum * maxval * maxval;
}

template <class T>
double calc_matrix_norm(rowmajor_matrix<T>& mat) {
  auto sqsm = mat.data.map(calc_squared_sum_of_elements<T>)
                      .reduce(sum<T>);
  return std::sqrt(sqsm);
}

template <class T>
rowmajor_matrix<T>
tsne(rowmajor_matrix<T>& mat, 
     double perplexity = 5.0,
     double early_exaggeration = 12.0,
     double min_grad_norm = 1e-7,
     double learning_rate = 200.0,
     size_t desired_dimensions = 2,
     size_t n_iter = 1000,
     size_t n_iter_without_progress = 300,
     bool verbose = false) {
    // Initialize variables to check convergence progress
    size_t best_iter = 0;
    size_t n_iter_progress_threshold = EXPLORATION_N_ITER;
    double error = DBL_MAX;
    double best_error = DBL_MAX;
    bool check_convergence = false;
    auto b_learning_rate = broadcast(learning_rate);
    double momentum = INITIAL_MOMENTUM;
    auto b_momentum = broadcast(momentum);

    time_spent t_calP(DEBUG), t_init(DEBUG), t_comp(DEBUG);
    time_spent t_qmat(DEBUG), t_gradmat(DEBUG), t_updateY(DEBUG); 
    t_calP.lap_start();
    auto probability_mat = compute_conditional_probability<T>(mat, perplexity);
    t_calP.lap_stop();
    t_calP.show_lap("Conditional probability matrix computation time: ");

    size_t n_row = probability_mat.num_row;
    // Perform initial optimization with early exaggeration 
    probability_mat.data.mapv(scale_matrix_in_place<T>, 
                              broadcast(early_exaggeration));

    // Initialize necessary matrices
    t_init.lap_start();
    auto Y_mat = initialize_y<T>(n_row, desired_dimensions);
    auto update_Y = init_matrix<T>(mat.get_local_num_rows(),
                                   n_row, desired_dimensions, (T) 0.0);
    auto gains = init_matrix<T>(mat.get_local_num_rows(),
                                n_row, desired_dimensions, (T) 1.0);
    t_init.lap_stop();
    t_init.show_lap("Matrix initialization time: ");

    //  Perform Gradient Descent with kullback-divergence cost function
    time_spent trace_iter(TRACE);
    t_comp.lap_start();
    for(size_t i = 0; i < n_iter; i++) {
      check_convergence = (((i + 1) % N_ITER_CHECK) == 0) or (i == n_iter - 1);

      t_qmat.lap_start();
      // Compute Q matrix
      auto Q_mat = compute_low_dimensional_probabilities<T>(Y_mat);
      auto sum_Q = Q_mat.data.map(sum_of_elements<T>).reduce(sum<T>);
      t_qmat.lap_stop();

      t_gradmat.lap_start();
      // Compute Gradient
      double inv_sumq = 1.0;
      if (sum_Q != 0.0) inv_sumq = 1.0 / sum_Q;
      auto grad_mat = compute_gradient<T>(probability_mat, Q_mat, 
                                          Y_mat, inv_sumq);
      t_gradmat.lap_stop();

      t_updateY.lap_start();
      // Update Y values
      Y_mat.data.mapv(update_Y_mat<T>, update_Y.data, 
                      gains.data, grad_mat.data,
                      b_momentum, b_learning_rate);
      t_updateY.lap_stop();

      // perform final optimization with momentum at 0.8 and 
      // disable early_exaggeration
      if (i == EXPLORATION_N_ITER) {
        b_momentum = broadcast(FINAL_MOMENTUM);
        n_iter_progress_threshold = n_iter_without_progress;
        // ** Disable early exaggeration
        if (early_exaggeration != 0.0)
          probability_mat.data.mapv(scale_matrix_in_place<T>, 
                                    broadcast(1.0 / early_exaggeration));
      }

      if (check_convergence){
        error = compute_grad_error<T>(probability_mat, Q_mat, inv_sumq);
        if (error < best_error) {
            best_error = error;
            best_iter = i;
        }
        else if ((i - best_iter) > n_iter_progress_threshold) {
          if (verbose) {
            RLOG(INFO) << "[t-SNE] epoch: " << std::to_string(i + 1)
                       << ":: did not make any progress during the last "
                       << std::to_string(n_iter_progress_threshold)
                       << " episodes. Finished!\n";
          }
          break;
        }

        auto grad_norm = calc_matrix_norm<T>(grad_mat);
        if (grad_norm <= min_grad_norm) {
          if (verbose) {
            RLOG(INFO) << "[t-SNE] epoch: " << std::to_string(i + 1)
                       << ":: gradient norm: "
                       << std::to_string(grad_norm)
                       << ". Finished!" << std::endl;
          }
          break;
        }
      }
      trace_iter.show("one iter: ");
    }
    t_comp.lap_stop();
    t_comp.show_lap("tsne computation time: ");
    t_qmat.show_lap("  -> Qmat calculation time: ");
    t_gradmat.show_lap("  -> Gradient calculation time: ");
    t_updateY.show_lap("  -> Y update time: ");
    return Y_mat;
}

}
#endif
