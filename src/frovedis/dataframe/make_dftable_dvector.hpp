#ifndef MAKE_DFTABLE_DVECTOR_HPP
#define MAKE_DFTABLE_DVECTOR_HPP

#include "dftable.hpp"
#include "../core/tuple_serializer.hpp"

namespace frovedis {

template <class T0, class T1>
void convert_tuple_helper2(std::vector<std::tuple<T0,T1>>& src,
                           std::tuple<std::vector<T0>,
                           std::vector<T1>>& dst) {
  size_t size = src.size();
  auto& dst0 = std::get<0>(dst);
  auto& dst1 = std::get<1>(dst);
  dst0.resize(size);
  dst1.resize(size);
  for(size_t i = 0; i < size; i++) {
    dst0[i] = std::get<0>(src[i]);
    dst1[i] = std::get<1>(src[i]);
  }
}

template <class T0, class T1>
std::vector<T0> get20(std::tuple<std::vector<T0>, std::vector<T1>>& src) {
  return std::get<0>(src);
}

template <class T0, class T1>
std::vector<T1> get21(std::tuple<std::vector<T0>, std::vector<T1>>& src) {
  return std::get<1>(src);
}

template <class T0, class T1>
std::tuple<dvector<T0>, dvector<T1>>
  convert_tuple(dvector<std::tuple<T0,T1>>& d) {
  auto tmp = make_node_local_allocate<std::tuple<std::vector<T0>,
                                                 std::vector<T1>>>();
  d.viewas_node_local().mapv(convert_tuple_helper2<T0,T1>, tmp);
  auto d0 = tmp.map(get20<T0,T1>).template moveto_dvector<T0>();
  auto d1 = tmp.map(get21<T0,T1>).template moveto_dvector<T1>();
  return std::make_tuple(d0, d1);
}

template <class T0, class T1>
dftable make_dftable_dvector(dvector<std::tuple<T0,T1>>& d,
                             std::vector<std::string> names) {
  auto dvs = convert_tuple(d);
  dftable ret;
  ret.append_column(names[0], std::get<0>(dvs));
  ret.append_column(names[1], std::get<1>(dvs));
  return ret;
}

template <class T0, class T1, class T2>
void convert_tuple_helper3(std::vector<std::tuple<T0,T1,T2>>& src,
                           std::tuple<std::vector<T0>,
                           std::vector<T1>,
                           std::vector<T2>>& dst) {
  size_t size = src.size();
  auto& dst0 = std::get<0>(dst);
  auto& dst1 = std::get<1>(dst);
  auto& dst2 = std::get<2>(dst);
  dst0.resize(size);
  dst1.resize(size);
  dst2.resize(size);
  for(size_t i = 0; i < size; i++) {
    dst0[i] = std::get<0>(src[i]);
    dst1[i] = std::get<1>(src[i]);
    dst2[i] = std::get<2>(src[i]);
  }
}

template <class T0, class T1, class T2>
std::vector<T0> get30(std::tuple<std::vector<T0>,
                      std::vector<T1>,
                      std::vector<T2>>& src) {
  return std::get<0>(src);
}

template <class T0, class T1, class T2>
std::vector<T1> get31(std::tuple<std::vector<T0>,
                      std::vector<T1>,
                      std::vector<T2>>& src) {
  return std::get<1>(src);
}

template <class T0, class T1, class T2>
std::vector<T2> get32(std::tuple<std::vector<T0>,
                      std::vector<T1>,
                      std::vector<T2>>& src) {
  return std::get<2>(src);
}

template <class T0, class T1, class T2>
std::tuple<dvector<T0>, dvector<T1>, dvector<T2>>
  convert_tuple(dvector<std::tuple<T0,T1,T2>>& d) {
  auto tmp = make_node_local_allocate<std::tuple<std::vector<T0>,
                                                 std::vector<T1>,
                                                 std::vector<T2>>>();
  d.viewas_node_local().mapv(convert_tuple_helper3<T0,T1,T2>, tmp);
  auto d0 = tmp.map(get30<T0,T1,T2>).template moveto_dvector<T0>();
  auto d1 = tmp.map(get31<T0,T1,T2>).template moveto_dvector<T1>();
  auto d2 = tmp.map(get32<T0,T1,T2>).template moveto_dvector<T2>();
  return std::make_tuple(d0, d1, d2);
}

template <class T0, class T1, class T2>
dftable make_dftable_dvector(dvector<std::tuple<T0,T1,T2>>& d,
                             std::vector<std::string> names) {
  auto dvs = convert_tuple(d);
  dftable ret;
  ret.append_column(names[0], std::get<0>(dvs));
  ret.append_column(names[1], std::get<1>(dvs));
  ret.append_column(names[2], std::get<2>(dvs));
  return ret;
}

template <class T0, class T1, class T2, class T3>
void convert_tuple_helper4(std::vector<std::tuple<T0,T1,T2,T3>>& src,
                           std::tuple<std::vector<T0>,
                           std::vector<T1>,
                           std::vector<T2>,
                           std::vector<T3>>& dst) {
  size_t size = src.size();
  auto& dst0 = std::get<0>(dst);
  auto& dst1 = std::get<1>(dst);
  auto& dst2 = std::get<2>(dst);
  auto& dst3 = std::get<3>(dst);
  dst0.resize(size);
  dst1.resize(size);
  dst2.resize(size);
  dst3.resize(size);
  for(size_t i = 0; i < size; i++) {
    dst0[i] = std::get<0>(src[i]);
    dst1[i] = std::get<1>(src[i]);
    dst2[i] = std::get<2>(src[i]);
    dst3[i] = std::get<3>(src[i]);
  }
}

template <class T0, class T1, class T2, class T3>
std::vector<T0> get40(std::tuple<std::vector<T0>,
                      std::vector<T1>,
                      std::vector<T2>,
                      std::vector<T3>>& src) {
  return std::get<0>(src);
}

template <class T0, class T1, class T2, class T3>
std::vector<T1> get41(std::tuple<std::vector<T0>,
                      std::vector<T1>,
                      std::vector<T2>,
                      std::vector<T3>>& src) {
  return std::get<1>(src);
}

template <class T0, class T1, class T2, class T3>
std::vector<T2> get42(std::tuple<std::vector<T0>,
                      std::vector<T1>,
                      std::vector<T2>,
                      std::vector<T3>>& src) {
  return std::get<2>(src);
}

template <class T0, class T1, class T2, class T3>
std::vector<T3> get43(std::tuple<std::vector<T0>,
                      std::vector<T1>,
                      std::vector<T2>,
                      std::vector<T3>>& src) {
  return std::get<3>(src);
}

template <class T0, class T1, class T2, class T3>
std::tuple<dvector<T0>, dvector<T1>, dvector<T2>, dvector<T3>>
  convert_tuple(dvector<std::tuple<T0,T1,T2,T3>>& d) {
  auto tmp = make_node_local_allocate<std::tuple<std::vector<T0>,
                                                 std::vector<T1>,
                                                 std::vector<T2>,
                                                 std::vector<T3>>>();
  d.viewas_node_local().mapv(convert_tuple_helper4<T0,T1,T2,T3>, tmp);
  auto d0 = tmp.map(get40<T0,T1,T2,T3>).template moveto_dvector<T0>();
  auto d1 = tmp.map(get41<T0,T1,T2,T3>).template moveto_dvector<T1>();
  auto d2 = tmp.map(get42<T0,T1,T2,T3>).template moveto_dvector<T2>();
  auto d3 = tmp.map(get43<T0,T1,T2,T3>).template moveto_dvector<T3>();
  return std::make_tuple(d0, d1, d2, d3);
}

template <class T0, class T1, class T2, class T3>
dftable make_dftable_dvector(dvector<std::tuple<T0,T1,T2,T3>>& d,
                             std::vector<std::string> names) {
  auto dvs = convert_tuple(d);
  dftable ret;
  ret.append_column(names[0], std::get<0>(dvs));
  ret.append_column(names[1], std::get<1>(dvs));
  ret.append_column(names[2], std::get<2>(dvs));
  ret.append_column(names[3], std::get<3>(dvs));
  return ret;
}

template <class T0, class T1, class T2, class T3, class T4>
void convert_tuple_helper5(std::vector<std::tuple<T0,T1,T2,T3,T4>>& src,
                           std::tuple<std::vector<T0>,
                           std::vector<T1>,
                           std::vector<T2>,
                           std::vector<T3>,
                           std::vector<T4>>& dst) {
  size_t size = src.size();
  auto& dst0 = std::get<0>(dst);
  auto& dst1 = std::get<1>(dst);
  auto& dst2 = std::get<2>(dst);
  auto& dst3 = std::get<3>(dst);
  auto& dst4 = std::get<4>(dst);
  dst0.resize(size);
  dst1.resize(size);
  dst2.resize(size);
  dst3.resize(size);
  dst4.resize(size);
  for(size_t i = 0; i < size; i++) {
    dst0[i] = std::get<0>(src[i]);
    dst1[i] = std::get<1>(src[i]);
    dst2[i] = std::get<2>(src[i]);
    dst3[i] = std::get<3>(src[i]);
    dst4[i] = std::get<4>(src[i]);
  }
}

template <class T0, class T1, class T2, class T3, class T4>
std::vector<T0> get50(std::tuple<std::vector<T0>,
                      std::vector<T1>,
                      std::vector<T2>,
                      std::vector<T3>,
                      std::vector<T4>>& src) {
  return std::get<0>(src);
}

template <class T0, class T1, class T2, class T3, class T4>
std::vector<T1> get51(std::tuple<std::vector<T0>,
                      std::vector<T1>,
                      std::vector<T2>,
                      std::vector<T3>,
                      std::vector<T4>>& src) {
  return std::get<1>(src);
}

template <class T0, class T1, class T2, class T3, class T4>
std::vector<T2> get52(std::tuple<std::vector<T0>,
                      std::vector<T1>,
                      std::vector<T2>,
                      std::vector<T3>,
                      std::vector<T4>>& src) {
  return std::get<2>(src);
}

template <class T0, class T1, class T2, class T3, class T4>
std::vector<T3> get53(std::tuple<std::vector<T0>,
                      std::vector<T1>,
                      std::vector<T2>,
                      std::vector<T3>,
                      std::vector<T4>>& src) {
  return std::get<3>(src);
}

template <class T0, class T1, class T2, class T3, class T4>
std::vector<T4> get54(std::tuple<std::vector<T0>,
                      std::vector<T1>,
                      std::vector<T2>,
                      std::vector<T3>,
                      std::vector<T4>>& src) {
  return std::get<4>(src);
}

template <class T0, class T1, class T2, class T3, class T4>
std::tuple<dvector<T0>, dvector<T1>, dvector<T2>, dvector<T3>, dvector<T4>>
  convert_tuple(dvector<std::tuple<T0,T1,T2,T3,T4>>& d) {
  auto tmp = make_node_local_allocate<std::tuple<std::vector<T0>,
                                                 std::vector<T1>,
                                                 std::vector<T2>,
                                                 std::vector<T3>,
                                                 std::vector<T4>>>();
  d.viewas_node_local().mapv(convert_tuple_helper5<T0,T1,T2,T3,T4>, tmp);
  auto d0 = tmp.map(get50<T0,T1,T2,T3,T4>).template moveto_dvector<T0>();
  auto d1 = tmp.map(get51<T0,T1,T2,T3,T4>).template moveto_dvector<T1>();
  auto d2 = tmp.map(get52<T0,T1,T2,T3,T4>).template moveto_dvector<T2>();
  auto d3 = tmp.map(get53<T0,T1,T2,T3,T4>).template moveto_dvector<T3>();
  auto d4 = tmp.map(get54<T0,T1,T2,T3,T4>).template moveto_dvector<T4>();
  return std::make_tuple(d0, d1, d2, d3, d4);
}

template <class T0, class T1, class T2, class T3, class T4>
dftable make_dftable_dvector(dvector<std::tuple<T0,T1,T2,T3,T4>>& d,
                             std::vector<std::string> names) {
  auto dvs = convert_tuple(d);
  dftable ret;
  ret.append_column(names[0], std::get<0>(dvs));
  ret.append_column(names[1], std::get<1>(dvs));
  ret.append_column(names[2], std::get<2>(dvs));
  ret.append_column(names[3], std::get<3>(dvs));
  ret.append_column(names[4], std::get<4>(dvs));
  return ret;
}

template <class T0, class T1, class T2, class T3, class T4, class T5>
void convert_tuple_helper6(std::vector<std::tuple<T0,T1,T2,T3,T4,T5>>& src,
                           std::tuple<std::vector<T0>,
                           std::vector<T1>,
                           std::vector<T2>,
                           std::vector<T3>,
                           std::vector<T4>,
                           std::vector<T5>>& dst) {
  size_t size = src.size();
  auto& dst0 = std::get<0>(dst);
  auto& dst1 = std::get<1>(dst);
  auto& dst2 = std::get<2>(dst);
  auto& dst3 = std::get<3>(dst);
  auto& dst4 = std::get<4>(dst);
  auto& dst5 = std::get<5>(dst);
  dst0.resize(size);
  dst1.resize(size);
  dst2.resize(size);
  dst3.resize(size);
  dst4.resize(size);
  dst5.resize(size);
  for(size_t i = 0; i < size; i++) {
    dst0[i] = std::get<0>(src[i]);
    dst1[i] = std::get<1>(src[i]);
    dst2[i] = std::get<2>(src[i]);
    dst3[i] = std::get<3>(src[i]);
    dst4[i] = std::get<4>(src[i]);
    dst5[i] = std::get<5>(src[i]);
  }
}

template <class T0, class T1, class T2, class T3, class T4, class T5>
std::vector<T0> get60(std::tuple<std::vector<T0>,
                      std::vector<T1>,
                      std::vector<T2>,
                      std::vector<T3>,
                      std::vector<T4>,
                      std::vector<T5>>& src) {
  return std::get<0>(src);
}

template <class T0, class T1, class T2, class T3, class T4, class T5>
std::vector<T1> get61(std::tuple<std::vector<T0>,
                      std::vector<T1>,
                      std::vector<T2>,
                      std::vector<T3>,
                      std::vector<T4>,
                      std::vector<T5>>& src) {
  return std::get<1>(src);
}

template <class T0, class T1, class T2, class T3, class T4, class T5>
std::vector<T2> get62(std::tuple<std::vector<T0>,
                      std::vector<T1>,
                      std::vector<T2>,
                      std::vector<T3>,
                      std::vector<T4>,
                      std::vector<T5>>& src) {
  return std::get<2>(src);
}

template <class T0, class T1, class T2, class T3, class T4, class T5>
std::vector<T3> get63(std::tuple<std::vector<T0>,
                      std::vector<T1>,
                      std::vector<T2>,
                      std::vector<T3>,
                      std::vector<T4>,
                      std::vector<T5>>& src) {
  return std::get<3>(src);
}

template <class T0, class T1, class T2, class T3, class T4, class T5>
std::vector<T4> get64(std::tuple<std::vector<T0>,
                      std::vector<T1>,
                      std::vector<T2>,
                      std::vector<T3>,
                      std::vector<T4>,
                      std::vector<T5>>& src) {
  return std::get<4>(src);
}

template <class T0, class T1, class T2, class T3, class T4, class T5>
std::vector<T5> get65(std::tuple<std::vector<T0>,
                      std::vector<T1>,
                      std::vector<T2>,
                      std::vector<T3>,
                      std::vector<T4>,
                      std::vector<T5>>& src) {
  return std::get<5>(src);
}

template <class T0, class T1, class T2, class T3, class T4, class T5>
std::tuple<dvector<T0>, dvector<T1>, dvector<T2>, dvector<T3>, dvector<T4>,
           dvector<T5>>
  convert_tuple(dvector<std::tuple<T0,T1,T2,T3,T4,T5>>& d) {
  auto tmp = make_node_local_allocate<std::tuple<std::vector<T0>,
                                                 std::vector<T1>,
                                                 std::vector<T2>,
                                                 std::vector<T3>,
                                                 std::vector<T4>,
                                                 std::vector<T5>>>();
  d.viewas_node_local().mapv(convert_tuple_helper6<T0,T1,T2,T3,T4,T5>, tmp);
  auto d0 = tmp.map(get60<T0,T1,T2,T3,T4,T5>).template moveto_dvector<T0>();
  auto d1 = tmp.map(get61<T0,T1,T2,T3,T4,T5>).template moveto_dvector<T1>();
  auto d2 = tmp.map(get62<T0,T1,T2,T3,T4,T5>).template moveto_dvector<T2>();
  auto d3 = tmp.map(get63<T0,T1,T2,T3,T4,T5>).template moveto_dvector<T3>();
  auto d4 = tmp.map(get64<T0,T1,T2,T3,T4,T5>).template moveto_dvector<T4>();
  auto d5 = tmp.map(get65<T0,T1,T2,T3,T4,T5>).template moveto_dvector<T5>();
  return std::make_tuple(d0, d1, d2, d3, d4, d5);
}

template <class T0, class T1, class T2, class T3, class T4, class T5>
dftable make_dftable_dvector(dvector<std::tuple<T0,T1,T2,T3,T4,T5>>& d,
                             std::vector<std::string> names) {
  auto dvs = convert_tuple(d);
  dftable ret;
  ret.append_column(names[0], std::get<0>(dvs));
  ret.append_column(names[1], std::get<1>(dvs));
  ret.append_column(names[2], std::get<2>(dvs));
  ret.append_column(names[3], std::get<3>(dvs));
  ret.append_column(names[4], std::get<4>(dvs));
  ret.append_column(names[5], std::get<5>(dvs));
  return ret;
}

template <class T0, class T1, class T2, class T3, class T4, class T5, class T6>
void convert_tuple_helper7(std::vector<std::tuple<T0,T1,T2,T3,T4,T5,T6>>& src,
                           std::tuple<std::vector<T0>,
                           std::vector<T1>,
                           std::vector<T2>,
                           std::vector<T3>,
                           std::vector<T4>,
                           std::vector<T5>,
                           std::vector<T6>>& dst) {
  size_t size = src.size();
  auto& dst0 = std::get<0>(dst);
  auto& dst1 = std::get<1>(dst);
  auto& dst2 = std::get<2>(dst);
  auto& dst3 = std::get<3>(dst);
  auto& dst4 = std::get<4>(dst);
  auto& dst5 = std::get<5>(dst);
  auto& dst6 = std::get<6>(dst);
  dst0.resize(size);
  dst1.resize(size);
  dst2.resize(size);
  dst3.resize(size);
  dst4.resize(size);
  dst5.resize(size);
  dst6.resize(size);
  for(size_t i = 0; i < size; i++) {
    dst0[i] = std::get<0>(src[i]);
    dst1[i] = std::get<1>(src[i]);
    dst2[i] = std::get<2>(src[i]);
    dst3[i] = std::get<3>(src[i]);
    dst4[i] = std::get<4>(src[i]);
    dst5[i] = std::get<5>(src[i]);
    dst6[i] = std::get<6>(src[i]);
  }
}

template <class T0, class T1, class T2, class T3, class T4, class T5, class T6>
std::vector<T0> get70(std::tuple<std::vector<T0>,
                      std::vector<T1>,
                      std::vector<T2>,
                      std::vector<T3>,
                      std::vector<T4>,
                      std::vector<T5>,
                      std::vector<T6>>& src) {
  return std::get<0>(src);
}

template <class T0, class T1, class T2, class T3, class T4, class T5, class T6>
std::vector<T1> get71(std::tuple<std::vector<T0>,
                      std::vector<T1>,
                      std::vector<T2>,
                      std::vector<T3>,
                      std::vector<T4>,
                      std::vector<T5>,
                      std::vector<T6>>& src) {
  return std::get<1>(src);
}

template <class T0, class T1, class T2, class T3, class T4, class T5, class T6>
std::vector<T2> get72(std::tuple<std::vector<T0>,
                      std::vector<T1>,
                      std::vector<T2>,
                      std::vector<T3>,
                      std::vector<T4>,
                      std::vector<T5>,
                      std::vector<T6>>& src) {
  return std::get<2>(src);
}

template <class T0, class T1, class T2, class T3, class T4, class T5, class T6>
std::vector<T3> get73(std::tuple<std::vector<T0>,
                      std::vector<T1>,
                      std::vector<T2>,
                      std::vector<T3>,
                      std::vector<T4>,
                      std::vector<T5>,
                      std::vector<T6>>& src) {
  return std::get<3>(src);
}

template <class T0, class T1, class T2, class T3, class T4, class T5, class T6>
std::vector<T4> get74(std::tuple<std::vector<T0>,
                      std::vector<T1>,
                      std::vector<T2>,
                      std::vector<T3>,
                      std::vector<T4>,
                      std::vector<T5>,
                      std::vector<T6>>& src) {
  return std::get<4>(src);
}

template <class T0, class T1, class T2, class T3, class T4, class T5, class T6>
std::vector<T5> get75(std::tuple<std::vector<T0>,
                      std::vector<T1>,
                      std::vector<T2>,
                      std::vector<T3>,
                      std::vector<T4>,
                      std::vector<T5>,
                      std::vector<T6>>& src) {
  return std::get<5>(src);
}

template <class T0, class T1, class T2, class T3, class T4, class T5, class T6>
std::vector<T6> get76(std::tuple<std::vector<T0>,
                      std::vector<T1>,
                      std::vector<T2>,
                      std::vector<T3>,
                      std::vector<T4>,
                      std::vector<T5>,
                      std::vector<T6>>& src) {
  return std::get<6>(src);
}

template <class T0, class T1, class T2, class T3, class T4, class T5, class T6>
std::tuple<dvector<T0>, dvector<T1>, dvector<T2>, dvector<T3>, dvector<T4>,
           dvector<T5>, dvector<T6>>
  convert_tuple(dvector<std::tuple<T0,T1,T2,T3,T4,T5,T6>>& d) {
  auto tmp = make_node_local_allocate<std::tuple<std::vector<T0>,
                                                 std::vector<T1>,
                                                 std::vector<T2>,
                                                 std::vector<T3>,
                                                 std::vector<T4>,
                                                 std::vector<T5>,
                                                 std::vector<T6>>>();
  d.viewas_node_local().mapv(convert_tuple_helper7<T0,T1,T2,T3,T4,T5,T6>, tmp);
  auto d0 = tmp.map(get70<T0,T1,T2,T3,T4,T5,T6>).template moveto_dvector<T0>();
  auto d1 = tmp.map(get71<T0,T1,T2,T3,T4,T5,T6>).template moveto_dvector<T1>();
  auto d2 = tmp.map(get72<T0,T1,T2,T3,T4,T5,T6>).template moveto_dvector<T2>();
  auto d3 = tmp.map(get73<T0,T1,T2,T3,T4,T5,T6>).template moveto_dvector<T3>();
  auto d4 = tmp.map(get74<T0,T1,T2,T3,T4,T5,T6>).template moveto_dvector<T4>();
  auto d5 = tmp.map(get75<T0,T1,T2,T3,T4,T5,T6>).template moveto_dvector<T5>();
  auto d6 = tmp.map(get76<T0,T1,T2,T3,T4,T5,T6>).template moveto_dvector<T6>();
  return std::make_tuple(d0, d1, d2, d3, d4, d5, d6);
}

template <class T0, class T1, class T2, class T3, class T4, class T5, class T6>
dftable make_dftable_dvector(dvector<std::tuple<T0,T1,T2,T3,T4,T5,T6>>& d,
                             std::vector<std::string> names) {
  auto dvs = convert_tuple(d);
  dftable ret;
  ret.append_column(names[0], std::get<0>(dvs));
  ret.append_column(names[1], std::get<1>(dvs));
  ret.append_column(names[2], std::get<2>(dvs));
  ret.append_column(names[3], std::get<3>(dvs));
  ret.append_column(names[4], std::get<4>(dvs));
  ret.append_column(names[5], std::get<5>(dvs));
  ret.append_column(names[6], std::get<6>(dvs));
  return ret;
}
}
#endif
