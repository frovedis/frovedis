#ifndef SPLIT_CONTEXT_EXECUTION_HPP
#define SPLIT_CONTEXT_EXECUTION_HPP

#include "mpi_rpc.hpp"
#include "node_local.hpp"
#include "radix_sort.hpp"
#include "set_operations.hpp"

namespace frovedis {

void init_split_context_execution(int color);
void finalize_split_context_execution();
std::vector<int> get_split_rank_stack();

// -- 0
// operator() of f might not be const
template <class F>
void split_context_execution_helper0(dvid_t& dcolor, F& f) {
  int color = *get_data<int>(dcolor);
  init_split_context_execution(color);
  bool exception_caught = false;
  std::string what;
  try {
    if(frovedis_self_rank != 0) {
      while (handle_one_bcast_req());
    } else {
      f();
    }
  } catch (std::exception& e) {
    exception_caught = true;
    what = e.what();
  }
  finalize_split_context_execution();
  if(exception_caught) throw std::runtime_error(what);
}

template <class F>
void split_context_execution(const std::vector<int>& color, const F& f) {
  if(color.size() != get_nodesize())
    throw std::runtime_error("size of color is different from node size");
  auto nlcolor = make_node_local_scatter(color);
  auto dcolor = nlcolor.get_dvid().dvid;
  bcast_rpc_oneway(split_context_execution_helper0<F>, dcolor, f);
}

void split_context_execution(const std::vector<int>& color, void(*fp)());

// -- 1
template <class F, class T1>
void split_context_execution_helper1(dvid_t& dcolor, F& f, dvid_t& a1) {
  auto a1p = get_data<T1>(a1);
  int color = *get_data<int>(dcolor);
  init_split_context_execution(color);
  // new dvid is created on all node; different from normal case
  auto dvid1 = get_new_dvid(); 
  dvid_table[dvid1] = reinterpret_cast<intptr_t>(a1p);
  bool exception_caught = false;
  std::string what;
  try {
    if(frovedis_self_rank != 0) {
      while (handle_one_bcast_req());
    } else {
      auto na1 = node_local<T1>(DVID<T1>(dvid1),true);
      f(na1);
    }
  } catch (std::exception& e) {
    exception_caught = true;
    what = e.what();
  }
  finalize_split_context_execution();
  if(exception_caught) throw std::runtime_error(what);
}

template <class F, class T1>
void split_context_execution(const std::vector<int>& color, const F& f,
                             node_local<T1>& a1) {
  if(color.size() != get_nodesize())
    throw std::runtime_error("size of color is different from node size");
  auto nlcolor = make_node_local_scatter(color);
  auto dcolor = nlcolor.get_dvid().dvid;
  auto da1 = a1.get_dvid().dvid;
  bcast_rpc_oneway(split_context_execution_helper1<F,T1>, dcolor, f, da1);
}

template <class T1>
void split_context_execution(const std::vector<int>& color,
                             void(*fp)(node_local<T1>&),
                             node_local<T1>& a1) {
  split_context_execution(color, make_serfunc(fp), a1);
}

// -- 2
template <class F, class T1, class T2>
void split_context_execution_helper2(dvid_t& dcolor, F& f, dvid_t& a1,
                                     dvid_t& a2) {
  auto a1p = get_data<T1>(a1);
  auto a2p = get_data<T2>(a2);
  int color = *get_data<int>(dcolor);
  init_split_context_execution(color);
  auto dvid1 = get_new_dvid(); 
  dvid_table[dvid1] = reinterpret_cast<intptr_t>(a1p);
  auto dvid2 = get_new_dvid(); 
  dvid_table[dvid2] = reinterpret_cast<intptr_t>(a2p);
  bool exception_caught = false;
  std::string what;
  try {
    if(frovedis_self_rank != 0) {
      while (handle_one_bcast_req());
    } else {
      auto na1 = node_local<T1>(DVID<T1>(dvid1),true);
      auto na2 = node_local<T2>(DVID<T2>(dvid2),true);
      f(na1, na2);
    }
  } catch (std::exception& e) {
    exception_caught = true;
    what = e.what();
  }
  finalize_split_context_execution();
  if(exception_caught) throw std::runtime_error(what);
}

template <class F, class T1, class T2>
void split_context_execution(const std::vector<int>& color, const F& f,
                             node_local<T1>& a1, node_local<T2>& a2) {
  if(color.size() != get_nodesize())
    throw std::runtime_error("size of color is different from node size");
  auto nlcolor = make_node_local_scatter(color);
  auto dcolor = nlcolor.get_dvid().dvid;
  auto da1 = a1.get_dvid().dvid;
  auto da2 = a2.get_dvid().dvid;
  bcast_rpc_oneway(split_context_execution_helper2<F,T1,T2>, dcolor, f,
                   da1, da2);
}

template <class T1, class T2>
void split_context_execution(const std::vector<int>& color,
                             void(*fp)(node_local<T1>&,
                                       node_local<T2>&),
                             node_local<T1>& a1,
                             node_local<T2>& a2) {
  split_context_execution(color, make_serfunc(fp), a1, a2);
}

// -- 3
template <class F, class T1, class T2, class T3>
void split_context_execution_helper3(dvid_t& dcolor, F& f, dvid_t& a1,
                                     dvid_t& a2, dvid_t& a3) {
  auto a1p = get_data<T1>(a1);
  auto a2p = get_data<T2>(a2);
  auto a3p = get_data<T3>(a3);
  int color = *get_data<int>(dcolor);
  init_split_context_execution(color);
  auto dvid1 = get_new_dvid(); 
  dvid_table[dvid1] = reinterpret_cast<intptr_t>(a1p);
  auto dvid2 = get_new_dvid(); 
  dvid_table[dvid2] = reinterpret_cast<intptr_t>(a2p);
  auto dvid3 = get_new_dvid(); 
  dvid_table[dvid3] = reinterpret_cast<intptr_t>(a3p);
  bool exception_caught = false;
  std::string what;
  try {
    if(frovedis_self_rank != 0) {
      while (handle_one_bcast_req());
    } else {
      auto na1 = node_local<T1>(DVID<T1>(dvid1),true);
      auto na2 = node_local<T2>(DVID<T2>(dvid2),true);
      auto na3 = node_local<T3>(DVID<T3>(dvid3),true);
      f(na1, na2, na3);
    }
  } catch (std::exception& e) {
    exception_caught = true;
    what = e.what();
  }
  finalize_split_context_execution();
  if(exception_caught) throw std::runtime_error(what);
}

template <class F, class T1, class T2, class T3>
void split_context_execution(const std::vector<int>& color, const F& f,
                             node_local<T1>& a1, node_local<T2>& a2,
                             node_local<T3>& a3) {
  if(color.size() != get_nodesize())
    throw std::runtime_error("size of color is different from node size");
  auto nlcolor = make_node_local_scatter(color);
  auto dcolor = nlcolor.get_dvid().dvid;
  auto da1 = a1.get_dvid().dvid;
  auto da2 = a2.get_dvid().dvid;
  auto da3 = a3.get_dvid().dvid;
  bcast_rpc_oneway(split_context_execution_helper3<F,T1,T2,T3>, dcolor, f,
                   da1, da2, da3);
}

template <class T1, class T2, class T3>
void split_context_execution(const std::vector<int>& color,
                             void(*fp)(node_local<T1>&,
                                       node_local<T2>&,
                                       node_local<T3>&),
                             node_local<T1>& a1,
                             node_local<T2>& a2,
                             node_local<T3>& a3) {
  split_context_execution(color, make_serfunc(fp), a1, a2, a3);
}

// -- 4
template <class F, class T1, class T2, class T3, class T4>
void split_context_execution_helper4(dvid_t& dcolor, F& f, dvid_t& a1,
                                     dvid_t& a2, dvid_t& a3, dvid_t& a4) {
  auto a1p = get_data<T1>(a1);
  auto a2p = get_data<T2>(a2);
  auto a3p = get_data<T3>(a3);
  auto a4p = get_data<T4>(a4);
  int color = *get_data<int>(dcolor);
  init_split_context_execution(color);
  auto dvid1 = get_new_dvid(); 
  dvid_table[dvid1] = reinterpret_cast<intptr_t>(a1p);
  auto dvid2 = get_new_dvid(); 
  dvid_table[dvid2] = reinterpret_cast<intptr_t>(a2p);
  auto dvid3 = get_new_dvid(); 
  dvid_table[dvid3] = reinterpret_cast<intptr_t>(a3p);
  auto dvid4 = get_new_dvid(); 
  dvid_table[dvid4] = reinterpret_cast<intptr_t>(a4p);
  bool exception_caught = false;
  std::string what;
  try {
    if(frovedis_self_rank != 0) {
      while (handle_one_bcast_req());
    } else {
      auto na1 = node_local<T1>(DVID<T1>(dvid1),true);
      auto na2 = node_local<T2>(DVID<T2>(dvid2),true);
      auto na3 = node_local<T3>(DVID<T3>(dvid3),true);
      auto na4 = node_local<T4>(DVID<T4>(dvid4),true);
      f(na1, na2, na3, na4);
    }
  } catch (std::exception& e) {
    exception_caught = true;
    what = e.what();
  }
  finalize_split_context_execution();
  if(exception_caught) throw std::runtime_error(what);
}

template <class F, class T1, class T2, class T3, class T4>
void split_context_execution(const std::vector<int>& color, const F& f,
                             node_local<T1>& a1, node_local<T2>& a2,
                             node_local<T3>& a3, node_local<T4>& a4) {
  if(color.size() != get_nodesize())
    throw std::runtime_error("size of color is different from node size");
  auto nlcolor = make_node_local_scatter(color);
  auto dcolor = nlcolor.get_dvid().dvid;
  auto da1 = a1.get_dvid().dvid;
  auto da2 = a2.get_dvid().dvid;
  auto da3 = a3.get_dvid().dvid;
  auto da4 = a4.get_dvid().dvid;
  bcast_rpc_oneway(split_context_execution_helper4<F,T1,T2,T3,T4>, dcolor, f,
                   da1, da2, da3, da4);
}

template <class T1, class T2, class T3, class T4>
void split_context_execution(const std::vector<int>& color,
                             void(*fp)(node_local<T1>&,
                                       node_local<T2>&,
                                       node_local<T3>&,
                                       node_local<T4>&),
                             node_local<T1>& a1,
                             node_local<T2>& a2,
                             node_local<T3>& a3,
                             node_local<T4>& a4) {
  split_context_execution(color, make_serfunc(fp), a1, a2, a3, a4);
}

// -- 5
template <class F, class T1, class T2, class T3, class T4, class T5>
void split_context_execution_helper5(dvid_t& dcolor, F& f, dvid_t& a1,
                                     dvid_t& a2, dvid_t& a3, dvid_t& a4,
                                     dvid_t& a5) {
  auto a1p = get_data<T1>(a1);
  auto a2p = get_data<T2>(a2);
  auto a3p = get_data<T3>(a3);
  auto a4p = get_data<T4>(a4);
  auto a5p = get_data<T5>(a5);
  int color = *get_data<int>(dcolor);
  init_split_context_execution(color);
  auto dvid1 = get_new_dvid(); 
  dvid_table[dvid1] = reinterpret_cast<intptr_t>(a1p);
  auto dvid2 = get_new_dvid(); 
  dvid_table[dvid2] = reinterpret_cast<intptr_t>(a2p);
  auto dvid3 = get_new_dvid(); 
  dvid_table[dvid3] = reinterpret_cast<intptr_t>(a3p);
  auto dvid4 = get_new_dvid(); 
  dvid_table[dvid4] = reinterpret_cast<intptr_t>(a4p);
  auto dvid5 = get_new_dvid(); 
  dvid_table[dvid5] = reinterpret_cast<intptr_t>(a5p);
  bool exception_caught = false;
  std::string what;
  try {
    if(frovedis_self_rank != 0) {
      while (handle_one_bcast_req());
    } else {
      auto na1 = node_local<T1>(DVID<T1>(dvid1),true);
      auto na2 = node_local<T2>(DVID<T2>(dvid2),true);
      auto na3 = node_local<T3>(DVID<T3>(dvid3),true);
      auto na4 = node_local<T4>(DVID<T4>(dvid4),true);
      auto na5 = node_local<T5>(DVID<T5>(dvid5),true);
      f(na1, na2, na3, na4, na5);
    }
  } catch (std::exception& e) {
    exception_caught = true;
    what = e.what();
  }
  finalize_split_context_execution();
  if(exception_caught) throw std::runtime_error(what);
}

template <class F, class T1, class T2, class T3, class T4, class T5>
void split_context_execution(const std::vector<int>& color, const F& f,
                             node_local<T1>& a1, node_local<T2>& a2,
                             node_local<T3>& a3, node_local<T4>& a4,
                             node_local<T5>& a5) {
  if(color.size() != get_nodesize())
    throw std::runtime_error("size of color is different from node size");
  auto nlcolor = make_node_local_scatter(color);
  auto dcolor = nlcolor.get_dvid().dvid;
  auto da1 = a1.get_dvid().dvid;
  auto da2 = a2.get_dvid().dvid;
  auto da3 = a3.get_dvid().dvid;
  auto da4 = a4.get_dvid().dvid;
  auto da5 = a5.get_dvid().dvid;
  bcast_rpc_oneway(split_context_execution_helper5<F,T1,T2,T3,T4,T5>, dcolor,
                   f, da1, da2, da3, da4, da5);
}

template <class T1, class T2, class T3, class T4, class T5>
void split_context_execution(const std::vector<int>& color,
                             void(*fp)(node_local<T1>&,
                                       node_local<T2>&,
                                       node_local<T3>&,
                                       node_local<T4>&,
                                       node_local<T5>&),
                             node_local<T1>& a1,
                             node_local<T2>& a2,
                             node_local<T3>& a3,
                             node_local<T4>& a4,
                             node_local<T5>& a5) {
  split_context_execution(color, make_serfunc(fp), a1, a2, a3, a4, a5);
}

// -- 6
template <class F, class T1, class T2, class T3, class T4, class T5, class T6>
void split_context_execution_helper6(dvid_t& dcolor, F& f, dvid_t& a1,
                                     dvid_t& a2, dvid_t& a3, dvid_t& a4,
                                     dvid_t& a5, dvid_t& a6) {
  auto a1p = get_data<T1>(a1);
  auto a2p = get_data<T2>(a2);
  auto a3p = get_data<T3>(a3);
  auto a4p = get_data<T4>(a4);
  auto a5p = get_data<T5>(a5);
  auto a6p = get_data<T6>(a6);
  int color = *get_data<int>(dcolor);
  init_split_context_execution(color);
  auto dvid1 = get_new_dvid(); 
  dvid_table[dvid1] = reinterpret_cast<intptr_t>(a1p);
  auto dvid2 = get_new_dvid(); 
  dvid_table[dvid2] = reinterpret_cast<intptr_t>(a2p);
  auto dvid3 = get_new_dvid(); 
  dvid_table[dvid3] = reinterpret_cast<intptr_t>(a3p);
  auto dvid4 = get_new_dvid(); 
  dvid_table[dvid4] = reinterpret_cast<intptr_t>(a4p);
  auto dvid5 = get_new_dvid(); 
  dvid_table[dvid5] = reinterpret_cast<intptr_t>(a5p);
  auto dvid6 = get_new_dvid(); 
  dvid_table[dvid6] = reinterpret_cast<intptr_t>(a6p);
  bool exception_caught = false;
  std::string what;
  try {
    if(frovedis_self_rank != 0) {
      while (handle_one_bcast_req());
    } else {
      auto na1 = node_local<T1>(DVID<T1>(dvid1),true);
      auto na2 = node_local<T2>(DVID<T2>(dvid2),true);
      auto na3 = node_local<T3>(DVID<T3>(dvid3),true);
      auto na4 = node_local<T4>(DVID<T4>(dvid4),true);
      auto na5 = node_local<T5>(DVID<T5>(dvid5),true);
      auto na6 = node_local<T6>(DVID<T6>(dvid6),true);
      f(na1, na2, na3, na4, na5, na6);
    }
  } catch (std::exception& e) {
    exception_caught = true;
    what = e.what();
  }
  finalize_split_context_execution();
  if(exception_caught) throw std::runtime_error(what);
}

template <class F, class T1, class T2, class T3, class T4, class T5, class T6>
void split_context_execution(const std::vector<int>& color, const F& f,
                             node_local<T1>& a1, node_local<T2>& a2,
                             node_local<T3>& a3, node_local<T4>& a4,
                             node_local<T5>& a5, node_local<T6>& a6) {
  if(color.size() != get_nodesize())
    throw std::runtime_error("size of color is different from node size");
  auto nlcolor = make_node_local_scatter(color);
  auto dcolor = nlcolor.get_dvid().dvid;
  auto da1 = a1.get_dvid().dvid;
  auto da2 = a2.get_dvid().dvid;
  auto da3 = a3.get_dvid().dvid;
  auto da4 = a4.get_dvid().dvid;
  auto da5 = a5.get_dvid().dvid;
  auto da6 = a6.get_dvid().dvid;
  bcast_rpc_oneway(split_context_execution_helper6<F,T1,T2,T3,T4,T5,T6>,
                   dcolor, f, da1, da2, da3, da4, da5, da6);
}

template <class T1, class T2, class T3, class T4, class T5, class T6>
void split_context_execution(const std::vector<int>& color,
                             void(*fp)(node_local<T1>&,
                                       node_local<T2>&,
                                       node_local<T3>&,
                                       node_local<T4>&,
                                       node_local<T5>&,
                                       node_local<T6>&),
                             node_local<T1>& a1,
                             node_local<T2>& a2,
                             node_local<T3>& a3,
                             node_local<T4>& a4,
                             node_local<T5>& a5,
                             node_local<T6>& a6) {
  split_context_execution(color, make_serfunc(fp), a1, a2, a3, a4, a5, a6);
}

// -- 7
template <class F, class T1, class T2, class T3, class T4, class T5, class T6,
          class T7>
void split_context_execution_helper7(dvid_t& dcolor, F& f, dvid_t& a1,
                                     dvid_t& a2, dvid_t& a3, dvid_t& a4,
                                     dvid_t& a5, dvid_t& a6, dvid_t& a7) {
  auto a1p = get_data<T1>(a1);
  auto a2p = get_data<T2>(a2);
  auto a3p = get_data<T3>(a3);
  auto a4p = get_data<T4>(a4);
  auto a5p = get_data<T5>(a5);
  auto a6p = get_data<T6>(a6);
  auto a7p = get_data<T7>(a7);
  int color = *get_data<int>(dcolor);
  init_split_context_execution(color);
  auto dvid1 = get_new_dvid(); 
  dvid_table[dvid1] = reinterpret_cast<intptr_t>(a1p);
  auto dvid2 = get_new_dvid(); 
  dvid_table[dvid2] = reinterpret_cast<intptr_t>(a2p);
  auto dvid3 = get_new_dvid(); 
  dvid_table[dvid3] = reinterpret_cast<intptr_t>(a3p);
  auto dvid4 = get_new_dvid(); 
  dvid_table[dvid4] = reinterpret_cast<intptr_t>(a4p);
  auto dvid5 = get_new_dvid(); 
  dvid_table[dvid5] = reinterpret_cast<intptr_t>(a5p);
  auto dvid6 = get_new_dvid(); 
  dvid_table[dvid6] = reinterpret_cast<intptr_t>(a6p);
  auto dvid7 = get_new_dvid(); 
  dvid_table[dvid7] = reinterpret_cast<intptr_t>(a7p);
  bool exception_caught = false;
  std::string what;
  try {
    if(frovedis_self_rank != 0) {
      while (handle_one_bcast_req());
    } else {
      auto na1 = node_local<T1>(DVID<T1>(dvid1),true);
      auto na2 = node_local<T2>(DVID<T2>(dvid2),true);
      auto na3 = node_local<T3>(DVID<T3>(dvid3),true);
      auto na4 = node_local<T4>(DVID<T4>(dvid4),true);
      auto na5 = node_local<T5>(DVID<T5>(dvid5),true);
      auto na6 = node_local<T6>(DVID<T6>(dvid6),true);
      auto na7 = node_local<T7>(DVID<T7>(dvid7),true);
      f(na1, na2, na3, na4, na5, na6, na7);
    }
  } catch (std::exception& e) {
    exception_caught = true;
    what = e.what();
  }
  finalize_split_context_execution();
  if(exception_caught) throw std::runtime_error(what);
}

template <class F, class T1, class T2, class T3, class T4, class T5, class T6,
          class T7>
void split_context_execution(const std::vector<int>& color, const F& f,
                             node_local<T1>& a1, node_local<T2>& a2,
                             node_local<T3>& a3, node_local<T4>& a4,
                             node_local<T5>& a5, node_local<T6>& a6,
                             node_local<T7>& a7) {
  if(color.size() != get_nodesize())
    throw std::runtime_error("size of color is different from node size");
  auto nlcolor = make_node_local_scatter(color);
  auto dcolor = nlcolor.get_dvid().dvid;
  auto da1 = a1.get_dvid().dvid;
  auto da2 = a2.get_dvid().dvid;
  auto da3 = a3.get_dvid().dvid;
  auto da4 = a4.get_dvid().dvid;
  auto da5 = a5.get_dvid().dvid;
  auto da6 = a6.get_dvid().dvid;
  auto da7 = a7.get_dvid().dvid;
  bcast_rpc_oneway(split_context_execution_helper7<F,T1,T2,T3,T4,T5,T6,T7>,
                   dcolor, f, da1, da2, da3, da4, da5, da6, da7);
}

template <class T1, class T2, class T3, class T4, class T5, class T6, class T7>
void split_context_execution(const std::vector<int>& color,
                             void(*fp)(node_local<T1>&,
                                       node_local<T2>&,
                                       node_local<T3>&,
                                       node_local<T4>&,
                                       node_local<T5>&,
                                       node_local<T6>&,
                                       node_local<T7>&),
                             node_local<T1>& a1,
                             node_local<T2>& a2,
                             node_local<T3>& a3,
                             node_local<T4>& a4,
                             node_local<T5>& a5,
                             node_local<T6>& a6,
                             node_local<T7>& a7) {
  split_context_execution(color, make_serfunc(fp), a1, a2, a3, a4, a5, a6, a7);
}

// -- 8
template <class F, class T1, class T2, class T3, class T4, class T5, class T6,
          class T7, class T8>
void split_context_execution_helper8(dvid_t& dcolor, F& f, dvid_t& a1,
                                     dvid_t& a2, dvid_t& a3, dvid_t& a4,
                                     dvid_t& a5, dvid_t& a6, dvid_t& a7,
                                     dvid_t& a8) {
  auto a1p = get_data<T1>(a1);
  auto a2p = get_data<T2>(a2);
  auto a3p = get_data<T3>(a3);
  auto a4p = get_data<T4>(a4);
  auto a5p = get_data<T5>(a5);
  auto a6p = get_data<T6>(a6);
  auto a7p = get_data<T7>(a7);
  auto a8p = get_data<T8>(a8);
  int color = *get_data<int>(dcolor);
  init_split_context_execution(color);
  auto dvid1 = get_new_dvid(); 
  dvid_table[dvid1] = reinterpret_cast<intptr_t>(a1p);
  auto dvid2 = get_new_dvid(); 
  dvid_table[dvid2] = reinterpret_cast<intptr_t>(a2p);
  auto dvid3 = get_new_dvid(); 
  dvid_table[dvid3] = reinterpret_cast<intptr_t>(a3p);
  auto dvid4 = get_new_dvid(); 
  dvid_table[dvid4] = reinterpret_cast<intptr_t>(a4p);
  auto dvid5 = get_new_dvid(); 
  dvid_table[dvid5] = reinterpret_cast<intptr_t>(a5p);
  auto dvid6 = get_new_dvid(); 
  dvid_table[dvid6] = reinterpret_cast<intptr_t>(a6p);
  auto dvid7 = get_new_dvid(); 
  dvid_table[dvid7] = reinterpret_cast<intptr_t>(a7p);
  auto dvid8 = get_new_dvid(); 
  dvid_table[dvid8] = reinterpret_cast<intptr_t>(a8p);
  bool exception_caught = false;
  std::string what;
  try {
    if(frovedis_self_rank != 0) {
      while (handle_one_bcast_req());
    } else {
      auto na1 = node_local<T1>(DVID<T1>(dvid1),true);
      auto na2 = node_local<T2>(DVID<T2>(dvid2),true);
      auto na3 = node_local<T3>(DVID<T3>(dvid3),true);
      auto na4 = node_local<T4>(DVID<T4>(dvid4),true);
      auto na5 = node_local<T5>(DVID<T5>(dvid5),true);
      auto na6 = node_local<T6>(DVID<T6>(dvid6),true);
      auto na7 = node_local<T7>(DVID<T7>(dvid7),true);
      auto na8 = node_local<T8>(DVID<T8>(dvid8),true);
      f(na1, na2, na3, na4, na5, na6, na7, na8);
    }
  } catch (std::exception& e) {
    exception_caught = true;
    what = e.what();
  }
  finalize_split_context_execution();
  if(exception_caught) throw std::runtime_error(what);
}

template <class F, class T1, class T2, class T3, class T4, class T5, class T6,
          class T7, class T8>
void split_context_execution(const std::vector<int>& color, const F& f,
                             node_local<T1>& a1, node_local<T2>& a2,
                             node_local<T3>& a3, node_local<T4>& a4,
                             node_local<T5>& a5, node_local<T6>& a6,
                             node_local<T7>& a7, node_local<T8>& a8) {
  if(color.size() != get_nodesize())
    throw std::runtime_error("size of color is different from node size");
  auto nlcolor = make_node_local_scatter(color);
  auto dcolor = nlcolor.get_dvid().dvid;
  auto da1 = a1.get_dvid().dvid;
  auto da2 = a2.get_dvid().dvid;
  auto da3 = a3.get_dvid().dvid;
  auto da4 = a4.get_dvid().dvid;
  auto da5 = a5.get_dvid().dvid;
  auto da6 = a6.get_dvid().dvid;
  auto da7 = a7.get_dvid().dvid;
  auto da8 = a8.get_dvid().dvid;
  bcast_rpc_oneway(split_context_execution_helper8<F,T1,T2,T3,T4,T5,T6,T7,T8>,
                   dcolor, f, da1, da2, da3, da4, da5, da6, da7, da8);
}

template <class T1, class T2, class T3, class T4, class T5, class T6, class T7,
          class T8>
void split_context_execution(const std::vector<int>& color,
                             void(*fp)(node_local<T1>&,
                                       node_local<T2>&,
                                       node_local<T3>&,
                                       node_local<T4>&,
                                       node_local<T5>&,
                                       node_local<T6>&,
                                       node_local<T7>&,
                                       node_local<T8>&),
                             node_local<T1>& a1,
                             node_local<T2>& a2,
                             node_local<T3>& a3,
                             node_local<T4>& a4,
                             node_local<T5>& a5,
                             node_local<T6>& a6,
                             node_local<T7>& a7,
                             node_local<T8>& a8) {
  split_context_execution(color, make_serfunc(fp), a1, a2, a3, a4, a5, a6, a7,
                          a8);
}

// ---------- wrappers w/o color 
// -- 0
template <class F>
void split_context_execution(const F& f) {
  auto nodesize = get_nodesize();
  std::vector<int> color(nodesize);
  auto colorp = color.data();
  for(size_t i = 0; i < nodesize; i++) colorp[i] = i;
  split_context_execution(color, f);
}

void split_context_execution(void(*fp)());

// -- 1
template <class F, class T1>
void split_context_execution(const F& f, node_local<T1>& a1) {
  auto nodesize = get_nodesize();
  std::vector<int> color(nodesize);
  auto colorp = color.data();
  for(size_t i = 0; i < nodesize; i++) colorp[i] = i;
  split_context_execution(color, f, a1);
}

template <class T1>
void split_context_execution(void(*fp)(node_local<T1>&),
                             node_local<T1>& a1) {
  split_context_execution(make_serfunc(fp), a1);
}

// -- 2
template <class F, class T1, class T2>
void split_context_execution(const F& f, node_local<T1>& a1,
                             node_local<T2>& a2) {
  auto nodesize = get_nodesize();
  std::vector<int> color(nodesize);
  auto colorp = color.data();
  for(size_t i = 0; i < nodesize; i++) colorp[i] = i;
  split_context_execution(color, f, a1, a2);
}

template <class T1, class T2>
void split_context_execution(void(*fp)(node_local<T1>&,
                                       node_local<T2>&),
                             node_local<T1>& a1,
                             node_local<T2>& a2) {
  split_context_execution(make_serfunc(fp), a1, a2);
}

// -- 3
template <class F, class T1, class T2, class T3>
void split_context_execution(const F& f, node_local<T1>& a1,
                             node_local<T2>& a2, node_local<T3>& a3) {
  auto nodesize = get_nodesize();
  std::vector<int> color(nodesize);
  auto colorp = color.data();
  for(size_t i = 0; i < nodesize; i++) colorp[i] = i;
  split_context_execution(color, f, a1, a2, a3);
}

template <class T1, class T2, class T3>
void split_context_execution(void(*fp)(node_local<T1>&,
                                       node_local<T2>&,
                                       node_local<T3>&),
                             node_local<T1>& a1,
                             node_local<T2>& a2,
                             node_local<T3>& a3) {
  split_context_execution(make_serfunc(fp), a1, a2, a3);
}

// -- 4
template <class F, class T1, class T2, class T3, class T4>
void split_context_execution(const F& f, node_local<T1>& a1,
                             node_local<T2>& a2, node_local<T3>& a3,
                             node_local<T4>& a4) {
  auto nodesize = get_nodesize();
  std::vector<int> color(nodesize);
  auto colorp = color.data();
  for(size_t i = 0; i < nodesize; i++) colorp[i] = i;
  split_context_execution(color, f, a1, a2, a3, a4);
}

template <class T1, class T2, class T3, class T4>
void split_context_execution(void(*fp)(node_local<T1>&,
                                       node_local<T2>&,
                                       node_local<T3>&,
                                       node_local<T4>&),
                             node_local<T1>& a1,
                             node_local<T2>& a2,
                             node_local<T3>& a3,
                             node_local<T4>& a4) {
  split_context_execution(make_serfunc(fp), a1, a2, a3, a4);
}

// -- 5
template <class F, class T1, class T2, class T3, class T4, class T5>
void split_context_execution(const F& f, node_local<T1>& a1,
                             node_local<T2>& a2, node_local<T3>& a3,
                             node_local<T4>& a4, node_local<T5>& a5) {
  auto nodesize = get_nodesize();
  std::vector<int> color(nodesize);
  auto colorp = color.data();
  for(size_t i = 0; i < nodesize; i++) colorp[i] = i;
  split_context_execution(color, f, a1, a2, a3, a4, a5);
}

template <class T1, class T2, class T3, class T4, class T5>
void split_context_execution(void(*fp)(node_local<T1>&,
                                       node_local<T2>&,
                                       node_local<T3>&,
                                       node_local<T4>&,
                                       node_local<T5>&),
                             node_local<T1>& a1,
                             node_local<T2>& a2,
                             node_local<T3>& a3,
                             node_local<T4>& a4,
                             node_local<T5>& a5) {
  split_context_execution(make_serfunc(fp), a1, a2, a3, a4, a5);
}

// -- 6
template <class F, class T1, class T2, class T3, class T4, class T5, class T6>
void split_context_execution(const F& f, node_local<T1>& a1,
                             node_local<T2>& a2, node_local<T3>& a3,
                             node_local<T4>& a4, node_local<T5>& a5,
                             node_local<T6>& a6) {
  auto nodesize = get_nodesize();
  std::vector<int> color(nodesize);
  auto colorp = color.data();
  for(size_t i = 0; i < nodesize; i++) colorp[i] = i;
  split_context_execution(color, f, a1, a2, a3, a4, a5, a6);
}

template <class T1, class T2, class T3, class T4, class T5, class T6>
void split_context_execution(void(*fp)(node_local<T1>&,
                                       node_local<T2>&,
                                       node_local<T3>&,
                                       node_local<T4>&,
                                       node_local<T5>&,
                                       node_local<T6>&),
                             node_local<T1>& a1,
                             node_local<T2>& a2,
                             node_local<T3>& a3,
                             node_local<T4>& a4,
                             node_local<T5>& a5,
                             node_local<T6>& a6) {
  split_context_execution(make_serfunc(fp), a1, a2, a3, a4, a5, a6);
}

// -- 7
template <class F, class T1, class T2, class T3, class T4, class T5, class T6,
          class T7>
void split_context_execution(const F& f, node_local<T1>& a1,
                             node_local<T2>& a2, node_local<T3>& a3,
                             node_local<T4>& a4, node_local<T5>& a5,
                             node_local<T6>& a6, node_local<T7>& a7) {
  auto nodesize = get_nodesize();
  std::vector<int> color(nodesize);
  auto colorp = color.data();
  for(size_t i = 0; i < nodesize; i++) colorp[i] = i;
  split_context_execution(color, f, a1, a2, a3, a4, a5, a6, a7);
}

template <class T1, class T2, class T3, class T4, class T5, class T6, class T7>
void split_context_execution(void(*fp)(node_local<T1>&,
                                       node_local<T2>&,
                                       node_local<T3>&,
                                       node_local<T4>&,
                                       node_local<T5>&,
                                       node_local<T6>&,
                                       node_local<T7>&),
                             node_local<T1>& a1,
                             node_local<T2>& a2,
                             node_local<T3>& a3,
                             node_local<T4>& a4,
                             node_local<T5>& a5,
                             node_local<T6>& a6,
                             node_local<T7>& a7) {
  split_context_execution(make_serfunc(fp), a1, a2, a3, a4, a5, a6, a7);
}

// -- 8
template <class F, class T1, class T2, class T3, class T4, class T5, class T6,
          class T7, class T8>
void split_context_execution(const F& f, node_local<T1>& a1,
                             node_local<T2>& a2, node_local<T3>& a3,
                             node_local<T4>& a4, node_local<T5>& a5,
                             node_local<T6>& a6, node_local<T7>& a7,
                             node_local<T8>& a8) {
  auto nodesize = get_nodesize();
  std::vector<int> color(nodesize);
  auto colorp = color.data();
  for(size_t i = 0; i < nodesize; i++) colorp[i] = i;
  split_context_execution(color, f, a1, a2, a3, a4, a5, a6, a7, a8);
}

template <class T1, class T2, class T3, class T4, class T5, class T6, class T7,
          class T8>
void split_context_execution(void(*fp)(node_local<T1>&,
                                       node_local<T2>&,
                                       node_local<T3>&,
                                       node_local<T4>&,
                                       node_local<T5>&,
                                       node_local<T6>&,
                                       node_local<T7>&,
                                       node_local<T8>&),
                             node_local<T1>& a1,
                             node_local<T2>& a2,
                             node_local<T3>& a3,
                             node_local<T4>& a4,
                             node_local<T5>& a5,
                             node_local<T6>& a6,
                             node_local<T7>& a7,
                             node_local<T8>& a8) {
  split_context_execution(make_serfunc(fp), a1, a2, a3, a4, a5, a6, a7, a8);
}

// -- split_context_broadcast

template <class T>
void split_context_broadcast_helper(node_local<T>& nl) {
  if(get_split_rank_stack()[0] == 0) {
    auto bcast = broadcast(nl.get(0));
    bcast.mapv(+[](T& src, T& dst) {std::swap(src, dst);}, nl);
  }
}

template <class T>
node_local<T>
split_context_broadcast(const std::vector<int>& color, const T& arg) {
  auto color_size = color.size();
  if(color_size != get_nodesize())
    throw std::runtime_error("size of color is different from node size");
  std::vector<int> tmp_color(color_size), idx(color_size);
  auto tmp_colorp = tmp_color.data();
  auto idxp = idx.data();
  auto colorp = color.data();
  for(size_t i = 0; i < color_size; i++) {
    tmp_colorp[i] = colorp[i];
    idxp[i] = i;
  }
  radix_sort(tmp_colorp, idxp, color_size, true);
  auto sep = set_separate(tmp_color);
  auto sepp = sep.data();
  auto sep_size = sep.size();
  auto new_root_size = sep_size - 1;
  std::vector<int> new_color(color_size);
  auto new_colorp = new_color.data();
  for(size_t i = 0; i < color_size; i++) {
    new_colorp[i] = 1;
  }
  for(size_t i = 0; i < new_root_size; i++) {
    new_colorp[idxp[sepp[i]]] = 0;
  }
  auto nl = make_node_local_allocate<T>();
  nl.put(0, arg);
  split_context_execution(new_color,
                          make_serfunc(split_context_broadcast_helper<T>), nl);
  return nl;
}

// -- split_context_scatter

template <class T>
void split_context_scatter_helper(node_local<std::vector<T>>& nl,
                                  node_local<T>& r) {
  if(get_split_rank_stack()[0] == 0) {
    auto sc = make_node_local_scatter(nl.get(0));
    sc.mapv(+[](T& src, T& dst) {std::swap(src, dst);}, r);
  }
}

template <class T>
node_local<T> split_context_scatter(const std::vector<int>& color,
                                    const std::vector<T>& arg) {
  auto color_size = color.size();
  if(color_size != get_nodesize())
    throw std::runtime_error("size of color is different from node size");
  std::vector<int> tmp_color(color_size), idx(color_size);
  auto tmp_colorp = tmp_color.data();
  auto idxp = idx.data();
  auto colorp = color.data();
  for(size_t i = 0; i < color_size; i++) {
    tmp_colorp[i] = colorp[i];
    idxp[i] = i;
  }
  radix_sort(tmp_colorp, idxp, color_size, true);
  auto sep = set_separate(tmp_color);
  auto sepp = sep.data();
  auto sep_size = sep.size();
  auto new_root_size = sep_size - 1;
  if(new_root_size != arg.size()) 
    throw std::runtime_error("size of new root is different from argument");
  std::vector<int> new_color(color_size);
  auto new_colorp = new_color.data();
  for(size_t i = 0; i < color_size; i++) {
    new_colorp[i] = 1;
  }
  for(size_t i = 0; i < new_root_size; i++) {
    new_colorp[idxp[sepp[i]]] = 0;
  }
  auto nl = make_node_local_allocate<std::vector<T>>();
  nl.put(0, arg);
  auto r = make_node_local_allocate<T>();
  split_context_execution
    (new_color, make_serfunc(split_context_scatter_helper<T>), nl, r);
  return r;
}

// -- split_context_gather

template <class T>
void split_context_gather_helper(node_local<T>& nl,
                                 node_local<std::vector<T>>& r) {
  if(get_split_rank_stack()[0] == 0) {
    r.put(0, nl.gather());
  }
}

template <class T>
const std::vector<T> split_context_gather(const std::vector<int>& color,
                                          node_local<T>& to_gather) {
  auto color_size = color.size();
  if(color_size != get_nodesize())
    throw std::runtime_error("size of color is different from node size");
  std::vector<int> tmp_color(color_size), idx(color_size);
  auto tmp_colorp = tmp_color.data();
  auto idxp = idx.data();
  auto colorp = color.data();
  for(size_t i = 0; i < color_size; i++) {
    tmp_colorp[i] = colorp[i];
    idxp[i] = i;
  }
  radix_sort(tmp_colorp, idxp, color_size, true);
  auto sep = set_separate(tmp_color);
  auto sepp = sep.data();
  auto sep_size = sep.size();
  auto new_root_size = sep_size - 1;
  std::vector<int> new_color(color_size);
  auto new_colorp = new_color.data();
  for(size_t i = 0; i < color_size; i++) {
    new_colorp[i] = 1;
  }
  for(size_t i = 0; i < new_root_size; i++) {
    new_colorp[idxp[sepp[i]]] = 0;
  }
  auto r = make_node_local_allocate<std::vector<T>>();
  split_context_execution
    (new_color, make_serfunc(split_context_gather_helper<T>), to_gather, r);
  return r.get(0);
}

std::vector<int> make_color(int size);

}
#endif
