#include "exrpc_data_storage.hpp"
#include "short_hand_dtype.hpp"

using namespace frovedis;

void expose_frovedis_dvector_functions() {
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
}
