#ifndef EXRPC_EXPOSE_HPP
#define EXRPC_EXPOSE_HPP

#include "frovedis.hpp"
#include "exrpc.hpp"
#include "exrpc_async.hpp"
#include "exrpc_oneway.hpp"
#include "exrpc_oneway_noexcept.hpp"

#define expose(func) expose_function(#func, func)

namespace frovedis {

// function_id, <pointer_to_wrapper, pointer_to_function>
extern std::map<std::string,
                std::pair<intptr_t, intptr_t>> expose_table;

// expose is not thread safe
template <class R, class T1, class T2, class T3, class T4, class T5,
          class T6, class T7, class T8, class T9, class T10, class T11>
void expose_function(const std::string& func_name, R(*f)(T1&,T2&,T3&,T4&,T5&,
                                                         T6&,T7&,T8&,T9&,
                                                         T10&,T11&)){
  expose_table[func_name] =
    std::make_pair(reinterpret_cast<intptr_t>(pfwrapper<R,T1,T2,T3,T4,T5,
                                              T6,T7,T8,T9,T10,T11>),
                   reinterpret_cast<intptr_t>(f));
}

template <class R, class T1, class T2, class T3, class T4, class T5,
          class T6, class T7, class T8, class T9, class T10>
void expose_function(const std::string& func_name, R(*f)(T1&,T2&,T3&,T4&,T5&,
                                                         T6&,T7&,T8&,T9&,
                                                         T10&)){
  expose_table[func_name] =
    std::make_pair(reinterpret_cast<intptr_t>(pfwrapper<R,T1,T2,T3,T4,T5,
                                              T6,T7,T8,T9,T10>),
                   reinterpret_cast<intptr_t>(f));
}

template <class R, class T1, class T2, class T3, class T4, class T5,
          class T6, class T7, class T8, class T9>
void expose_function(const std::string& func_name, R(*f)(T1&,T2&,T3&,T4&,T5&,
                                                         T6&,T7&,T8&,T9&)){
  expose_table[func_name] =
    std::make_pair(reinterpret_cast<intptr_t>(pfwrapper<R,T1,T2,T3,T4,T5,
                                              T6,T7,T8,T9>),
                   reinterpret_cast<intptr_t>(f));
}

template <class R, class T1, class T2, class T3, class T4, class T5,
          class T6, class T7, class T8>
void expose_function(const std::string& func_name, R(*f)(T1&,T2&,T3&,T4&,T5&,
                                                         T6&,T7&,T8&)) {
  expose_table[func_name] =
    std::make_pair(reinterpret_cast<intptr_t>(pfwrapper<R,T1,T2,T3,T4,T5,
                                              T6,T7,T8>),
                   reinterpret_cast<intptr_t>(f));
}

template <class R, class T1, class T2, class T3, class T4, class T5,
          class T6, class T7>
void expose_function(const std::string& func_name, R(*f)(T1&,T2&,T3&,T4&,T5&,
                                                         T6&,T7&)) {
  expose_table[func_name] =
    std::make_pair(reinterpret_cast<intptr_t>(pfwrapper<R,T1,T2,T3,T4,T5,
                                              T6,T7>),
                   reinterpret_cast<intptr_t>(f));
}

template <class R, class T1, class T2, class T3, class T4, class T5,
          class T6>
void expose_function(const std::string& func_name, R(*f)(T1&,T2&,T3&,T4&,T5&,
                                                         T6&)) {
  expose_table[func_name] =
    std::make_pair(reinterpret_cast<intptr_t>(pfwrapper<R,T1,T2,T3,T4,T5,
                                              T6>),
                   reinterpret_cast<intptr_t>(f));
}

template <class R, class T1, class T2, class T3, class T4, class T5>
void expose_function(const std::string& func_name, R(*f)(T1&,T2&,T3&,T4&,T5&)){
  expose_table[func_name] =
    std::make_pair(reinterpret_cast<intptr_t>(pfwrapper<R,T1,T2,T3,T4,T5>),
                   reinterpret_cast<intptr_t>(f));
}

template <class R, class T1, class T2, class T3, class T4>
void expose_function(const std::string& func_name, R(*f)(T1&,T2&,T3&,T4&)) {
  expose_table[func_name] =
    std::make_pair(reinterpret_cast<intptr_t>(pfwrapper<R,T1,T2,T3,T4>),
                   reinterpret_cast<intptr_t>(f));
}

template <class R, class T1, class T2, class T3>
void expose_function(const std::string& func_name, R(*f)(T1&,T2&,T3&)) {
  expose_table[func_name] =
    std::make_pair(reinterpret_cast<intptr_t>(pfwrapper<R,T1,T2,T3>),
                   reinterpret_cast<intptr_t>(f));
}

template <class R, class T1, class T2>
void expose_function(const std::string& func_name, R(*f)(T1&,T2&)) {
  expose_table[func_name] =
    std::make_pair(reinterpret_cast<intptr_t>(pfwrapper<R,T1,T2>),
                   reinterpret_cast<intptr_t>(f));
}

template <class R, class T1>
void expose_function(const std::string& func_name, R(*f)(T1&)) {
  expose_table[func_name] =
    std::make_pair(reinterpret_cast<intptr_t>(pfwrapper<R,T1>),
                   reinterpret_cast<intptr_t>(f));
}

template <class R>
void expose_function(const std::string& func_name, R(*f)()) {
  expose_table[func_name] =
    std::make_pair(reinterpret_cast<intptr_t>(pfwrapper<R>),
                   reinterpret_cast<intptr_t>(f));
}

template <class T1, class T2, class T3, class T4, class T5,
          class T6, class T7, class T8, class T9, class T10, class T11>
void expose_function(const std::string& func_name, void(*f)(T1&,T2&,T3&,T4&,
                                                            T5&,T6&,T7&,T8&,
                                                            T9&,T10&,T11&)) {
  expose_table[func_name] =
    std::make_pair(reinterpret_cast<intptr_t>(pfwrapper_oneway<T1,T2,T3,T4,
                                              T5,T6,T7,T8,T9,T10,T11>),
                   reinterpret_cast<intptr_t>(f));
}

template <class T1, class T2, class T3, class T4, class T5, 
          class T6, class T7, class T8, class T9, class T10>
void expose_function(const std::string& func_name, void(*f)(T1&,T2&,T3&,T4&,
                                                            T5&,T6&,T7&,T8&,
                                                            T9&,T10&)) {
  expose_table[func_name] =
    std::make_pair(reinterpret_cast<intptr_t>(pfwrapper_oneway<T1,T2,T3,T4,
                                              T5,T6,T7,T8,T9,T10>),
                   reinterpret_cast<intptr_t>(f));
}

template <class T1, class T2, class T3, class T4, class T5, 
          class T6, class T7, class T8, class T9>
void expose_function(const std::string& func_name, void(*f)(T1&,T2&,T3&,T4&,
                                                            T5&,T6&,T7&,T8&,
                                                            T9&)) {
  expose_table[func_name] =
    std::make_pair(reinterpret_cast<intptr_t>(pfwrapper_oneway<T1,T2,T3,T4,
                                              T5,T6,T7,T8,T9>),
                   reinterpret_cast<intptr_t>(f));
}

template <class T1, class T2, class T3, class T4, class T5, 
          class T6, class T7, class T8>
void expose_function(const std::string& func_name, void(*f)(T1&,T2&,T3&,T4&,
                                                            T5&,T6&,T7&,T8&)){
  expose_table[func_name] =
    std::make_pair(reinterpret_cast<intptr_t>(pfwrapper_oneway<T1,T2,T3,T4,
                                              T5,T6,T7,T8>),
                   reinterpret_cast<intptr_t>(f));
}

template <class T1, class T2, class T3, class T4, class T5, 
          class T6, class T7>
void expose_function(const std::string& func_name, void(*f)(T1&,T2&,T3&,T4&,
                                                            T5&,T6&,T7&)){
  expose_table[func_name] =
    std::make_pair(reinterpret_cast<intptr_t>(pfwrapper_oneway<T1,T2,T3,T4,
                                              T5,T6,T7>),
                   reinterpret_cast<intptr_t>(f));
}

template <class T1, class T2, class T3, class T4, class T5, 
          class T6>
void expose_function(const std::string& func_name, void(*f)(T1&,T2&,T3&,T4&,
                                                            T5&,T6&)){
  expose_table[func_name] =
    std::make_pair(reinterpret_cast<intptr_t>(pfwrapper_oneway<T1,T2,T3,T4,
                                              T5,T6>),
                   reinterpret_cast<intptr_t>(f));
}

template <class T1, class T2, class T3, class T4, class T5>
void expose_function(const std::string& func_name, void(*f)(T1&,T2&,T3&,T4&,
                                                            T5&)){
  expose_table[func_name] =
    std::make_pair(reinterpret_cast<intptr_t>(pfwrapper_oneway<T1,T2,T3,T4,
                                              T5>),
                   reinterpret_cast<intptr_t>(f));
}

template <class T1, class T2, class T3, class T4>
void expose_function(const std::string& func_name, void(*f)(T1&,T2&,T3&,T4&)) {
  expose_table[func_name] =
    std::make_pair(reinterpret_cast<intptr_t>(pfwrapper_oneway<T1,T2,T3,T4>),
                   reinterpret_cast<intptr_t>(f));
}

template <class T1, class T2, class T3>
void expose_function(const std::string& func_name, void(*f)(T1&,T2&,T3&)) {
  expose_table[func_name] =
    std::make_pair(reinterpret_cast<intptr_t>(pfwrapper_oneway<T1,T2,T3>),
                   reinterpret_cast<intptr_t>(f));
}

template <class T1, class T2>
void expose_function(const std::string& func_name, void(*f)(T1&,T2&)) {
  expose_table[func_name] =
    std::make_pair(reinterpret_cast<intptr_t>(pfwrapper_oneway<T1,T2>),
                   reinterpret_cast<intptr_t>(f));
}

template <class T1>
void expose_function(const std::string& func_name, void(*f)(T1&)) {
  expose_table[func_name] =
    std::make_pair(reinterpret_cast<intptr_t>(pfwrapper_oneway<T1>),
                   reinterpret_cast<intptr_t>(f));
}

void expose_function(const std::string& func_name, void(*f)());

void flush_exposed();

}
#endif
