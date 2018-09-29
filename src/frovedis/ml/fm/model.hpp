#ifndef _FM_MODEL_HPP_
#define _FM_MODEL_HPP_

#include <frovedis/matrix/rowmajor_matrix.hpp>

namespace frovedis {
namespace fm {
  
template <class T>
struct fm_config {
  fm_config() {}
  fm_config(bool dim_w0, bool dim_w, size_t dim_v,
            T init_stdev, size_t iteration, T init_learn_rate,
            T regular_w0, T regular_w, T regular_v, bool is_regression, size_t batch_size_pernode):
              init_stdev(init_stdev), iteration(iteration), 
              init_learn_rate(init_learn_rate), learn_rate(init_learn_rate), 
              regular_w0(regular_w0), regular_w(regular_w), regular_v(regular_v), is_regression(is_regression),
              use_w0(dim_w0), use_w(dim_w), use_v(dim_v > 0), 
              factor_size(dim_v), batch_size_pernode(batch_size_pernode) {
                bool assumption = init_stdev >= 0 && iteration > 0 && init_learn_rate > 0 && \
                                  regular_w0 >= 0 && regular_w >= 0 && regular_v >= 0 && batch_size_pernode > 0;
                if (!assumption) REPORT_ERROR(USER_ERROR, "Input config do not have correct value");
              }
    
  T init_stdev;
  size_t iteration;
  T init_learn_rate;
  T learn_rate;
  T regular_w0;
  T regular_w;
  T regular_v;
  bool is_regression;
    
  bool use_w0;
  bool use_w;
  bool use_v;
  size_t factor_size;
  size_t feature_size;  // not assigned at initialization
  size_t batch_size_pernode;
  
  SERIALIZE(init_stdev, iteration, init_learn_rate, learn_rate,
            regular_w0, regular_w, regular_v, is_regression, 
            use_w0, use_w, use_v, factor_size, feature_size, batch_size_pernode);
};


template <class T>
struct fm_parameter {
  fm_parameter() {}
  fm_parameter(size_t feature, size_t factor):
    w0(), w(feature, 0), v(feature, factor) {}
  fm_parameter(T _w0, std::vector<T>& _w, rowmajor_matrix_local<T>& _v):
    w0(_w0), w(_w), v(_v) { assert(w.size() == v.local_num_row); }   
  fm_parameter(T _w0, std::vector<T>&& _w, rowmajor_matrix_local<T>&& _v): 
    w0(_w0), w(std::move(_w)), v(std::move(_v)) { 
      assert(w.size() == v.local_num_row); 
    }   
    
  size_t feature_size() {
    return v.local_num_row;
  }
  size_t factor_size() {
    return v.local_num_col;
  }
  
  T w0;
  std::vector<T> w;
  rowmajor_matrix_local<T> v;   
  
  SERIALIZE(w0, w, v);
};


template <class T>
node_local<fm_parameter<T>> 
make_local_fm_parameter(std::vector<size_t> feature_sizes, 
                        size_t factor_size) {
  size_t nodesize = get_nodesize();
  assert(nodesize == feature_sizes.size());
  
  auto nl_feature_size = make_node_local_scatter(feature_sizes);
  auto nl_factor_size = broadcast(factor_size);
  
  auto func = +[](size_t feat, size_t fac){
    return fm_parameter<T>(feat, fac);
  };
  return nl_feature_size.map(func, nl_factor_size);
}


template <class T>
fm_parameter<T> concatenate_fm_parameter(node_local<fm_parameter<T>>& nl_parameters) {
  auto parameters = nl_parameters.gather();
  size_t nodesize = get_nodesize();

  size_t feature_size = 0;
  for (size_t i_node = 0; i_node < nodesize; i_node++) {
    feature_size += parameters[i_node].feature_size();
  }
  size_t factor_size = parameters[0].factor_size();
  
  fm_parameter<T> concat_parameter(feature_size, factor_size);

  // w0
  concat_parameter.w0 = parameters[0].w0;
  
  auto* ptr_concat_w = concat_parameter.w.data();
  auto* ptr_concat_v = concat_parameter.v.val.data();  
  size_t feature_offset = 0;
  for (size_t i_node = 0; i_node < nodesize; i_node++) {
    size_t node_feature_size = parameters[i_node].feature_size();
    // w
    auto& w = parameters[i_node].w;
    auto* ptr_w = w.data();
    for (size_t i_feat = 0; i_feat < node_feature_size; i_feat++) {
      ptr_concat_w[feature_offset + i_feat] = ptr_w[i_feat];
    }
    // v
    auto& v = parameters[i_node].v;
    auto* ptr_v = v.val.data();
    for (size_t i_feat = 0; i_feat < node_feature_size; i_feat++) {
      for (size_t i_fac = 0; i_fac < factor_size; i_fac++) {
        size_t concat_pos = factor_size * (feature_offset + i_feat) + i_fac;
        size_t pos = factor_size * i_feat + i_fac;
        ptr_concat_v[concat_pos] = ptr_v[pos];
      }
    }
    feature_offset += node_feature_size;
  }
  return concat_parameter;
}


template <class T>
void init_fm_parameter_with_stdev(fm_parameter<T>& parameter, T stdev, int random_seed) {
  // initialize (ONLY) v with normal distribution
  auto* ptr_v = parameter.v.val.data();
  auto v_size = parameter.v.val.size();
  
#ifdef USE_STD_RANDOM
  std::random_device seed_gen;
  std::default_random_engine engine(random_seed);
  std::normal_distribution<> dist(0, stdev);
  for (size_t i = 0; i < v_size; i++) {
    ptr_v[i] = dist(engine);
  }
#else
  srand48(random_seed);
  for (size_t i = 0; i < v_size; i++) {
    ptr_v[i] = std::sqrt(-2.0 * std::log(drand48())) * std::cos(2.0 * M_PI * drand48());
  }
#endif  // USE_STD_RANDOM
}  
  

template <class T>
struct fm_model {
  fm_model(){}
  fm_model(fm_config<T>& c, fm_parameter<T>& p):
    config(c), parameter(p) {}
  fm_model(fm_config<T>&& c, fm_parameter<T>&& p) {
    config.swap(c);
    parameter.swap(p);
  }
  
  void save(const std::string& file) {
    std::ofstream str(file.c_str());
    cereal::BinaryOutputArchive ar(str);
    ar << *this;
  }
  // void savebinary(const std::string& file);

  template <class I = size_t, class O = size_t>
  std::vector<T> predict_value(crs_matrix_local<T,I,O>& input);
  template <class I = size_t, class O = size_t>
  std::vector<T> predict(crs_matrix_local<T,I,O>& input);
      
  fm_config<T> config;
  fm_parameter<T> parameter;
  
  SERIALIZE(config, parameter);
};


template <class T> template <class I, class O>
std::vector<T> 
fm_model<T>::predict_value(crs_matrix_local<T,I,O>& input) {
  auto& comp_parameter = parameter;
  if (input.local_num_col > parameter.feature_size()) {
    fm_parameter<T> extended = parameter;
    extended.w.resize(input.local_num_col);
    extended.v.val.resize(input.local_num_col * parameter.factor_size());
    extended.v.local_num_row = input.local_num_col;
    
    comp_parameter = extended;
  }
  
  // w0
  std::vector<T> y(input.local_num_row, parameter.w0);
  auto* ptr_y = y.data();
  // w
  auto yw = input * comp_parameter.w;
  auto* ptr_yw = yw.data();
  for (size_t ix = 0; ix < input.local_num_row; ix++) {
    ptr_y[ix] += ptr_yw[ix];
  }
  // v
  std::vector<T> ones(comp_parameter.factor_size(), 0.5);
  auto yv = ((input * comp_parameter.v).pow_val(2) - input.pow_val(2) * comp_parameter.v.pow_val(2)) * ones;
  auto* ptr_yv = yv.data();
  for (size_t ix = 0; ix < input.local_num_row; ix++) {
    ptr_y[ix] += ptr_yv[ix];
  }
  
  return y;
}


template <class T> template <class I, class O>
std::vector<T> 
fm_model<T>::predict(crs_matrix_local<T,I,O>& input) {
  if (config.is_regression) {
    return predict_value<I>(input);
  } else {
    auto proba = predict_value(input);
    auto* ptr_proba = proba.data();

    std::vector<T> y(proba.size());
    auto* ptr_y = y.data();
    size_t y_size = y.size();
    for (size_t i = 0; i < y_size ; i++) {
      if (ptr_proba[i] > 0) {
	ptr_y[i] = 1.0;
      } else {
       	ptr_y[i] = - 1.0;
      }
    }
    return y;
  }
}


template <class T>
fm_model<T> load_fm_model(const std::string& input) {
  fm_model<T> model;
  std::ifstream str(input.c_str());
  cereal::BinaryInputArchive ar(str);
  ar >> model;
  return model;
}

// template <class T>
// fm_model<T> loadbinary_fm_model(const std::string& input);


}  // namespace fm
}  // namespace frovedis

#endif  // _FM_MODEL_HPP_
