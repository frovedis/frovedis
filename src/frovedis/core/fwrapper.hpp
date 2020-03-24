#ifndef FWRAPPER_HPP
#define FWRAPPER_HPP

namespace frovedis {

template <class R, class T1, class T2, class T3, class T4, class T5, class T6,
          class T7, class T8, class T9, class T10>
void fwrapper(intptr_t function_addr, my_iarchive& inar, 
              my_oarchive& outar) {
  typedef R(*Ftype)(T1&,T2&,T3&,T4&,T5&,T6&,T7&,T8&,T9&,T10&);
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
  R o = f(a1,a2,a3,a4,a5,a6,a7,a8,a9,a10);
  outar & (const R&) o;
  return;
}

template <class R, class T1, class T2, class T3, class T4, class T5, class T6,
          class T7, class T8, class T9>
void fwrapper(intptr_t function_addr, my_iarchive& inar, 
              my_oarchive& outar) {
  typedef R(*Ftype)(T1&,T2&,T3&,T4&,T5&,T6&,T7&,T8&,T9&);
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
  R o = f(a1,a2,a3,a4,a5,a6,a7,a8,a9);
  outar & (const R&) o;
  return;
}

template <class R, class T1, class T2, class T3, class T4, class T5, class T6,
          class T7, class T8>
void fwrapper(intptr_t function_addr, my_iarchive& inar, 
              my_oarchive& outar) {
  typedef R(*Ftype)(T1&,T2&,T3&,T4&,T5&,T6&,T7&,T8&);
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
  R o = f(a1,a2,a3,a4,a5,a6,a7,a8);
  outar & (const R&) o;
  return;
}

template <class R, class T1, class T2, class T3, class T4, class T5, class T6,
          class T7>
void fwrapper(intptr_t function_addr, my_iarchive& inar, 
              my_oarchive& outar) {
  typedef R(*Ftype)(T1&,T2&,T3&,T4&,T5&,T6&,T7&);
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
  R o = f(a1,a2,a3,a4,a5,a6,a7);
  outar & (const R&) o;
  return;
}

template <class R, class T1, class T2, class T3, class T4, class T5, class T6>
void fwrapper(intptr_t function_addr, my_iarchive& inar, 
              my_oarchive& outar) {
  typedef R(*Ftype)(T1&,T2&,T3&,T4&,T5&,T6&);
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
  R o = f(a1,a2,a3,a4,a5,a6);
  outar & (const R&) o;
  return;
}

template <class R, class T1, class T2, class T3, class T4, class T5>
void fwrapper(intptr_t function_addr, my_iarchive& inar, 
              my_oarchive& outar) {
  typedef R(*Ftype)(T1&,T2&,T3&,T4&,T5&);
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
  R o = f(a1,a2,a3,a4,a5);
  outar & (const R&) o;
  return;
}

template <class R, class T1, class T2, class T3, class T4>
void fwrapper(intptr_t function_addr, my_iarchive& inar, 
              my_oarchive& outar) {
  typedef R(*Ftype)(T1&,T2&,T3&,T4&);
  Ftype f = reinterpret_cast<Ftype>(function_addr);
  T1 a1;
  T2 a2;
  T3 a3;
  T4 a4;
  inar & a1;
  inar & a2;
  inar & a3;
  inar & a4;
  R o = f(a1,a2,a3,a4);
  outar & (const R&) o;
  return;
}

template <class R, class T1, class T2, class T3>
void fwrapper(intptr_t function_addr, my_iarchive& inar, 
              my_oarchive& outar) {
  typedef R(*Ftype)(T1&,T2&,T3&);
  Ftype f = reinterpret_cast<Ftype>(function_addr);
  T1 a1;
  T2 a2;
  T3 a3;
  inar & a1;
  inar & a2;
  inar & a3;
  R o = f(a1,a2,a3);
  outar & (const R&) o;
  return;
}

template <class R, class T1, class T2>
void fwrapper(intptr_t function_addr, my_iarchive& inar, 
              my_oarchive& outar) {
  typedef R(*Ftype)(T1&,T2&);
  Ftype f = reinterpret_cast<Ftype>(function_addr);
  T1 a1;
  T2 a2;
  inar & a1;
  inar & a2;
  R o = f(a1,a2);
  outar & (const R&) o;
  return;
}

template <class R, class T1>
void fwrapper(intptr_t function_addr, my_iarchive& inar, 
              my_oarchive& outar) {
  typedef R(*Ftype)(T1&);
  Ftype f = reinterpret_cast<Ftype>(function_addr);
  T1 a1;
  inar & a1;
  R o = f(a1);
  outar & (const R&) o;
  return;
}

template <class R>
void fwrapper(intptr_t function_addr, my_iarchive& inar, 
              my_oarchive& outar) {
  typedef R(*Ftype)();
  Ftype f = reinterpret_cast<Ftype>(function_addr);
  R o = f();
  outar & (const R&) o;
  return;
}

}

#endif
