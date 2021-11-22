#include "frovedis/text/words.hpp"
#include "exrpc_data_storage.hpp"
#include "short_hand_dtype.hpp"

using namespace frovedis;

exrpc_ptr_t make_node_local_words(std::vector<exrpc_ptr_t>& data_ptrs, 
                                  std::vector<exrpc_ptr_t>& size_ptrs) {
  auto sizesp = new dvector<int>(make_dvector_allocate<int>());
  auto each_size_eps =  make_node_local_scatter(size_ptrs);
  auto sizes_ss = sizesp->map_partitions(merge_and_set_dvector_impl<int>, 
                                         each_size_eps).gather();
  sizesp->set_sizes(sizes_ss);
  sizesp->align_block(); 

  auto datap = new dvector<int>(make_dvector_allocate<int>());
  auto each_data_eps =  make_node_local_scatter(data_ptrs);
  auto data_ss = datap->map_partitions(merge_and_set_dvector_impl<int>, 
                                       each_data_eps).gather();
  datap->set_sizes(data_ss);
  auto dist = sizesp->viewas_node_local()
                    .map(+[](const std::vector<int>& sizes) {
                          return static_cast<size_t>(vector_sum(sizes));
                     }).gather();
  datap->align_as(dist); // to ensure data and sizes distributions are in-sync

  auto retp = new node_local<words>(make_node_local_allocate<words>());
  datap->viewas_node_local().mapv(
       +[](std::vector<int>& data, std::vector<int>& sizes, words& w) {
           auto size = sizes.size();
           if (size == 0) return;
           std::vector<size_t> starts(size); starts[0] = 0;
           auto length = vector_astype<size_t>(sizes);
           prefix_sum(length.data(), starts.data() + 1, size - 1);
           w.chars.swap(data);
           w.lens.swap(length);
           w.starts.swap(starts);
       }, sizesp->viewas_node_local(), *retp);
  return reinterpret_cast<exrpc_ptr_t>(retp);
}

std::vector<std::string> get_string_vector_from_words(exrpc_ptr_t& wordsptr) {
  auto& w = *reinterpret_cast<words*>(wordsptr);
  return words_to_vector_string(w);
}

std::vector<exrpc_ptr_t>
get_node_local_word_pointers(exrpc_ptr_t& words_nl_ptr) {
  auto& w_nl = *reinterpret_cast<node_local<words>*>(words_nl_ptr);
  return w_nl.map(+[](words& w) {
                   auto wptr = &w;
                   return reinterpret_cast<exrpc_ptr_t>(wptr);
                 }).gather();
}

void expose_frovedis_dvector_functions() {
  expose(count_distinct<int>);
  expose(count_distinct<long>);
  expose(count_distinct<unsigned long>);
  expose(count_distinct<float>);
  expose(count_distinct<double>);
  expose(get_distinct_elements<int>);
  expose(get_distinct_elements<long>);
  expose(get_distinct_elements<unsigned long>);
  expose(get_distinct_elements<float>);
  expose(get_distinct_elements<double>);
  expose(get_encoded_dvector<int>);
  expose(get_encoded_dvector<long>);
  expose(get_encoded_dvector<unsigned long>);
  expose(get_encoded_dvector<float>);
  expose(get_encoded_dvector<double>);
  expose(get_encoded_dvector_zero_based<int>);
  expose(get_encoded_dvector_zero_based<long>);
  expose(get_encoded_dvector_zero_based<unsigned long>);
  expose(get_encoded_dvector_zero_based<float>);
  expose(get_encoded_dvector_zero_based<double>);
  // --- frovedis dvector for labels ---
  expose((load_local_data<std::vector<DT1>>));
  expose(create_and_set_dvector<DT1>);
  expose(show_dvector<DT1>);
  expose(release_dvector<DT1>);
  // --- frovedis typed dvector for dataframes ---
  // for spark (allocate_local_vector, load_local_vector, merge_and_set_dvector)
  expose((allocate_local_vector<std::vector<int>>));
  expose((load_local_vector<std::vector<int>>));
  expose(merge_and_set_dvector<int>);
  expose((allocate_local_vector<std::vector<long>>));
  expose((load_local_vector<std::vector<long>>));
  expose(merge_and_set_dvector<long>);
  expose((allocate_local_vector<std::vector<float>>));
  expose((load_local_vector<std::vector<float>>));
  expose(merge_and_set_dvector<float>);
  expose((allocate_local_vector<std::vector<double>>));
  expose((load_local_vector<std::vector<double>>));
  expose(merge_and_set_dvector<double>);
  expose((allocate_local_vector<std::vector<std::string>>));
  expose((load_local_vector<std::vector<std::string>>));
  expose(merge_and_set_dvector<std::string>);
  // for handling strings as words (chars, sizes pair)
  expose((allocate_local_vector_pair<std::vector<int>, std::vector<int>>));
  expose((load_local_vector_pair<std::vector<int>, std::vector<int>>));
  expose(make_node_local_words);
  expose(get_node_local_word_pointers);
  expose(get_string_vector_from_words);
  // for python (load_local_data, create_and_set_dvector)
  expose((load_local_data<std::vector<int>>));
  expose((load_local_data<std::vector<long>>));
  expose((load_local_data<std::vector<unsigned long>>));
  expose((load_local_data<std::vector<float>>));
  expose((load_local_data<std::vector<double>>));
  expose((load_local_data<std::vector<std::string>>));
  expose(create_and_set_dvector<int>);
  expose(create_and_set_dvector<long>);
  expose(create_and_set_dvector<unsigned long>);
  expose(create_and_set_dvector<float>);
  expose(create_and_set_dvector<double>);
  expose(create_and_set_dvector<std::string>);
  //expose common (spark/python) dvector functionalities
  expose(show_dvector<int>);
  expose(show_dvector<long>);
  expose(show_dvector<unsigned long>);
  expose(show_dvector<float>);
  expose(show_dvector<double>);
  expose(show_dvector<std::string>);
  expose(release_dvector<int>);
  expose(release_dvector<long>);
  expose(release_dvector<unsigned long>);
  expose(release_dvector<float>);
  expose(release_dvector<double>);
  expose(release_dvector<std::string>);
  // frovedis (simple std::vector) vector functionalities
  expose(create_frovedis_vector<int>);
  expose(create_frovedis_vector<long>);
  expose(create_frovedis_vector<float>);
  expose(create_frovedis_vector<double>);
  expose(create_frovedis_vector<std::string>);
  expose(save_frovedis_vector<int>);
  expose(save_frovedis_vector<long>);
  expose(save_frovedis_vector<float>);
  expose(save_frovedis_vector<double>);
  //expose(save_frovedis_vector<std::string>); // not supported in frovedis
  //expose frovedis vector load
  expose(load_frovedis_vector<int>);
  expose(load_frovedis_vector<long>);
  expose(load_frovedis_vector<float>);
  expose(load_frovedis_vector<double>);
  //expose(load_frovedis_vector<std::string>); // not supported in frovedis
  //expose frovedis vector release
  expose((release_data<std::vector<int>>));    // FrovedisVector + LAPACK(IPIV)
  expose((release_data<std::vector<long>>));   // FrovedisVector
  expose((release_data<std::vector<float>>));  // FrovedisVector + SVAL/SVEC
  expose((release_data<std::vector<double>>)); // FrovedisVector + SVAL/SVEC/EIGVAL
  expose((release_data<std::vector<std::string>>));  // FrovedisVector
  // dvector to spark RDD or python numpy array
  expose(get_dvector_local_pointers<int>);
  expose(get_dvector_local_pointers<long>);
  expose(get_dvector_local_pointers<unsigned long>);
  expose(get_dvector_local_pointers<float>);
  expose(get_dvector_local_pointers<double>);
  expose(get_dvector_local_pointers<std::string>);
  expose(get_local_vector<int>);
  expose(get_local_vector<long>);
  expose(get_local_vector<unsigned long>);
  expose(get_local_vector<float>);
  expose(get_local_vector<double>);
  expose(get_local_vector<std::string>);
}
