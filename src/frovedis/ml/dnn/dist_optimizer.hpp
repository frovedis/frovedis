#ifndef DIST_OPTIMIZER_HPP
#define DIST_OPTIMIZER_HPP

#include <math.h>

namespace frovedis {

struct dist_adagrad : public tiny_dnn::stateful_optimizer<1> {

  dist_adagrad() : alpha(float_t(0.01)), eps(float_t(1e-8)) {}

  void update(const tiny_dnn::vec_t &dW, tiny_dnn::vec_t &W,
              bool parallelize) {
    tiny_dnn::vec_t &g = get<0>(W);
    size_t size = dW.size();
    tiny_dnn::vec_t red_dW(size);
    tiny_dnn::vec_t& org_dW = const_cast<tiny_dnn::vec_t&>(dW);
    if(sizeof(tiny_dnn::float_t) == 4) {
      MPI_Allreduce(&org_dW[0], &red_dW[0], size,
                    MPI_FLOAT, MPI_SUM, MPI_COMM_WORLD);    
    } else {
      MPI_Allreduce(&org_dW[0], &red_dW[0], size,
                    MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD);    
    }
    float_t* gp = g.data();
    float_t* red_dWp = red_dW.data();
    float_t* Wp = W.data();
    if(sizeof(tiny_dnn::float_t) == 4) {
      for(size_t i = 0; i < size; i++) {
        gp[i] += red_dWp[i] * red_dWp[i];
        Wp[i] -= alpha * red_dWp[i] / (sqrtf(gp[i]) + eps);
      }
    }
    else {
      for(size_t i = 0; i < size; i++) {
        gp[i] += red_dWp[i] * red_dWp[i];
        Wp[i] -= alpha * red_dWp[i] / (sqrt(gp[i]) + eps);
      }
    }
  }

  tiny_dnn::float_t alpha;  // learning rate
 private:
  tiny_dnn::float_t eps;

  SERIALIZE(alpha, eps)
};

struct dist_RMSprop : public tiny_dnn::stateful_optimizer<1> {
  dist_RMSprop() : alpha(float_t(0.0001)), mu(float_t(0.99)), eps(float_t(1e-8)) {}

  void update(const tiny_dnn::vec_t &dW, tiny_dnn::vec_t &W, bool parallelize) {
    tiny_dnn::vec_t &g = get<0>(W);
    size_t size = dW.size();
    tiny_dnn::vec_t red_dW(size);
    tiny_dnn::vec_t& org_dW = const_cast<tiny_dnn::vec_t&>(dW);
    if(sizeof(tiny_dnn::float_t) == 4) {
      MPI_Allreduce(&org_dW[0], &red_dW[0], size,
                    MPI_FLOAT, MPI_SUM, MPI_COMM_WORLD);
    } else {
      MPI_Allreduce(&org_dW[0], &red_dW[0], size,
                    MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD);
    }
    float_t* gp = g.data();
    float_t* red_dWp = red_dW.data();
    float_t* Wp = W.data();

    if(sizeof(tiny_dnn::float_t) == 4) {
#pragma _NEC ivdep
      for(size_t i = 0; i < size; i++) {
        gp[i] = mu * gp[i] + (1 - mu) * red_dWp[i] * red_dWp[i];
        Wp[i] -= alpha * red_dWp[i] / sqrtf(gp[i] + eps);
      }
    }
    else {
#pragma _NEC ivdep
      for(size_t i = 0; i < size; i++) {
        gp[i] = mu * gp[i] + (1 - mu) * red_dWp[i] * red_dWp[i];
        Wp[i] -= alpha * red_dWp[i] / sqrt(gp[i] + eps);
      }
    }
  }

  tiny_dnn::float_t alpha;  // learning rate
  tiny_dnn::float_t mu;     // decay term
 private:
  tiny_dnn::float_t eps;  // constant value to avoid zero-division
  
  SERIALIZE(alpha, mu, eps)
};

}

#endif
