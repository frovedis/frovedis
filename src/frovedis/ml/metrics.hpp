#ifndef _METRICS_
#define _METRICS_

namespace frovedis {

template <class T>
float accuracy_score(const std::vector<T>& pred_lbl,
                     const std::vector<T>& actual_lbl) {
  auto size = pred_lbl.size();
  checkAssumption(size == actual_lbl.size());
  auto predp = pred_lbl.data();
  auto actualp = actual_lbl.data();
  size_t count = 0;
  for(size_t i = 0; i < size; ++i) count += predp[i] == actualp[i];
  return static_cast<float>(count) / size;
}

}

#endif
