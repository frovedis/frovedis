#include "exrpc_data_storage.hpp"
#include "short_hand_dtype.hpp"

using namespace frovedis;

void expose_frovedis_dvector_functions() {
  expose(count_distinct<int>);
  expose(count_distinct<long>);
  expose(count_distinct<float>);
  expose(count_distinct<double>);
  expose(get_distinct_elements<int>);
  expose(get_distinct_elements<long>);
  expose(get_distinct_elements<float>);
  expose(get_distinct_elements<double>);
  expose(get_encoded_dvector<int>);
  expose(get_encoded_dvector<long>);
  expose(get_encoded_dvector<float>);
  expose(get_encoded_dvector<double>);
  expose(get_encoded_dvector_zero_based<int>);
  expose(get_encoded_dvector_zero_based<long>);
  expose(get_encoded_dvector_zero_based<float>);
  expose(get_encoded_dvector_zero_based<double>);
  // --- frovedis dvector for labels ---
  expose((load_local_data<std::vector<DT1>>));
  expose(create_and_set_dvector<DT1>);
  expose(show_dvector<DT1>);
  expose(release_dvector<DT1>);
  // --- frovedis typed dvector for dataframes ---
  expose((load_local_data<std::vector<int>>));
  expose((load_local_data<std::vector<long>>));
  expose((load_local_data<std::vector<float>>));
  expose((load_local_data<std::vector<double>>));
  expose((load_local_data<std::vector<std::string>>));
  expose(create_and_set_dvector<int>);
  expose(create_and_set_dvector<long>);
  expose(create_and_set_dvector<float>);
  expose(create_and_set_dvector<double>);
  expose(create_and_set_dvector<std::string>);
  expose(show_dvector<int>);
  expose(show_dvector<long>);
  expose(show_dvector<float>);
  expose(show_dvector<double>);
  expose(show_dvector<std::string>);
  expose(release_dvector<int>);
  expose(release_dvector<long>);
  expose(release_dvector<float>);
  expose(release_dvector<double>);
  expose(release_dvector<std::string>);
  //expose frovedis vector create
  expose(create_frovedis_vector<int>);
  expose(create_frovedis_vector<long>);
  expose(create_frovedis_vector<float>);
  expose(create_frovedis_vector<double>);
  expose(create_frovedis_vector<std::string>);
  // expose frovedis vector save
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
  // dvector to spark RDD
  expose(get_dvector_local_pointers<int>);
  expose(get_dvector_local_pointers<long>);
  expose(get_dvector_local_pointers<float>);
  expose(get_dvector_local_pointers<double>);
  expose(get_dvector_local_pointers<std::string>);
  expose(get_local_vector<int>);
  expose(get_local_vector<long>);
  expose(get_local_vector<float>);
  expose(get_local_vector<double>);
  expose(get_local_vector<std::string>);
}
