#include "vector_operations.hpp"

namespace frovedis {

template <>
std::vector<std::string> 
vector_add(const std::vector<std::string>& vec,
           const std::string& by_elem) {
  if (by_elem == "") return vec;
  auto vecsz = vec.size();
  std::vector<std::string> ret(vecsz);
  for(size_t i = 0; i < vecsz; ++i) ret[i] = vec[i] + by_elem;
  return ret;
}

template <>
std::vector<int>
vector_divide(const std::vector<int>& vec,
              const int& by_elem) {
  auto vecsz = vec.size();
  if (by_elem == 0) {
    REPORT_WARNING(WARNING_MESSAGE,
        "RuntimeWarning: divide by zero encountered in divide");
    return vector_zeros<int>(vecsz);
  }
  std::vector<int> ret(vecsz);
  auto vecp = vec.data();
  auto retp = ret.data();
  for(size_t i = 0; i < vecsz; ++i) retp[i] = vecp[i] / by_elem;
  return ret;
}

template <>
int vector_squared_sum(const std::vector<int>& vec) {
  auto sz = vec.size();
  if (sz == 0) return 0;
  auto vptr = vec.data();
  int sqsum = 0;
  // might overflow here... 
  for(size_t i = 0; i < sz; ++i) sqsum += vptr[i] * vptr[i];
  return sqsum;
}

// similar to numpy.sqrt(x) for integral x
std::vector<double>
integral_vector_sqrt(const std::vector<int>& vec) {
  auto vecsz = vec.size();
  auto vecp = vec.data();
  std::vector<double> ret(vecsz);
  auto retp = ret.data();
  for(size_t i = 0; i < vecsz; ++i) {
    retp[i] = (vecp[i] == 0) ? 0.0 : std::sqrt(vecp[i]);
  }
  return ret;
}

template <>
size_t vector_count_negatives(const std::vector<size_t>& vec) {
  return 0; // vector of size_t (unsigned) should have all elements >= 0
}

std::vector<size_t>
extract_sliced_idx(size_t my_st, size_t my_end,
                   size_t g_st, size_t g_end, size_t step) {
  size_t lb = 0, ub = 0;
  auto imax = std::numeric_limits<size_t>::max();
  if (my_st > g_st && my_st > g_end)         { lb = ub = imax; } // slice not possible
  else if (my_st < g_st && my_end < g_st)    { lb = ub = imax; } // slice not possible
  else if (my_st <= g_st && my_end >= g_end) { lb = g_st; ub = g_end; }
  else if (my_st <= g_st && my_end < g_end)  { lb = g_st; ub = my_end; }
  else if (my_st > g_st && my_end <= g_end)  { lb = my_st; ub = my_end; }
  else if (my_st > g_st && my_end >= g_end)  { lb = my_st; ub = g_end; }
  else { // should never occur
    std::string msg = "[" + STR(g_st) + " : " + STR(g_end) + "] => ";
    msg += "[" + STR(my_st) + " : " + STR(my_end) + "]";
    REPORT_ERROR(INTERNAL_ERROR,
    "extract_sliced_idx failed to perform bound check: " + msg + "! Report Bug!\n");
  }
  std::vector<size_t> ret;
  if (lb != imax) { // if slice is possible
    auto traversed = lb - g_st;
    if (traversed % step) lb += step - (traversed % step); // adjust lb as per 'step'
    ret = vector_arrange<size_t>(lb - my_st, ub - my_st, step);
  }
  /*
   *  auto myrank = get_selfid();
   *  std::cout << "[" << myrank << "] my_st: " << my_st << "; my_end " << my_end << std::endl;
   *  std::cout << "[" << myrank << "] lb: " << lb << "; ub: " << ub << std::endl;
   *  show("ret: ", ret);
   */
  return ret;
}

node_local<std::vector<size_t>>
make_sliced_impl(const std::vector<size_t> sizes,
                 size_t nrow, size_t st, size_t end,
                 size_t step) {
  end = std::min(end, nrow); // allowed end to be larger than nrow
  auto idx = make_node_local_allocate<std::vector<size_t>>();
  if (nrow && st < end) {
    auto nproc = sizes.size();
    std::vector<size_t> myst(nproc), myend(nproc);
    myst[0] = 0;
    for(size_t i = 1; i < nproc; ++i) myst[i] = myst[i - 1] + sizes[i - 1];
    for(size_t i = 0; i < nproc; ++i) myend[i] = myst[i] + sizes[i];
    auto mst = make_node_local_scatter(myst);
    auto mend = make_node_local_scatter(myend);
    idx = mst.map(extract_sliced_idx, mend, broadcast(st),
                  broadcast(end), broadcast(step));
  }
  return idx;
}

}
