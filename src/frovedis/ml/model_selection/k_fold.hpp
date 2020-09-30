#ifndef __KFOLD__
#define __KFOLD__

#include "splitter_merger.incl"

namespace frovedis {

template <class MATRIX, class T>
struct labeled_point {
  labeled_point() {}
  labeled_point(MATRIX& mat, dvector<T>& lbl) {
    point = mat;
    label = lbl;
  }
  labeled_point(MATRIX&& mat, dvector<T>&& lbl) {
    point = std::move(mat);
    label = std::move(lbl);
  }
  void debug_print(size_t n = 0) {
    std::cout << "points: \n"; point.debug_print(n);
    std::cout << "labels: \n"; debug_print_vector(label.gather(), n);
  }
  MATRIX point;
  dvector<T> label;
  SERIALIZE(point, label);
};

template <class MATRIX, class T>
struct k_fold {
  k_fold(int k = 5): nsplits(k) {}

  k_fold& fit(MATRIX& mat, 
              dvector<T>& lbl) {
    split_points(mat.gather());
    split_labels(lbl.gather());
    return *this;
  }

  std::pair<labeled_point<MATRIX,T>, 
            labeled_point<MATRIX,T>>
  operator[](size_t index) {
    auto train_mat = merge_k_minus_one_splitted_points(index);
    auto train_lbl = merge_k_minus_one_splitted_labels(index);
    auto train_data = labeled_point<MATRIX,T>(make_dmatrix_scatter(train_mat),
                                              make_dvector_scatter(train_lbl));
    auto test_mat = get_ith_splitted_points(index);
    auto test_lbl = get_ith_splitted_labels(index);
    auto test_data = labeled_point<MATRIX,T>(make_dmatrix_scatter(test_mat),
                                             make_dvector_scatter(test_lbl));
    return std::make_pair(train_data, test_data);
  }

  typename MATRIX::local_mat_type 
  get_ith_splitted_points(size_t index) { 
    return splitted_points[index]; 
  }

  std::vector<T> 
  get_ith_splitted_labels(size_t index) { 
    return splitted_labels[index]; 
  }

  typename MATRIX::local_mat_type 
  merge_k_minus_one_splitted_points(size_t not_index) {
    return merge_splitted_matrices<T>(splitted_points, not_index);  
  }
  
  std::vector<T> 
  merge_k_minus_one_splitted_labels(size_t not_index) {
    return merge_splitted_vectors<T>(splitted_labels, not_index);  
  }

  void debug_print(size_t n = 0) {
    std::cout << "points: \n"; 
    for(auto& e: splitted_points) e.debug_print(n);
    std::cout << "labels: \n"; 
    for(auto& e: splitted_labels) debug_print_vector(e, n);
  }

  size_t get_nsplits() { return nsplits; }

  private:
  void split_points(const typename MATRIX::local_mat_type& loc_mat) {
    splitted_points = prepare_scattered_matrices<T>(loc_mat, nsplits);
  }

  void split_labels(const std::vector<T>& loc_lbl) {
    splitted_labels = prepare_scattered_vectors<T>(loc_lbl, nsplits);
  }

  // --- data members ---
  size_t nsplits;
  std::vector<typename MATRIX::local_mat_type> splitted_points;
  std::vector<std::vector<T>> splitted_labels;
  SERIALIZE(nsplits, splitted_points, splitted_labels)
};

}
#endif
