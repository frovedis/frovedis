#ifndef _TEST_RMSE_
#define _TEST_RMSE_

template <class T>
double calc_rms_err (std::vector<T>& v1,
                     std::vector<T>& v2) {
   if(v1.size() != v2.size()) return 1.0;
   T sum = 0.0;
   for(size_t i=0; i<v1.size(); ++i) {
      T diff = v1[i] - v2[i];
      sum += (diff * diff);
   }
   return sqrt(sum/v1.size());
}

#endif
