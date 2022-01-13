#ifndef DFTABLE_TO_DVECTOR_HPP
#define DFTABLE_TO_DVECTOR_HPP

#include "dftable.hpp"
#include "../core/tuple_serializer.hpp"

namespace frovedis {

template <class T0>
dvector<T0> dftable_to_dvector(dftable_base& table) {
  auto cols = table.columns();
  use_dfcolumn use(cols, table);
  if(cols.size() != 1) throw std::runtime_error("size of column is not 1");
  auto col0 = table.column(cols[0]);
  auto typed_col0 = std::dynamic_pointer_cast<typed_dfcolumn<T0>>(col0);
  if(!typed_col0)
    throw std::runtime_error
      ("dftable_to_dvector: column type is different from specified type");
  auto&& val0 = typed_col0->get_val();
  auto ret = val0.template as_dvector<T0>(); 
  return ret;
}

template <class T0, class T1>
void dftable_to_dvector_helper2(std::vector<std::tuple<T0,T1>>& v,
                                std::vector<T0>& val0,
                                std::vector<T1>& val1) {
  v.resize(val0.size());
  for(size_t i = 0; i < v.size(); i++) {
    std::get<0>(v[i]) = val0[i];
    std::get<1>(v[i]) = val1[i];
  }
}

template <class T0, class T1>
dvector<std::tuple<T0,T1>> dftable_to_dvector(dftable_base& table) {
  auto ret = make_node_local_allocate<std::vector<std::tuple<T0,T1>>>();
  auto cols = table.columns();
  use_dfcolumn use(cols, table);
  if(cols.size() != 2) throw std::runtime_error("size of column is not 2");
  auto col0 = table.column(cols[0]);
  auto col1 = table.column(cols[1]);
  auto typed_col0 = std::dynamic_pointer_cast<typed_dfcolumn<T0>>(col0);
  auto typed_col1 = std::dynamic_pointer_cast<typed_dfcolumn<T1>>(col1);
  if(!static_cast<bool>(typed_col0) || !static_cast<bool>(typed_col1))
    throw std::runtime_error
      ("dftable_to_dvector: column types are different from specified types");
  auto&& val0 = typed_col0->get_val();
  auto&& val1 = typed_col1->get_val();
  ret.mapv(dftable_to_dvector_helper2<T0,T1>, val0, val1);
  return ret.template moveto_dvector<std::tuple<T0,T1>>();
}

template <class T0, class T1, class T2>
void dftable_to_dvector_helper3(std::vector<std::tuple<T0,T1,T2>>& v,
                                std::vector<T0>& val0,
                                std::vector<T1>& val1,
                                std::vector<T2>& val2) {
  v.resize(val0.size());
  for(size_t i = 0; i < v.size(); i++) {
    std::get<0>(v[i]) = val0[i];
    std::get<1>(v[i]) = val1[i];
    std::get<2>(v[i]) = val2[i];
  }
}

template <class T0, class T1, class T2>
dvector<std::tuple<T0,T1,T2>> dftable_to_dvector(dftable_base& table) {
  auto ret = make_node_local_allocate<std::vector<std::tuple<T0,T1,T2>>>();
  auto cols = table.columns();
  use_dfcolumn use(cols, table);
  if(cols.size() != 3) throw std::runtime_error("size of column is not 3");
  auto col0 = table.column(cols[0]);
  auto col1 = table.column(cols[1]);
  auto col2 = table.column(cols[2]);
  auto typed_col0 = std::dynamic_pointer_cast<typed_dfcolumn<T0>>(col0);
  auto typed_col1 = std::dynamic_pointer_cast<typed_dfcolumn<T1>>(col1);
  auto typed_col2 = std::dynamic_pointer_cast<typed_dfcolumn<T2>>(col2);
  if(!static_cast<bool>(typed_col0) || !static_cast<bool>(typed_col1) ||
     !static_cast<bool>(typed_col2))
    throw std::runtime_error
      ("dftable_to_dvector: column types are different from specified types");
  auto&& val0 = typed_col0->get_val();
  auto&& val1 = typed_col1->get_val();
  auto&& val2 = typed_col2->get_val();
  ret.mapv(dftable_to_dvector_helper3<T0,T1,T2>, val0, val1, val2);
  return ret.template moveto_dvector<std::tuple<T0,T1,T2>>();
}

template <class T0, class T1, class T2, class T3>
void dftable_to_dvector_helper4(std::vector<std::tuple<T0,T1,T2,T3>>& v,
                                std::vector<T0>& val0,
                                std::vector<T1>& val1,
                                std::vector<T2>& val2,
                                std::vector<T3>& val3) {
  v.resize(val0.size());
  for(size_t i = 0; i < v.size(); i++) {
    std::get<0>(v[i]) = val0[i];
    std::get<1>(v[i]) = val1[i];
    std::get<2>(v[i]) = val2[i];
    std::get<3>(v[i]) = val3[i];
  }
}

template <class T0, class T1, class T2, class T3>
dvector<std::tuple<T0,T1,T2,T3>> dftable_to_dvector(dftable_base& table) {
  auto ret = make_node_local_allocate<std::vector<std::tuple<T0,T1,T2,T3>>>();
  auto cols = table.columns();
  use_dfcolumn use(cols, table);
  if(cols.size() != 4) throw std::runtime_error("size of column is not 4");
  auto col0 = table.column(cols[0]);
  auto col1 = table.column(cols[1]);
  auto col2 = table.column(cols[2]);
  auto col3 = table.column(cols[3]);
  auto typed_col0 = std::dynamic_pointer_cast<typed_dfcolumn<T0>>(col0);
  auto typed_col1 = std::dynamic_pointer_cast<typed_dfcolumn<T1>>(col1);
  auto typed_col2 = std::dynamic_pointer_cast<typed_dfcolumn<T2>>(col2);
  auto typed_col3 = std::dynamic_pointer_cast<typed_dfcolumn<T3>>(col3);
  if(!static_cast<bool>(typed_col0) || !static_cast<bool>(typed_col1) ||
     !static_cast<bool>(typed_col2) || !static_cast<bool>(typed_col3))
    throw std::runtime_error
      ("dftable_to_dvector: column types are different from specified types");
  auto&& val0 = typed_col0->get_val();
  auto&& val1 = typed_col1->get_val();
  auto&& val2 = typed_col2->get_val();
  auto&& val3 = typed_col3->get_val();
  ret.mapv(dftable_to_dvector_helper4<T0,T1,T2,T3>, val0, val1, val2, val3);
  return ret.template moveto_dvector<std::tuple<T0,T1,T2,T3>>();
}

template <class T0, class T1, class T2, class T3, class T4>
void dftable_to_dvector_helper5(std::vector<std::tuple<T0,T1,T2,T3,T4>>& v,
                                std::vector<T0>& val0,
                                std::vector<T1>& val1,
                                std::vector<T2>& val2,
                                std::vector<T3>& val3,
                                std::vector<T4>& val4) {
  v.resize(val0.size());
  for(size_t i = 0; i < v.size(); i++) {
    std::get<0>(v[i]) = val0[i];
    std::get<1>(v[i]) = val1[i];
    std::get<2>(v[i]) = val2[i];
    std::get<3>(v[i]) = val3[i];
    std::get<4>(v[i]) = val4[i];
  }
}

template <class T0, class T1, class T2, class T3, class T4>
dvector<std::tuple<T0,T1,T2,T3,T4>> dftable_to_dvector(dftable_base& table) {
  auto ret =
    make_node_local_allocate<std::vector<std::tuple<T0,T1,T2,T3,T4>>>();
  auto cols = table.columns();
  use_dfcolumn use(cols, table);
  if(cols.size() != 5) throw std::runtime_error("size of column is not 5");
  auto col0 = table.column(cols[0]);
  auto col1 = table.column(cols[1]);
  auto col2 = table.column(cols[2]);
  auto col3 = table.column(cols[3]);
  auto col4 = table.column(cols[4]);
  auto typed_col0 = std::dynamic_pointer_cast<typed_dfcolumn<T0>>(col0);
  auto typed_col1 = std::dynamic_pointer_cast<typed_dfcolumn<T1>>(col1);
  auto typed_col2 = std::dynamic_pointer_cast<typed_dfcolumn<T2>>(col2);
  auto typed_col3 = std::dynamic_pointer_cast<typed_dfcolumn<T3>>(col3);
  auto typed_col4 = std::dynamic_pointer_cast<typed_dfcolumn<T4>>(col4);
  if(!static_cast<bool>(typed_col0) || !static_cast<bool>(typed_col1) ||
     !static_cast<bool>(typed_col2) || !static_cast<bool>(typed_col3) ||
     !static_cast<bool>(typed_col4))
    throw std::runtime_error
      ("dftable_to_dvector: column types are different from specified types");
  auto&& val0 = typed_col0->get_val();
  auto&& val1 = typed_col1->get_val();
  auto&& val2 = typed_col2->get_val();
  auto&& val3 = typed_col3->get_val();
  auto&& val4 = typed_col4->get_val();
  ret.mapv(dftable_to_dvector_helper5<T0,T1,T2,T3,T4>,
           val0, val1, val2, val3, val4);
  return ret.template moveto_dvector<std::tuple<T0,T1,T2,T3,T4>>();
}

// number of arguments is limited, so separate into two functions
template <class T0, class T1, class T2, class T3, class T4, class T5>
void dftable_to_dvector_helper61(std::vector<std::tuple<T0,T1,T2,T3,T4,T5>>& v,
                                 std::vector<T0>& val0,
                                 std::vector<T1>& val1,
                                 std::vector<T2>& val2,
                                 std::vector<T3>& val3,
                                 std::vector<T4>& val4) {
  v.resize(val0.size());
  for(size_t i = 0; i < v.size(); i++) {
    std::get<0>(v[i]) = val0[i];
    std::get<1>(v[i]) = val1[i];
    std::get<2>(v[i]) = val2[i];
    std::get<3>(v[i]) = val3[i];
    std::get<4>(v[i]) = val4[i];
  }
}

template <class T0, class T1, class T2, class T3, class T4, class T5>
void dftable_to_dvector_helper62(std::vector<std::tuple<T0,T1,T2,T3,T4,T5>>& v,
                                 std::vector<T5>& val5) {
  for(size_t i = 0; i < v.size(); i++) {
    std::get<5>(v[i]) = val5[i];
  }
}

template <class T0, class T1, class T2, class T3, class T4, class T5>
dvector<std::tuple<T0,T1,T2,T3,T4,T5>>
  dftable_to_dvector(dftable_base& table) {
  auto ret =
    make_node_local_allocate<std::vector<std::tuple<T0,T1,T2,T3,T4,T5>>>();
  auto cols = table.columns();
  use_dfcolumn use(cols, table);
  if(cols.size() != 6) throw std::runtime_error("size of column is not 6");
  auto col0 = table.column(cols[0]);
  auto col1 = table.column(cols[1]);
  auto col2 = table.column(cols[2]);
  auto col3 = table.column(cols[3]);
  auto col4 = table.column(cols[4]);
  auto col5 = table.column(cols[5]);
  auto typed_col0 = std::dynamic_pointer_cast<typed_dfcolumn<T0>>(col0);
  auto typed_col1 = std::dynamic_pointer_cast<typed_dfcolumn<T1>>(col1);
  auto typed_col2 = std::dynamic_pointer_cast<typed_dfcolumn<T2>>(col2);
  auto typed_col3 = std::dynamic_pointer_cast<typed_dfcolumn<T3>>(col3);
  auto typed_col4 = std::dynamic_pointer_cast<typed_dfcolumn<T4>>(col4);
  auto typed_col5 = std::dynamic_pointer_cast<typed_dfcolumn<T5>>(col5);
  if(!static_cast<bool>(typed_col0) || !static_cast<bool>(typed_col1) ||
     !static_cast<bool>(typed_col2) || !static_cast<bool>(typed_col3) ||
     !static_cast<bool>(typed_col4) || !static_cast<bool>(typed_col5))
    throw std::runtime_error
      ("dftable_to_dvector: column types are different from specified types");
  auto&& val0 = typed_col0->get_val();
  auto&& val1 = typed_col1->get_val();
  auto&& val2 = typed_col2->get_val();
  auto&& val3 = typed_col3->get_val();
  auto&& val4 = typed_col4->get_val();
  auto&& val5 = typed_col5->get_val();
  ret.mapv(dftable_to_dvector_helper61<T0,T1,T2,T3,T4,T5>,
           val0, val1, val2, val3, val4);
  ret.mapv(dftable_to_dvector_helper62<T0,T1,T2,T3,T4,T5>, val5);
  return ret.template moveto_dvector<std::tuple<T0,T1,T2,T3,T4,T5>>();
}

template <class T0, class T1, class T2, class T3, class T4, class T5, class T6>
void dftable_to_dvector_helper71(std::vector
                                 <std::tuple<T0,T1,T2,T3,T4,T5,T6>>& v,
                                 std::vector<T0>& val0,
                                 std::vector<T1>& val1,
                                 std::vector<T2>& val2,
                                 std::vector<T3>& val3,
                                 std::vector<T4>& val4) {
  v.resize(val0.size());
  for(size_t i = 0; i < v.size(); i++) {
    std::get<0>(v[i]) = val0[i];
    std::get<1>(v[i]) = val1[i];
    std::get<2>(v[i]) = val2[i];
    std::get<3>(v[i]) = val3[i];
    std::get<4>(v[i]) = val4[i];
  }
}

template <class T0, class T1, class T2, class T3, class T4, class T5, class T6>
void dftable_to_dvector_helper72(std::vector
                                 <std::tuple<T0,T1,T2,T3,T4,T5,T6>>& v,
                                 std::vector<T5>& val5,
                                 std::vector<T6>& val6) {
  for(size_t i = 0; i < v.size(); i++) {
    std::get<5>(v[i]) = val5[i];
    std::get<6>(v[i]) = val6[i];
  }
}

template <class T0, class T1, class T2, class T3, class T4, class T5, class T6>
dvector<std::tuple<T0,T1,T2,T3,T4,T5,T6>>
  dftable_to_dvector(dftable_base& table) {
  auto ret =
    make_node_local_allocate<std::vector<std::tuple<T0,T1,T2,T3,T4,T5,T6>>>();
  auto cols = table.columns();
  use_dfcolumn use(cols, table);
  if(cols.size() != 7) throw std::runtime_error("size of column is not 7");
  auto col0 = table.column(cols[0]);
  auto col1 = table.column(cols[1]);
  auto col2 = table.column(cols[2]);
  auto col3 = table.column(cols[3]);
  auto col4 = table.column(cols[4]);
  auto col5 = table.column(cols[5]);
  auto col6 = table.column(cols[6]);
  auto typed_col0 = std::dynamic_pointer_cast<typed_dfcolumn<T0>>(col0);
  auto typed_col1 = std::dynamic_pointer_cast<typed_dfcolumn<T1>>(col1);
  auto typed_col2 = std::dynamic_pointer_cast<typed_dfcolumn<T2>>(col2);
  auto typed_col3 = std::dynamic_pointer_cast<typed_dfcolumn<T3>>(col3);
  auto typed_col4 = std::dynamic_pointer_cast<typed_dfcolumn<T4>>(col4);
  auto typed_col5 = std::dynamic_pointer_cast<typed_dfcolumn<T5>>(col5);
  auto typed_col6 = std::dynamic_pointer_cast<typed_dfcolumn<T6>>(col6);
  if(!static_cast<bool>(typed_col0) || !static_cast<bool>(typed_col1) ||
     !static_cast<bool>(typed_col2) || !static_cast<bool>(typed_col3) ||
     !static_cast<bool>(typed_col4) || !static_cast<bool>(typed_col5) ||
     !static_cast<bool>(typed_col6))
    throw std::runtime_error
      ("dftable_to_dvector: column types are different from specified types");
  auto&& val0 = typed_col0->get_val();
  auto&& val1 = typed_col1->get_val();
  auto&& val2 = typed_col2->get_val();
  auto&& val3 = typed_col3->get_val();
  auto&& val4 = typed_col4->get_val();
  auto&& val5 = typed_col5->get_val();
  auto&& val6 = typed_col6->get_val();
  ret.mapv(dftable_to_dvector_helper71<T0,T1,T2,T3,T4,T5,T6>,
           val0, val1, val2, val3, val4);
  ret.mapv(dftable_to_dvector_helper72<T0,T1,T2,T3,T4,T5,T6>, val5, val6);
  return ret.template moveto_dvector<std::tuple<T0,T1,T2,T3,T4,T5,T6>>();
}

}
#endif
