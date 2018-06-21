#include "exrpc.hpp"

frovedis::exptr<frovedis::dvector<int>> make_dvector_sample(std::vector<int>& v);

std::vector<int> gather_sample(frovedis::exptr<frovedis::dvector<int>>& dv);

std::vector<frovedis::exptr<std::vector<int>>>
get_each_pointer(frovedis::exptr<frovedis::dvector<int>>& dv);

void add_each(frovedis::exptr<std::vector<int>>& p, std::vector<int>& v);

std::vector<int> ex_sample(frovedis::exptr<frovedis::dvector<int>>& dv);
