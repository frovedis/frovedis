#ifndef ARIMA_H
#define ARIMA_H
#include <cmath>
#include <frovedis.hpp>
#include <frovedis/core/vector_operations.hpp>
#include <frovedis/matrix/colmajor_matrix.hpp>
#include <frovedis/ml/glm/linear_regression.hpp>
#define TMAX std::numeric_limits<T>::max()
#define NaN std::numeric_limits<T>::quiet_NaN()

namespace frovedis {
   
template <class T>	
void diff_helper(std::vector<T>& l_vec, 
		 const std::vector<T>& l_shadow,
                 size_t& interval) {
  auto l_vecp = l_vec.data();
  auto l_shadowp = l_shadow.data();
  auto sz = l_vec.size();
  auto s_sz = l_shadow.size();
  size_t  s_count = 0;
  auto less = sz + interval - s_sz;
  if (sz && s_sz) {
    for (size_t j = 0; isnan(l_shadowp[j]) && j < sz; j++) s_count++ ;
    size_t lim = sz;
    if ((less + s_count) < sz) lim = less + s_count;
    for (size_t j = 0; j < lim; j++)  l_vecp[j] = NaN; 
    for (size_t j = s_count + less; j < sz; j++)
        l_vecp[j] = l_vecp[j] - l_shadowp[j - less];
  }
}

template<class T>
std::vector <T> 
dropNaN_helper (const std::vector<T>& diffnum, size_t NaNs){
  size_t diffnum_size = diffnum.size();
  std::vector<T> ret;
  auto diffnum_data = diffnum.data(); 
  if (diffnum_size > NaNs) {
    if(!get_selfid()) {
      ret.resize(diffnum_size - NaNs);
      auto ret_data = ret.data();
      for (size_t i = NaNs; i < diffnum_size; ++i) 
	ret_data[i - NaNs] = diffnum_data[i];
    }
    else ret = diffnum;
  }
  else {
    size_t count = 0;
    for (size_t i = 0; i < diffnum_size ; ++i) {
      if (isnan(diffnum_data[i])) {
        count++;
      }
      else break;
    }
    if (count < diffnum_size) {
      ret.resize(diffnum_size - count);
      auto ret_data = ret.data();
      for (size_t i = 0; i < diffnum_size - count; ++i) 
        ret_data[i] = diffnum_data[i + count];
    }
  }
  return ret;
}


template <class T>
std::vector<T>
shift2_helper(const std::vector<T>& l_vec, size_t& interval) {
  auto l_vecp = l_vec.data();
  auto sz = l_vec.size();
  std::vector<T> ret;
  if (!get_selfid()){
    ret.resize(sz);
    auto retp = ret.data();
    for (size_t i = 0; i < interval; ++i)
      retp[i] = NaN;
    for (size_t i = interval; i < sz; ++i)
      retp[i] = l_vecp[i - interval];
  }
  else {
    if(sz >= interval){
      ret.resize(sz - interval);
      auto retp = ret.data();
      for (size_t i = 0; i < sz - interval; ++i)
        retp[i] = l_vecp[i];
    }
  }
  return ret;
}

template <class T>
std::vector<T>
shiftex_helper(const std::vector<T>& l_vec, size_t interval) {
  if (!get_selfid()) {
    auto tmp = vector_full<T>(interval, NaN);
    return  vector_concat(tmp, l_vec);
  }  
  return l_vec;
}
       
template <class T>
std::vector<T> 
dropall_NaN_helper(const std::vector<T>& vec){
  auto vec_size = vec.size();
  auto vec_data = vec.data();
  size_t  count = 0;
  if (vec_size) {
    for (size_t i = 0; i < vec_size; ++i) {
      if( isnan(vec_data[i])) count++;
      else break;
    }
  }
  std::vector<T> res(vec_size - count);
  auto res_data = res.data();
  for (size_t i = count; i < vec_size; ++i) {
    res_data[i - count] = vec_data[i];   
  }
  return res;
}
  

template <class T>    
std::vector<T>
do_shift(const std::vector<T>& lvec, size_t step_size) {
  if (get_selfid() == 0) {
    auto tmp = vector_full<T>(step_size, NaN);
    return vector_concat(tmp, lvec);
  } 
  return lvec;
}

template <class T>    
std::vector<T>
calc_diff_helper(std::vector<T>& lv1,
                 const std::vector<T>& lv2, 
                 const std::vector<size_t>& step,
                 const size_t& stepsize) {
  if (!step[0]) return lv1 ;
  if (step[0] == stepsize + 1) return lv1 + lv2 ;
  auto lv1_size = lv1.size();
  auto lv1_data = lv1.data();
  auto lv2_data = lv2.data();
  for (size_t i = step[0]; i < lv1_size; ++i) 
    lv1_data[i] += lv2_data[i];
    return lv1;
}

template <class T> 
std::vector<T>
appender_helper(const std::vector<T>& vec2,const std::vector<T>& vec1){
  return vector_concat(vec1, vec2);
}

template <class T>   
std::vector<T>
crop_helper(const std::vector<T>& vec, size_t& start_node, 
	    size_t& start_pos, size_t& stop_node, 
            size_t& stop_pos) {
  size_t start = 0;
  size_t end = vec.size();
  if ((get_selfid() > stop_node) || (get_selfid() < start_node)) {
    std::vector<T> res(0);
    return res; 
  }
  if (get_selfid() == start_node)
    start = start_pos;
  if (get_selfid() == stop_node)
    end = stop_pos;
    
  auto new_vec_size = vec.size() - start - (vec.size() - end);	
  std::vector<T>  vec_res(new_vec_size); 
  auto vec_res_data = vec_res.data();
  auto vec_data = vec.data();
  for (size_t i = start; i < end; ++i){ 
    vec_res_data[i-start] = vec_data[i];
  }
  return vec_res ;
}

template <class T>
std::vector<T>
t_crop_helper(const std::vector<T>& vec, 
              size_t& start_node, 
              size_t& start_pos) {
  size_t start = 0;
  size_t end = vec.size();
  if (get_selfid() < start_node) {
    std::vector<T> res(0);
    return res; 
  }
  if (get_selfid() == start_node)
    start = start_pos;
  auto new_vec_size = end - start;	
  std::vector<T>  vec_res(new_vec_size); 
  auto vec_res_data = vec_res.data();
  auto vec_data = vec.data();
  for (size_t i = start; i < end; ++i){ 
    vec_res_data[i - start] = vec_data[i];
  }
  return vec_res ;
}  

    
template <class T>    
rowmajor_matrix_local<T>
make_rmml_helper(const std::vector<T>& vec, size_t lag) {
  auto res = rowmajor_matrix_local<T>(vec);
  res.local_num_row = lag;
  res.local_num_col = vec.size() / lag;
  res = res.transpose();
  return res;  
}

    
template <class T>
std::vector<T> 
solver_helper(const frovedis::rowmajor_matrix_local<T>& d,
              const std::vector<T>& coeff,
              const  T& intercept){
  if(d.local_num_col != coeff.size())
    REPORT_ERROR(USER_ERROR, "COEFFICIENT SIZE AND DATA MATRIX SIZE MISMATCH IN SOLVER\n");
  
  std::vector<T> predicted_values_train = d * coeff;
  predicted_values_train = vector_add(predicted_values_train, intercept);
  return predicted_values_train;
}

     
template <class T>    
T calc_s(const T& num1, const T& num2) {
  return num1 + num2;
}

    
template <class T>    
T calc_var (const std::vector<T>& v1,const std::vector<T>& v2) {
  if (v1.size() != v2.size())
    REPORT_ERROR(USER_ERROR, "RMSE calculation failed . Size of input vectors doesn't match!\n");
  T sum = 0.0;
  T dif = 0.0;
  for (size_t i = 0; i < v1.size(); ++i) {
    dif = v1[i] - v2[i];
    sum += (dif * dif);
  }
  return sum;
}


template<class T>
std::vector<T> 
calc_subtract (const std::vector<T>& v1, const std::vector<T>& v2) {
  return vector_subtract(v1, v2);
}


template <class T>
std::vector<T>
create_lag_helper(const std::vector<T>& vec, size_t node_num, size_t lag) {
  if (get_selfid() == node_num){
    std::vector<T> res(lag);
    auto vec_size = vec.size();
    auto vec_data = vec.data();
    for (size_t i = 0; i < lag; ++i)
      res[i] = vec_data[vec_size - 1 - i];
    return res;
  }
  return {};
}

template <class T>
std::vector<T>
create_nondistdifflag_helper(const std::vector<T>& vec,
			     size_t node_num, 
			     size_t pos, 
			     size_t lag) {
  if (get_selfid() == node_num){
    std::vector<T> res(lag);
    auto vec_data = vec.data();
    for (size_t i = 0; i < lag; ++i)
      res[i] = vec_data[pos - lag + i];
    return res;
  }
  return {};
}

 
template <class T>
T
predict_solver_helper(const std::vector<T>& vec,
                      const std::vector<T>& coeff,
                      const T& intercept) {
  if (vec.size())
    return vector_dot(vec, coeff) + intercept;
  return 0.0;
}


template <class T>
std::vector<T>
push_back_helper(const std::vector<T>& vec,T data,size_t stop_node){
  if (get_selfid() == stop_node){
    return vector_concat(vec, {data});
  }
  return vec;
} 


template <class T>
std::vector<T>
front_zero_helper(const std::vector<T>& vec,size_t lag){
  auto tmp = vector_full<T>(lag, 0);
  if (!get_selfid()){
    return vector_concat(tmp,vec);
  }
  return vec;
}


template <class T>
std::vector<T>
create_distdifflag_helper(const std::vector<T>& vec, 
			  size_t start_node,
			  size_t start_pos,
                          size_t stop_node,
			  size_t stop_pos ) {
  size_t start = 0, stop = 0, vec_size = vec.size(); 
  auto selfid = get_selfid();
  if (selfid > start_node && selfid < stop_node){
	  stop = vec_size;
  }
  if (selfid == start_node){
	  start = start_pos;
	  stop = vec_size;
  }
  if (selfid == stop_node){
	  stop = stop_pos;
  }
  std::vector<T> res(stop - start);
  auto vec_data = vec.data();
  for (size_t i = start; i < stop ; ++i)
    res[i - start]= vec_data[i];
  return res;
}

  
template <class T>
class Arima{
  public:
    size_t ar_lag, ma_lag, diff_order, seasonal, sample_size = 0;
    T best_ar_intercept, best_ma_intercept;
    node_local<std::vector<T>>  fitted_values, diff_data, 
                                ma_predict_data, sample_data;
    std::vector<T> best_ar_coeff, best_ma_coeff;
    bool regression_lapack , debug, auto_arima, is_fitted;
    std::string solverx; 
   
 
    void validate_params(size_t sz) {
      if (int(ar_lag) < 1)
        REPORT_ERROR(USER_ERROR, 
                    "AR(p) order cannot be less than 1!\n");
      if (int(ma_lag) < 0)
        REPORT_ERROR(USER_ERROR, 
                    "MA(q) order cannot be negative!\n");
      if (int(diff_order) < 0)
        REPORT_ERROR(USER_ERROR, 
                    "I(d) order cannot be negative!\n");
      if (int(seasonal) < 0)
        REPORT_ERROR(USER_ERROR, 
                    "seasonal cannot be negative!\n");
      if (sample_size < ma_lag + ar_lag + diff_order + seasonal + 2)
        REPORT_ERROR(USER_ERROR, 
                   "Number of samples in input is too less for time series analysis!\n");

      if ((sample_size / sz) <= ar_lag + ma_lag + seasonal + diff_order){ 
        auto vec_size = vector_full<size_t>(sz, 0);
        vec_size[0] = sample_size;
        sample_data = sample_data.template moveto_dvector<T>()
			         .align_as(vec_size)
				 .moveto_node_local();
        std::cout << "Sample size is too small for distribution. Re-aligning " << 
                     "data on a single worker node."<<std::endl;
      } 
    } 


    Arima(const node_local<std::vector<T>>& sample_data_l,
          size_t ar_lag_l = 1,
          size_t diff_order_l = 0,
          size_t ma_lag_l = 0,
          size_t seasonal_l = 0,
          bool auto_arima_l = false,
          std::string solver_l = "lapack"){
      sample_data = sample_data_l;
      auto vec_size = sample_data.template viewas_dvector<T>().sizes(); 
      size_t sz = vec_size.size();
      for (size_t i = 0; i < sz; ++i) sample_size += vec_size[i]; 
      ar_lag = ar_lag_l;
      diff_order = diff_order_l;
      ma_lag = ma_lag_l;
      auto_arima = auto_arima_l;
      seasonal = seasonal_l;
      is_fitted = false;
      solverx = solver_l;
      validate_params(sz);
    }    

    node_local<std::vector<T>>
    shift2(node_local<std::vector<T>>& l,
           size_t interval) {
      auto vec_szs = l.template viewas_dvector<T>().sizes();
      l = extend_lower_shadow(l, interval);
      return l.map(shift2_helper<T>, broadcast(interval)); 
    }
   

    node_local<std::vector<T>> 
    shiftex_handler(node_local<std::vector<T>>& l,
                    size_t interval) { 
      return l.map(shiftex_helper<T>, broadcast(interval)); 
    }
    
    void 
    diff_handler(node_local<std::vector<T>>& l, 
                 size_t interval) { 
      auto l_shadow = extend_lower_shadow(l, interval);
      l.mapv(diff_helper<T>, l_shadow, broadcast(interval)); 
    }
    
    void 
    diff_handlersingle(node_local<std::vector<T>>& l, 
                 size_t interval) { 
      auto l_s = l;
      l.mapv(diff_helper<T>, l_s, broadcast(interval));
    }
   
    void
    differencing(node_local<std::vector<T>>& l) {
      for (int i = 1; i <= diff_order; ++i)
        diff_handler(l, 1); 
      if (seasonal)
        diff_handler(l, seasonal);
    }

    node_local<std::vector<T>>   
    dropNaN (node_local<std::vector<T>>& diffnum) {
      return diffnum.map(dropNaN_helper<T>, 
                         broadcast(ar_lag + seasonal + diff_order));
    }
    
    node_local<std::vector<T>> 
    dropall_NaN (node_local<std::vector<T>>& diffnum) {
      return diffnum.map(dropall_NaN_helper<T>);
    }   
    
    node_local<std::vector<T>> 
    residual_adder(node_local<std::vector<T>>& dv1,
                   node_local<std::vector<T>>& dv2,
                   size_t step,
                   bool shifted) {
      auto step_size = step; 
      auto size_vec = dv2.template viewas_dvector<T>().sizes();
      auto size_len = size_vec.size();
      for (size_t i =0; i<size_len ; ++i){
        if (size_vec[i] <=  step_size){
          step_size -= size_vec[i];
          size_vec[i] = 0;
        }
        else {
          if(!step_size)
            size_vec[i] = step + 1;
          else{
            size_vec[i] = step_size;
            step_size = 0;
          }
        }
      }
      auto node_step_size = frovedis::make_dvector_scatter(size_vec)
                                       .moveto_node_local();
      if (!shifted){
        return dv1.template moveto_dvector<T>() 
                .align_as(dv2.template viewas_dvector<T>().sizes())  
                .viewas_node_local()
                .map(calc_diff_helper<T>, dv2, node_step_size, broadcast(step));
      }
      return dv1.map(calc_diff_helper<T>,
                    (dv2.map(do_shift<T>,broadcast(step))
                        .template moveto_dvector<T>()
                        .align_as(dv1.template viewas_dvector<T>().sizes())
                        .viewas_node_local()),node_step_size, broadcast(step));
    }

    
    node_local<std::vector<T>> 
    calc_diff(node_local<std::vector<T>>& dv1,
              node_local<std::vector<T>>& dv2,
              bool shifted) {
      auto step_size = seasonal + ar_lag + diff_order + ma_lag; 
      auto size_vec = dv2.template viewas_dvector<T>().sizes();
      auto size_len = size_vec.size();
      for (size_t i =0; i<size_len; ++i){
        if  (size_vec[i] <=  step_size){
          step_size -= size_vec[i];
          size_vec[i] = 0;
        }
        else {
          if(!step_size)
            size_vec[i] = seasonal+ar_lag+diff_order+ma_lag+1;
          else{
            size_vec[i] = step_size;
            step_size = 0;
          }
        }
      }
      auto node_step_size = frovedis::make_dvector_scatter(size_vec)
                                   .moveto_node_local();
      if (!shifted){
        return dv1.template moveto_dvector<T>() 
                  .align_as(dv2.template viewas_dvector<T>().sizes()) 
                  .viewas_node_local()
                  .map(calc_diff_helper<T>, 
                       dv2, node_step_size, 
                       broadcast(ma_lag + ar_lag + diff_order + seasonal));
      }
      return dv1.map(do_shift<T>, broadcast(seasonal+ar_lag+diff_order))
                .template moveto_dvector<T>()
                .align_as(dv2.template viewas_dvector<T>().sizes())  
                .viewas_node_local()
                .map(calc_diff_helper<T>, dv2, node_step_size, 
                     broadcast(ma_lag + ar_lag + diff_order + seasonal));
    }
    
    node_local<std::vector<T>>
    undifferencing (const node_local<std::vector<T>>& numbers, 
                    node_local<std::vector<T>>& diffnum) {
      auto n1 = numbers;
      if (diff_order) {
        auto nshift = n1;
        nshift = shift2(nshift, 1);
        diffnum = calc_diff(diffnum, nshift, true);
        auto hd = n1;
        node_local<std::vector<T>>  hshift;     
        for (int i = 2; i <= diff_order; ++i){
          diff_handler(hd, 1);
          hshift = hd;
          hshift = shift2(hshift, 1);
          diffnum = calc_diff (diffnum, hshift, false);
        }
        if (seasonal) {
          auto nd = n1;
          for (int i = 1; i <= diff_order; ++i)
            diff_handler(nd, 1);
          nd = shift2(nd, seasonal);
          diffnum = calc_diff(diffnum, nd, false);
        }
        diffnum =  dropNaN (diffnum);
      }
      return diffnum;
    }
       
    void
    single_undifferencing_new (node_local<std::vector<T>>& numbers,
                               T& diffnum, size_t pos) {
      auto n1 = numbers;
      if (diff_order){
        auto nshift = n1;
        nshift = shiftex_handler(nshift, 1);
        diffnum += nshift.template as_dvector<T>().get(pos);
        auto hd = n1;
        auto hshift = hd;
        for (int i = 2; i <= diff_order; ++i){
          diff_handlersingle(hd, 1);
          hshift = hd;
          hshift = shiftex_handler(hshift, 1);
          diffnum += hshift.template as_dvector<T>().get(pos);
        }
        if (seasonal){
          auto nd = n1;
          for (int i = 1; i <= diff_order; ++i)
            diff_handlersingle(nd,1);
          nd = shiftex_handler(nd, seasonal);
          diffnum += nd.template as_dvector<T>().get(pos+seasonal-1);
        }
      }
    }
    
    node_local<std::vector<T>>
    appender(node_local<std::vector<T>>& vec1, node_local<std::vector<T>> vec2){
      return vec2.map(appender_helper<T>, vec1);
    }
    
    node_local<std::vector<T>>
    crop_vec(node_local<std::vector<T>>& vec1, size_t start, size_t stop) {
      auto size_vec = vec1.template viewas_dvector<T>().sizes();
      auto size_vec_data = size_vec.data();
      auto size_vec_len = size_vec.size();
      size_t start_node = 0;
      auto stop_node =  size_vec_len;
      for (size_t i = 0; i < size_vec_len; ++i) {
        if (size_vec_data[i] < start) 
          start -= size_vec_data[i];
        else{ 
          start_node = i;
          break;
        }
      }
      for (size_t i = 0; i < size_vec_len; ++i) { 
        if (size_vec_data[i] < stop)
          stop -= size_vec_data[i];
        else{
          stop_node = i;
          break;
        }
      }
      return vec1.map(crop_helper<T>, broadcast(start_node), broadcast(start),
                      broadcast(stop_node), broadcast(stop)) ;
    }

    node_local<std::vector<T>>
    t_crop(node_local<std::vector<T>>& vec1, size_t start) {
      auto size_vec = vec1.template viewas_dvector<T>().sizes();
      auto size_vec_data = size_vec.data();
      auto size_vec_len = size_vec.size();
      size_t start_node = 0;
      for (size_t i = 0; i < size_vec_len; ++i) {
        if (size_vec_data[i] < start) 
          start -= size_vec_data[i];
        else{ 
          start_node = i;
          break;
        }
      }
      return vec1.map(t_crop_helper<T>, broadcast(start_node), broadcast(start));
    }
    
    node_local<std::vector<T>>
    push_back_element(node_local<std::vector<T>>& vec1,T data) {
      auto size_vec = vec1.template viewas_dvector<T>().sizes();
      auto size_vec_data = size_vec.data();
      auto size_vec_len = size_vec.size();
      size_t i = 0;
      for (i = size_vec_len - 1; i > 0; --i) {
        if (size_vec_data[i]) break;
      }
      return vec1.map(push_back_helper<T>, broadcast(data), broadcast(i));
    }
 
    node_local<rowmajor_matrix_local<T>>
    make_rmml(node_local<std::vector<T>>& vec,size_t lag) {
      return (vec.map(make_rmml_helper<T>, broadcast(lag)));
    }
    
    node_local<std::vector<T>>
    solver(rowmajor_matrix<T>& mat, std::vector<T> coeff, T intercept) {
      return mat.data.map(solver_helper<T>, broadcast(coeff),
                          broadcast(intercept));
    } 
    
    T calc_rmse(node_local<std::vector<T>>& v1, 
                node_local<std::vector<T>>& v2, size_t sz) {
      return sqrt((v1.map(calc_var<T>,v2)
                     .reduce(calc_s<T>))/sz);
    }

    T ols_estimator(const node_local<std::vector<T>>& data,
                    const size_t& lag,
                    node_local<std::vector<T>>& predicted_values,
                    node_local<std::vector<T>>& value,
                    std::vector<T>& coeff,
                    T& intercept,
                    bool is_AR) {
      auto diff_vec = data;
      diff_vec = dropall_NaN(diff_vec);
      size_t train_size = sample_size - diff_order - seasonal - lag; 
      if(!is_AR) 
        train_size -= ar_lag;
    
      node_local<std::vector<T>> train_vec;
      auto shift_vec = diff_vec;
      if (lag ){
        shift_vec = shift2(shift_vec, 1);
        train_vec = t_crop(shift_vec, lag); 
      }
      for (size_t i = 2; i<= lag; ++i) {
        shift_vec = shift2(shift_vec, 1);
        train_vec = appender(train_vec, t_crop(shift_vec, lag)); 
      }
      auto x_train_matc = rowmajor_matrix<T>(make_rmml(train_vec,lag));
      x_train_matc.set_num(train_size, lag);
      auto x_train_solver = x_train_matc;
      auto y_train = t_crop(diff_vec, lag);
      auto y_train_dvec = y_train.template as_dvector<T>();
      const std::string& solver_l = solverx;    
      auto lm = linear_regression<T>().set_solver(solver_l).set_intercept(true);
      auto result= lm.fit(x_train_matc, y_train_dvec);
      coeff = result.model.weight;
      intercept = result.model.intercept;
      predicted_values = solver(x_train_solver, coeff, intercept); 
      value = y_train;
      auto  rmse = calc_rmse(y_train, predicted_values, train_size);
      return rmse;
    }

    void calc_ols(const node_local<std::vector<T>>& data, T& _rmse, 
                  node_local<std::vector<T>>& _predict, 
                  node_local<std::vector<T>>& _value, std::vector<T>& _coeff, 
                  T& _intercept, size_t& _lag, bool _is_AR, std::string _x) {
      T best_RMSE = TMAX;
      if (auto_arima){
        size_t max_ar_iter = _lag;
        for (size_t i = 1; i <= max_ar_iter; ++i){
          _rmse = ols_estimator(data, i, _predict, _value, 
                               _coeff, _intercept, _is_AR);
          if (_rmse < best_RMSE){
            best_RMSE = _rmse;
            _lag = i;
          }
        }
      RLOG(DEBUG) << "Best " << _x << " RMSE for lag " << _lag << " is " << best_RMSE << "\n";
      }
    }

    void fit() {
      diff_data = sample_data;
      differencing (diff_data);
      T rmse = TMAX;
      node_local<std::vector<T>> ar_predict;
      node_local<std::vector<T>> ar_value;
      std::vector<T> ar_coeff;
      T ar_intercept = 0.0;
      calc_ols(diff_data, rmse, ar_predict, ar_value, ar_coeff,
               ar_intercept, ar_lag, true, "AR");
      rmse = ols_estimator(diff_data, ar_lag, ar_predict, 
                           ar_value, ar_coeff, ar_intercept, true);
      best_ar_coeff = ar_coeff;
      best_ar_intercept = ar_intercept;

      if (ma_lag){
        node_local<std::vector<T>> residuals = ar_value.map(calc_subtract<T>, 
                                                            ar_predict);
        rmse = TMAX;
        T ma_intercept = 0.0;
        std::vector<T> ma_coeff;
        node_local<std::vector<T>> ma_value;
        node_local<std::vector<T>> ma_predict;
        calc_ols(residuals, rmse, ma_predict, ma_value, ma_coeff, 
                 ma_intercept, ma_lag, false, "MA");
        rmse = ols_estimator(residuals, ma_lag, ma_predict, ma_value, 
                             ma_coeff, ma_intercept, false);
        best_ma_coeff = ma_coeff;
        best_ma_intercept = ma_intercept;
        ma_predict_data = ma_predict ;
        ar_predict = residual_adder(ar_predict, ma_predict, ma_lag, true);

      }
      auto undiff_predicted_val = undifferencing(sample_data, ar_predict);
      fitted_values = undiff_predicted_val
                        .map(front_zero_helper<T>, 
                             broadcast(diff_order + ar_lag + seasonal)) ;
      is_fitted = true;    
    }

    node_local <std::vector<T>>
    create_lag(node_local <std::vector<T>>& pred_data, size_t pos, size_t lag) {
      auto size_vec = pred_data.template viewas_dvector<T>().sizes();
      auto size_vec_data = size_vec.data();
      auto size_vec_len = size_vec.size();
      size_t stop_node = 0;

      for (size_t i = 0; i < size_vec_len; ++i) {
        if (size_vec_data[i] < pos)
          pos -= size_vec_data[i];

        else{
          stop_node = i;
          break;
        }
      }
      if (size_vec_data[stop_node] < lag){
        return extend_lower_shadow(pred_data, lag - size_vec_data[stop_node])
                 .map(create_lag_helper<T>, broadcast(stop_node), broadcast (lag));   
      }
       
      return pred_data.map(create_lag_helper<T>, 
                           broadcast(stop_node), 
                           broadcast(lag));

    }

      
    T
    predict_solver(node_local <std::vector<T>>& dt,std::vector<T> coeff,T intercept){
      return dt.map(predict_solver_helper<T>,
                    broadcast(coeff),
                    broadcast(intercept))
               .reduce(calc_s<T>);


    }


    node_local <std::vector<T>>
    create_distdifflag (node_local <std::vector<T>>&  pred_data, size_t pos,size_t lag) {
      auto size_vec = pred_data.template viewas_dvector<T>().sizes();
      auto size_vec_data = size_vec.data();
      auto size_vec_len = size_vec.size();
      size_t start_node =0,stop_node = 0, start_pos = pos - lag;
      for (size_t i = 0; i < size_vec_len; ++i) {
        if (size_vec_data[i] < start_pos)
          start_pos -= size_vec_data[i];
        else {
          start_node = i;
        }
	if (size_vec_data[i] < pos)
          pos -= size_vec_data[i];
        else {
          stop_node = i;
          break;
        }
      }
      return pred_data.map(create_distdifflag_helper<T>,
                           broadcast(start_node),
			   broadcast(start_pos),
                           broadcast(stop_node),
			   broadcast(pos));
    }


    node_local <std::vector<T>>
    create_nondistdifflag (node_local <std::vector<T>>& pred_data, size_t pos,size_t lag) {
      auto size_vec = pred_data.template viewas_dvector<T>().sizes();
      auto size_vec_data = size_vec.data();
      auto size_vec_len = size_vec.size();
      size_t stop_node = 0;
      for (size_t i = 0; i < size_vec_len; ++i) {
	if (size_vec_data[i] < pos)
          pos -= size_vec_data[i];
        else{
          stop_node = i;
          break;
        }
      }
      if (pos-lag < 0){
        pred_data = extend_lower_shadow(pred_data, lag-pos);
      }     
      return pred_data.map(create_nondistdifflag_helper<T>,
                           broadcast(stop_node),
			   broadcast(pos),
                           broadcast(lag));
    }

    std::vector<T>
    predict(size_t start_step, size_t stop_step) {
      if(stop_step < start_step)
        REPORT_ERROR(USER_ERROR, "Stop index cannot be less than Start index\n");
      auto steps = stop_step - start_step + 1;
      if(int(start_step - ma_lag - ar_lag - diff_order - seasonal) <= 0) {
          REPORT_ERROR(USER_ERROR, "Index provided cannot be less than total ARIMA order\n");
      }
      node_local <std::vector<T>> lag_data;
      size_t start, stp;
      if (!is_fitted)
        REPORT_ERROR(USER_ERROR, "Cannot use predict before fit()!\n");

      if (start_step > sample_size) {
        start = sample_size;
        stp = steps + start_step - sample_size;
      }
      else{
        start = start_step;
        stp = steps;
      }
      auto predict_diff_data = create_nondistdifflag(diff_data, start, ar_lag);
      std::vector<T> result(stp);
      T res;
      node_local<std::vector<T>> ma_temp_data;
      auto ma_gap = diff_order + seasonal + ar_lag + ma_lag;
      if (ma_lag) 
        ma_temp_data = create_nondistdifflag(ma_predict_data, 
                                             start - ma_gap, 
                                             ma_lag);

      auto smp_undiff = create_distdifflag (sample_data, start, diff_order + seasonal); 
  
      for (size_t i = 0; i < stp; ++i){
        lag_data =  create_lag(predict_diff_data, ar_lag + i, ar_lag);
        res = predict_solver(lag_data, best_ar_coeff, best_ar_intercept);
        result[i] = res;

        if (ma_lag) {
          lag_data =  create_lag(ma_temp_data , ma_lag+i , ma_lag);
          res = predict_solver(lag_data, best_ma_coeff, best_ma_intercept);
          result[i] += res;
          ma_temp_data = push_back_element(ma_temp_data, res);
        }
        predict_diff_data = push_back_element(predict_diff_data, result[i]);
      
  
        single_undifferencing_new(smp_undiff,result[i], diff_order + seasonal + i);
        smp_undiff = push_back_element(smp_undiff, result[i]);
      }

      if (start_step > sample_size){
        std::vector<T> new_res(steps);
        auto new_res_data = new_res.data();
        auto res_data = result.data();
        size_t c_step = start_step - sample_size;
        for(size_t i = 0; i < steps; ++i) new_res_data[i] = res_data[c_step + i];

        return new_res;
      }
      return result;
    }

    std::vector<T> 
    forecast(size_t steps = 1) {
       return predict(sample_size, sample_size + steps - 1);
    } 

};
}
#endif
