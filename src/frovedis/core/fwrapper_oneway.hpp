#ifndef FWRAPPER_ONEWAY_HPP
#define FWRAPPER_ONEWAY_HPP

namespace frovedis {

template <class T1, class T2, class T3, class T4, class T5, class T6,
          class T7, class T8, class T9, class T10, class T11>
void fwrapper_oneway(intptr_t function_addr, my_iarchive& inar)
{
  typedef void(*Ftype)(T1&,T2&,T3&,T4&,T5&,T6&,T7&,T8&,T9&,T10&,T11&);
  Ftype f = reinterpret_cast<Ftype>(function_addr);
  T1 a1;
  T2 a2;
  T3 a3;
  T4 a4;
  T5 a5;
  T6 a6;
  T7 a7;
  T8 a8;
  T9 a9;
  T10 a10;
  T11 a11;
  inar & a1;
  inar & a2;
  inar & a3;
  inar & a4;
  inar & a5;
  inar & a6;
  inar & a7;
  inar & a8;
  inar & a9;
  inar & a10;
  inar & a11;
  f(a1,a2,a3,a4,a5,a6,a7,a8,a9,a10,a11);
  return;
}

template <class T1, class T2, class T3, class T4, class T5, class T6,
          class T7, class T8, class T9, class T10>
void fwrapper_oneway(intptr_t function_addr, my_iarchive& inar)
{
  typedef void(*Ftype)(T1&,T2&,T3&,T4&,T5&,T6&,T7&,T8&,T9&,T10&);
  Ftype f = reinterpret_cast<Ftype>(function_addr);
  T1 a1;
  T2 a2;
  T3 a3;
  T4 a4;
  T5 a5;
  T6 a6;
  T7 a7;
  T8 a8;
  T9 a9;
  T10 a10;
  inar & a1;
  inar & a2;
  inar & a3;
  inar & a4;
  inar & a5;
  inar & a6;
  inar & a7;
  inar & a8;
  inar & a9;
  inar & a10;
  f(a1,a2,a3,a4,a5,a6,a7,a8,a9,a10);
  return;
}

template <class T1, class T2, class T3, class T4, class T5, class T6,
          class T7, class T8, class T9>
void fwrapper_oneway(intptr_t function_addr, my_iarchive& inar)
{
  typedef void(*Ftype)(T1&,T2&,T3&,T4&,T5&,T6&,T7&,T8&,T9&);
  Ftype f = reinterpret_cast<Ftype>(function_addr);
  T1 a1;
  T2 a2;
  T3 a3;
  T4 a4;
  T5 a5;
  T6 a6;
  T7 a7;
  T8 a8;
  T9 a9;
  inar & a1;
  inar & a2;
  inar & a3;
  inar & a4;
  inar & a5;
  inar & a6;
  inar & a7;
  inar & a8;
  inar & a9;
  f(a1,a2,a3,a4,a5,a6,a7,a8,a9);
  return;
}

template <class T1, class T2, class T3, class T4, class T5, class T6,
          class T7, class T8>
void fwrapper_oneway(intptr_t function_addr, my_iarchive& inar)
{
  typedef void(*Ftype)(T1&,T2&,T3&,T4&,T5&,T6&,T7&,T8&);
  Ftype f = reinterpret_cast<Ftype>(function_addr);
  T1 a1;
  T2 a2;
  T3 a3;
  T4 a4;
  T5 a5;
  T6 a6;
  T7 a7;
  T8 a8;
  inar & a1;
  inar & a2;
  inar & a3;
  inar & a4;
  inar & a5;
  inar & a6;
  inar & a7;
  inar & a8;
  f(a1,a2,a3,a4,a5,a6,a7,a8);
  return;
}

template <class T1, class T2, class T3, class T4, class T5, class T6, class T7>
void fwrapper_oneway(intptr_t function_addr, my_iarchive& inar)
{
  typedef void(*Ftype)(T1&,T2&,T3&,T4&,T5&,T6&,T7&);
  Ftype f = reinterpret_cast<Ftype>(function_addr);
  T1 a1;
  T2 a2;
  T3 a3;
  T4 a4;
  T5 a5;
  T6 a6;
  T7 a7;
  inar & a1;
  inar & a2;
  inar & a3;
  inar & a4;
  inar & a5;
  inar & a6;
  inar & a7;
  f(a1,a2,a3,a4,a5,a6,a7);
  return;
}

template <class T1, class T2, class T3, class T4, class T5, class T6>
void fwrapper_oneway(intptr_t function_addr, my_iarchive& inar)
{
  typedef void(*Ftype)(T1&,T2&,T3&,T4&,T5&,T6&);
  Ftype f = reinterpret_cast<Ftype>(function_addr);
  T1 a1;
  T2 a2;
  T3 a3;
  T4 a4;
  T5 a5;
  T6 a6;
  inar & a1;
  inar & a2;
  inar & a3;
  inar & a4;
  inar & a5;
  inar & a6;
  f(a1,a2,a3,a4,a5,a6);
  return;
}

template <class T1, class T2, class T3, class T4, class T5>
void fwrapper_oneway(intptr_t function_addr, my_iarchive& inar)
{
  typedef void(*Ftype)(T1&,T2&,T3&,T4&,T5&);
  Ftype f = reinterpret_cast<Ftype>(function_addr);
  T1 a1;
  T2 a2;
  T3 a3;
  T4 a4;
  T5 a5;
  inar & a1;
  inar & a2;
  inar & a3;
  inar & a4;
  inar & a5;
  f(a1,a2,a3,a4,a5);
  return;
}

template <class T1, class T2, class T3, class T4>
void fwrapper_oneway(intptr_t function_addr, my_iarchive& inar)
{
  typedef void(*Ftype)(T1&,T2&,T3&,T4&);
  Ftype f = reinterpret_cast<Ftype>(function_addr);
  T1 a1;
  T2 a2;
  T3 a3;
  T4 a4;
  inar & a1;
  inar & a2;
  inar & a3;
  inar & a4;
  f(a1,a2,a3,a4);
  return;
}

template <class T1, class T2, class T3>
void fwrapper_oneway(intptr_t function_addr, my_iarchive& inar)
{
  typedef void(*Ftype)(T1&,T2&,T3&);
  Ftype f = reinterpret_cast<Ftype>(function_addr);
  T1 a1;
  T2 a2;
  T3 a3;
  inar & a1;
  inar & a2;
  inar & a3;
  f(a1,a2,a3);
  return;
}

template <class T1, class T2>
void fwrapper_oneway(intptr_t function_addr, my_iarchive& inar)
{
  typedef void(*Ftype)(T1&,T2&);
  Ftype f = reinterpret_cast<Ftype>(function_addr);
  T1 a1;
  T2 a2;
  inar & a1;
  inar & a2;
  f(a1,a2);
  return;
}

template <class T1>
void fwrapper_oneway(intptr_t function_addr, my_iarchive& inar)
{
  typedef void(*Ftype)(T1&);
  Ftype f = reinterpret_cast<Ftype>(function_addr);
  T1 a1;
  inar & a1;
  f(a1);
  return;
}

void fwrapper_oneway0(intptr_t function_addr, const std::string& input);

}

#endif
