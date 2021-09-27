/*
    Copyright (c) 2013, Taiga Nomi and the respective contributors
    All rights reserved.

    Use of this source code is governed by a BSD-style license that can be found
    in the LICENSE file.
*/
#pragma once

#include <algorithm>
#include <numeric>

#include "tiny_dnn/util/util.h"

namespace tiny_dnn {

// x = x / denom
inline void vector_div(vec_t &x, float_t denom) {
  std::transform(x.begin(), x.end(), x.begin(),
                 [=](float_t x) { return x / denom; });
}

namespace detail {

inline void moments_impl_calc_mean(size_t num_examples,
                                   size_t channels,
                                   size_t spatial_dim,
                                   const tensor_t &in,
                                   vec_t &mean) {
  vec_t local_mean = mean;
  size_t mean_size = mean.size();
  
  float_t local_div = num_examples * spatial_dim;
  float_t red_div;
 
  for (size_t i = 0; i < num_examples; i++) {
    const auto in_data = in[i].data();
    for (size_t j = 0; j < channels; j++) {
      float_t &rmean = local_mean.at(j);
      for(size_t k = 0; k < spatial_dim; ++k){
        rmean += in_data[j * spatial_dim + k]; 
      }
    }
  }
  if(sizeof(tiny_dnn::float_t) == 4) {
      MPI_Allreduce(&local_mean[0], &mean[0], mean_size,
                    MPI_FLOAT, MPI_SUM, frovedis::frovedis_comm_rpc);    
      MPI_Allreduce(&local_div, &red_div, 1,
                    MPI_FLOAT, MPI_SUM, frovedis::frovedis_comm_rpc);                          
  } else {
      MPI_Allreduce(&local_mean[0], &mean[0], mean_size,
                    MPI_DOUBLE, MPI_SUM, frovedis::frovedis_comm_rpc);    
      MPI_Allreduce(&local_div, &red_div, 1,
                    MPI_DOUBLE, MPI_SUM, frovedis::frovedis_comm_rpc);    
  }
  vector_div(mean, red_div);
}

inline void moments_impl_calc_variance(size_t num_examples,
                                       size_t channels,
                                       size_t spatial_dim,
                                       const tensor_t &in,
                                       const vec_t &mean,
                                       vec_t &variance) {
  vec_t local_variance (variance);
  size_t variance_size = variance.size();

  float_t local_div = std::max(float_t{1.0f}, static_cast<float_t>(num_examples * spatial_dim) - float_t{1.0f});
  float_t red_div;
  
  assert(mean.size() >= channels);
  for (size_t i = 0; i < num_examples; i++) {
    for (size_t j = 0; j < channels; j++) {
      float_t &rvar    = local_variance[j];
      const auto it    = in[i].begin() + (j * spatial_dim);
      const float_t ex = mean[j];
      rvar             = std::accumulate(it, it + spatial_dim, rvar,
                             [ex](float_t current, float_t x) {
                               return current + pow(x - ex, float_t{2.0});
                             });
    }
  }
 if(sizeof(tiny_dnn::float_t) == 4) {
      MPI_Allreduce(&local_variance[0], &variance[0], variance_size,
                    MPI_FLOAT, MPI_SUM, frovedis::frovedis_comm_rpc);    
      MPI_Allreduce(&local_div, &red_div, 1,
                    MPI_FLOAT, MPI_SUM, frovedis::frovedis_comm_rpc);    
  } else {
      MPI_Allreduce(&local_variance[0], &variance[0], variance_size,
                    MPI_DOUBLE, MPI_SUM, frovedis::frovedis_comm_rpc);    
      MPI_Allreduce(&local_div, &red_div, 1,
                    MPI_DOUBLE, MPI_SUM, frovedis::frovedis_comm_rpc);    
  }
  vector_div(variance, red_div);
}

}  // namespace detail

/**
 * calculate mean/variance across channels
 */
inline void moments(const tensor_t &in,
                    size_t spatial_dim,
                    size_t channels,
                    vec_t &mean) {
  const size_t num_examples = in.size();
  assert(in[0].size() == spatial_dim * channels);

  mean.resize(channels);
  vectorize::fill(&mean[0], mean.size(), float_t{0.0});
  detail::moments_impl_calc_mean(num_examples, channels, spatial_dim, in, mean);
}

inline void moments(const tensor_t &in,
                    size_t spatial_dim,
                    size_t channels,
                    vec_t &mean,
                    vec_t &variance) {
  const size_t num_examples = in.size();
  assert(in[0].size() == spatial_dim * channels);

  // calc mean
  moments(in, spatial_dim, channels, mean);

  variance.resize(channels);
  vectorize::fill(&variance[0], variance.size(), float_t{0.0});
  detail::moments_impl_calc_variance(num_examples, channels, spatial_dim, in,
                                     mean, variance);
}

}  // namespace tiny_dnn
