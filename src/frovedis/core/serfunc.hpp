#ifndef SERFUNC_HPP
#define SERFUNC_HPP

#include "mpi_rpc.hpp"
#include "serializehelper.hpp"

namespace frovedis {

template <class R>
class serfunc0 {
public:
  typedef R(*functype)();
  typedef R result_type;
  serfunc0() : func(0) {}
  serfunc0(R(*f)()) : func(f) {
    funcaddr = reinterpret_cast<intptr_t>(func);
  }
  R operator()(){
    if(func == 0) func = reinterpret_cast<functype>(funcaddr);
    return func();
  }
private:
  intptr_t funcaddr;
  functype func;

  SERIALIZE(funcaddr)
};

class serfuncv0 {
public:
  typedef void(*functype)();
  serfuncv0() : func(0) {}
  serfuncv0(void(*f)()) : func(f) {
    funcaddr = reinterpret_cast<intptr_t>(func);
  }
  void operator()(){
    if(func == 0) func = reinterpret_cast<functype>(funcaddr);
    func();
  }
private:
  intptr_t funcaddr;
  functype func;

  SERIALIZE(funcaddr)
};

template <class R, class T1>
class serfunc1 {
public:
  typedef R(*functype)(T1);
  typedef T1 argument_type;
  typedef R result_type;
  serfunc1() : func(0) {}
  serfunc1(R(*f)(T1)) : func(f) {
    funcaddr = reinterpret_cast<intptr_t>(func);
  }
  R operator()(T1 a1){
    if(func == 0) func = reinterpret_cast<functype>(funcaddr);
    return func(a1);
  }
private:
  intptr_t funcaddr;
  functype func;

  SERIALIZE(funcaddr)
};

template <class T1>
class serfuncv1 {
public:
  typedef void(*functype)(T1);
  typedef T1 argument_type;
  typedef void result_type;
  serfuncv1() : func(0) {}
  serfuncv1(void(*f)(T1)) : func(f) {
    funcaddr = reinterpret_cast<intptr_t>(func);
  }
  void operator()(T1 a1){
    if(func == 0) func = reinterpret_cast<functype>(funcaddr);
    func(a1);
  }
private:
  intptr_t funcaddr;
  functype func;

  SERIALIZE(funcaddr)
};

template <class R, class T1, class T2>
class serfunc2 {
public:
  typedef R(*functype)(T1,T2);
  typedef T1 first_argument_type;
  typedef T2 second_argument_type;
  typedef R result_type;
  serfunc2() : func(0) {}
  serfunc2(R(*f)(T1,T2)) : func(f) {
    funcaddr = reinterpret_cast<intptr_t>(func);
  }
  R operator()(T1 a1, T2 a2){
    if(func == 0) func = reinterpret_cast<functype>(funcaddr);
    return func(a1,a2);
  }
private:
  intptr_t funcaddr;
  functype func;

  SERIALIZE(funcaddr)
};

template <class T1, class T2>
class serfuncv2 {
public:
  typedef void(*functype)(T1,T2);
  typedef T1 argument_type;
  typedef T2 second_argument_type;
  typedef void result_type;
  serfuncv2() : func(0) {}
  serfuncv2(void(*f)(T1,T2)) : func(f) {
    funcaddr = reinterpret_cast<intptr_t>(func);
  }
  void operator()(T1 a1, T2 a2){
    if(func == 0) func = reinterpret_cast<functype>(funcaddr);
    func(a1,a2);
  }
private:
  intptr_t funcaddr;
  functype func;

  SERIALIZE(funcaddr)
};

template <class R, class T1, class T2, class T3>
class serfunc3 {
public:
  typedef R(*functype)(T1,T2,T3);
  typedef T1 first_argument_type;
  typedef T2 second_argument_type;
  typedef T3 third_argument_type;
  typedef R result_type;
  serfunc3() : func(0) {}
  serfunc3(R(*f)(T1,T2,T3)) : func(f) {
    funcaddr = reinterpret_cast<intptr_t>(func);
  }
  R operator()(T1 a1, T2 a2, T3 a3){
    if(func == 0) func = reinterpret_cast<functype>(funcaddr);
    return func(a1,a2,a3);
  }
private:
  intptr_t funcaddr;
  functype func;

  SERIALIZE(funcaddr)
};

template <class T1, class T2, class T3>
class serfuncv3 {
public:
  typedef void(*functype)(T1,T2,T3);
  typedef T1 argument_type;
  typedef T2 second_argument_type;
  typedef T3 third_argument_type;
  typedef void result_type;
  serfuncv3() : func(0) {}
  serfuncv3(void(*f)(T1,T2,T3)) : func(f) {
    funcaddr = reinterpret_cast<intptr_t>(func);
  }
  void operator()(T1 a1, T2 a2, T3 a3){
    if(func == 0) func = reinterpret_cast<functype>(funcaddr);
    func(a1,a2,a3);
  }
private:
  intptr_t funcaddr;
  functype func;

  SERIALIZE(funcaddr)
};

template <class R, class T1, class T2, class T3, class T4>
class serfunc4 {
public:
  typedef R(*functype)(T1,T2,T3,T4);
  typedef T1 first_argument_type;
  typedef T2 second_argument_type;
  typedef T3 third_argument_type;
  typedef T4 fourth_argument_type;
  typedef R result_type;
  serfunc4() : func(0) {}
  serfunc4(R(*f)(T1,T2,T3,T4)) : func(f) {
    funcaddr = reinterpret_cast<intptr_t>(func);
  }
  R operator()(T1 a1, T2 a2, T3 a3, T4 a4){
    if(func == 0) func = reinterpret_cast<functype>(funcaddr);
    return func(a1,a2,a3,a4);
  }
private:
  intptr_t funcaddr;
  functype func;

  SERIALIZE(funcaddr)
};

template <class T1, class T2, class T3, class T4>
class serfuncv4 {
public:
  typedef void(*functype)(T1,T2,T3,T4);
  typedef T1 argument_type;
  typedef T2 second_argument_type;
  typedef T3 third_argument_type;
  typedef T4 fourth_argument_type;
  typedef void result_type;
  serfuncv4() : func(0) {}
  serfuncv4(void(*f)(T1,T2,T3,T4)) : func(f) {
    funcaddr = reinterpret_cast<intptr_t>(func);
  }
  void operator()(T1 a1, T2 a2, T3 a3, T4 a4){
    if(func == 0) func = reinterpret_cast<functype>(funcaddr);
    func(a1,a2,a3,a4);
  }
private:
  intptr_t funcaddr;
  functype func;

  SERIALIZE(funcaddr)
};

template <class R, class T1, class T2, class T3, class T4, class T5>
class serfunc5 {
public:
  typedef R(*functype)(T1,T2,T3,T4,T5);
  typedef T1 first_argument_type;
  typedef T2 second_argument_type;
  typedef T3 third_argument_type;
  typedef T4 fourth_argument_type;
  typedef T5 fifth_argument_type;
  typedef R result_type;
  serfunc5() : func(0) {}
  serfunc5(R(*f)(T1,T2,T3,T4,T5)) : func(f) {
    funcaddr = reinterpret_cast<intptr_t>(func);
  }
  R operator()(T1 a1, T2 a2, T3 a3, T4 a4, T5 a5){
    if(func == 0) func = reinterpret_cast<functype>(funcaddr);
    return func(a1,a2,a3,a4,a5);
  }
private:
  intptr_t funcaddr;
  functype func;

  SERIALIZE(funcaddr)
};

template <class T1, class T2, class T3, class T4, class T5>
class serfuncv5 {
public:
  typedef void(*functype)(T1,T2,T3,T4,T5);
  typedef T1 argument_type;
  typedef T2 second_argument_type;
  typedef T3 third_argument_type;
  typedef T4 fourth_argument_type;
  typedef T5 fifth_argument_type;
  typedef void result_type;
  serfuncv5() : func(0) {}
  serfuncv5(void(*f)(T1,T2,T3,T4,T5)) : func(f) {
    funcaddr = reinterpret_cast<intptr_t>(func);
  }
  void operator()(T1 a1, T2 a2, T3 a3, T4 a4, T5 a5){
    if(func == 0) func = reinterpret_cast<functype>(funcaddr);
    func(a1,a2,a3,a4,a5);
  }
private:
  intptr_t funcaddr;
  functype func;

  SERIALIZE(funcaddr)
};

template <class R, class T1, class T2, class T3, class T4, class T5, class T6>
class serfunc6 {
public:
  typedef R(*functype)(T1,T2,T3,T4,T5,T6);
  typedef T1 first_argument_type;
  typedef T2 second_argument_type;
  typedef T3 third_argument_type;
  typedef T4 fourth_argument_type;
  typedef T5 fifth_argument_type;
  typedef T6 sixth_argument_type;
  typedef R result_type;
  serfunc6() : func(0) {}
  serfunc6(R(*f)(T1,T2,T3,T4,T5,T6)) : func(f) {
    funcaddr = reinterpret_cast<intptr_t>(func);
  }
  R operator()(T1 a1, T2 a2, T3 a3, T4 a4, T5 a5, T6 a6){
    if(func == 0) func = reinterpret_cast<functype>(funcaddr);
    return func(a1,a2,a3,a4,a5,a6);
  }
private:
  intptr_t funcaddr;
  functype func;

  SERIALIZE(funcaddr)
};

template <class T1, class T2, class T3, class T4, class T5, class T6>
class serfuncv6 {
public:
  typedef void(*functype)(T1,T2,T3,T4,T5,T6);
  typedef T1 argument_type;
  typedef T2 second_argument_type;
  typedef T3 third_argument_type;
  typedef T4 fourth_argument_type;
  typedef T5 fifth_argument_type;
  typedef T6 sixth_argument_type;
  typedef void result_type;
  serfuncv6() : func(0) {}
  serfuncv6(void(*f)(T1,T2,T3,T4,T5,T6)) : func(f) {
    funcaddr = reinterpret_cast<intptr_t>(func);
  }
  void operator()(T1 a1, T2 a2, T3 a3, T4 a4, T5 a5, T6 a6){
    if(func == 0) func = reinterpret_cast<functype>(funcaddr);
    func(a1,a2,a3,a4,a5,a6);
  }
private:
  intptr_t funcaddr;
  functype func;

  SERIALIZE(funcaddr)
};

template <class R, class T1, class T2, class T3, class T4, class T5, class T6,
          class T7>
class serfunc7 {
public:
  typedef R(*functype)(T1,T2,T3,T4,T5,T6,T7);
  typedef T1 first_argument_type;
  typedef T2 second_argument_type;
  typedef T3 third_argument_type;
  typedef T4 fourth_argument_type;
  typedef T5 fifth_argument_type;
  typedef T6 sixth_argument_type;
  typedef T7 seventh_argument_type;
  typedef R result_type;
  serfunc7() : func(0) {}
  serfunc7(R(*f)(T1,T2,T3,T4,T5,T6,T7)) : func(f) {
    funcaddr = reinterpret_cast<intptr_t>(func);
  }
  R operator()(T1 a1, T2 a2, T3 a3, T4 a4, T5 a5, T6 a6, T7 a7){
    if(func == 0) func = reinterpret_cast<functype>(funcaddr);
    return func(a1,a2,a3,a4,a5,a6,a7);
  }
private:
  intptr_t funcaddr;
  functype func;

  SERIALIZE(funcaddr)
};

template <class T1, class T2, class T3, class T4, class T5, class T6, class T7>
class serfuncv7 {
public:
  typedef void(*functype)(T1,T2,T3,T4,T5,T6,T7);
  typedef T1 argument_type;
  typedef T2 second_argument_type;
  typedef T3 third_argument_type;
  typedef T4 fourth_argument_type;
  typedef T5 fifth_argument_type;
  typedef T6 sixth_argument_type;
  typedef T7 seventh_argument_type;
  typedef void result_type;
  serfuncv7() : func(0) {}
  serfuncv7(void(*f)(T1,T2,T3,T4,T5,T6,T7)) : func(f) {
    funcaddr = reinterpret_cast<intptr_t>(func);
  }
  void operator()(T1 a1, T2 a2, T3 a3, T4 a4, T5 a5, T6 a6, T7 a7){
    if(func == 0) func = reinterpret_cast<functype>(funcaddr);
    func(a1,a2,a3,a4,a5,a6,a7);
  }
private:
  intptr_t funcaddr;
  functype func;

  SERIALIZE(funcaddr)
};

template <class R, class T1, class T2, class T3, class T4, class T5, class T6,
          class T7, class T8>
class serfunc8 {
public:
  typedef R(*functype)(T1,T2,T3,T4,T5,T6,T7,T8);
  typedef T1 first_argument_type;
  typedef T2 second_argument_type;
  typedef T3 third_argument_type;
  typedef T4 fourth_argument_type;
  typedef T5 fifth_argument_type;
  typedef T6 sixth_argument_type;
  typedef T7 seventh_argument_type;
  typedef T8 eighth_argument_type;
  typedef R result_type;
  serfunc8() : func(0) {}
  serfunc8(R(*f)(T1,T2,T3,T4,T5,T6,T7,T8)) : func(f) {
    funcaddr = reinterpret_cast<intptr_t>(func);
  }
  R operator()(T1 a1, T2 a2, T3 a3, T4 a4, T5 a5, T6 a6, T7 a7, T8 a8){
    if(func == 0) func = reinterpret_cast<functype>(funcaddr);
    return func(a1,a2,a3,a4,a5,a6,a7,a8);
  }
private:
  intptr_t funcaddr;
  functype func;

  SERIALIZE(funcaddr)
};

template <class T1, class T2, class T3, class T4, class T5, class T6, class T7,
          class T8>
class serfuncv8 {
public:
  typedef void(*functype)(T1,T2,T3,T4,T5,T6,T7,T8);
  typedef T1 argument_type;
  typedef T2 second_argument_type;
  typedef T3 third_argument_type;
  typedef T4 fourth_argument_type;
  typedef T5 fifth_argument_type;
  typedef T6 sixth_argument_type;
  typedef T7 seventh_argument_type;
  typedef T8 eighth_argument_type;
  typedef void result_type;
  serfuncv8() : func(0) {}
  serfuncv8(void(*f)(T1,T2,T3,T4,T5,T6,T7,T8)) : func(f) {
    funcaddr = reinterpret_cast<intptr_t>(func);
  }
  void operator()(T1 a1, T2 a2, T3 a3, T4 a4, T5 a5, T6 a6, T7 a7, T8 a8){
    if(func == 0) func = reinterpret_cast<functype>(funcaddr);
    func(a1,a2,a3,a4,a5,a6,a7,a8);
  }
private:
  intptr_t funcaddr;
  functype func;

  SERIALIZE(funcaddr)
};

template <class R, class T1, class T2, class T3, class T4, class T5, class T6,
          class T7, class T8, class T9>
class serfunc9 {
public:
  typedef R(*functype)(T1,T2,T3,T4,T5,T6,T7,T8,T9);
  typedef T1 first_argument_type;
  typedef T2 second_argument_type;
  typedef T3 third_argument_type;
  typedef T4 fourth_argument_type;
  typedef T5 fifth_argument_type;
  typedef T6 sixth_argument_type;
  typedef T7 seventh_argument_type;
  typedef T8 eighth_argument_type;
  typedef T9 ninth_argument_type;
  typedef R result_type;
  serfunc9() : func(0) {}
  serfunc9(R(*f)(T1,T2,T3,T4,T5,T6,T7,T8,T9)) : func(f) {
    funcaddr = reinterpret_cast<intptr_t>(func);
  }
  R operator()(T1 a1, T2 a2, T3 a3, T4 a4, T5 a5, T6 a6, T7 a7, T8 a8, T9 a9){
    if(func == 0) func = reinterpret_cast<functype>(funcaddr);
    return func(a1,a2,a3,a4,a5,a6,a7,a8,a9);
  }
private:
  intptr_t funcaddr;
  functype func;

  SERIALIZE(funcaddr)
};

template <class T1, class T2, class T3, class T4, class T5, class T6, class T7,
          class T8, class T9>
class serfuncv9 {
public:
  typedef void(*functype)(T1,T2,T3,T4,T5,T6,T7,T8,T9);
  typedef T1 argument_type;
  typedef T2 second_argument_type;
  typedef T3 third_argument_type;
  typedef T4 fourth_argument_type;
  typedef T5 fifth_argument_type;
  typedef T6 sixth_argument_type;
  typedef T7 seventh_argument_type;
  typedef T8 eighth_argument_type;
  typedef T9 ninth_argument_type;
  typedef void result_type;
  serfuncv9() : func(0) {}
  serfuncv9(void(*f)(T1,T2,T3,T4,T5,T6,T7,T8,T9)) : func(f) {
    funcaddr = reinterpret_cast<intptr_t>(func);
  }
  void operator()(T1 a1, T2 a2, T3 a3, T4 a4, T5 a5, T6 a6, T7 a7, T8 a8,
                  T9 a9){
    if(func == 0) func = reinterpret_cast<functype>(funcaddr);
    func(a1,a2,a3,a4,a5,a6,a7,a8,a9);
  }
private:
  intptr_t funcaddr;
  functype func;

  SERIALIZE(funcaddr)
};

template <class R>
serfunc0<R> make_serfunc(R(*f)()){return serfunc0<R>(f);}

inline
serfuncv0 make_serfunc(void(*f)()){return serfuncv0(f);}

template <class R, class T1>
serfunc1<R,T1> make_serfunc(R(*f)(T1)){return serfunc1<R,T1>(f);}

template <class T1>
serfuncv1<T1> make_serfunc(void(*f)(T1)){return serfuncv1<T1>(f);}

template <class R, class T1, class T2>
serfunc2<R,T1,T2> make_serfunc(R(*f)(T1,T2)){return serfunc2<R,T1,T2>(f);}

template <class T1, class T2>
serfuncv2<T1,T2> make_serfunc(void(*f)(T1,T2)){return serfuncv2<T1,T2>(f);}

template <class R, class T1, class T2, class T3>
serfunc3<R,T1,T2,T3> make_serfunc(R(*f)(T1,T2,T3)){
  return serfunc3<R,T1,T2,T3>(f);
}

template <class T1, class T2, class T3>
serfuncv3<T1,T2,T3> make_serfunc(void(*f)(T1,T2,T3)){
  return serfuncv3<T1,T2,T3>(f);
}

template <class R, class T1, class T2, class T3, class T4>
serfunc4<R,T1,T2,T3,T4> make_serfunc(R(*f)(T1,T2,T3,T4)){
  return serfunc4<R,T1,T2,T3,T4>(f);
}

template <class T1, class T2, class T3, class T4>
serfuncv4<T1,T2,T3,T4> make_serfunc(void(*f)(T1,T2,T3,T4)){
  return serfuncv4<T1,T2,T3,T4>(f);
}

template <class R, class T1, class T2, class T3, class T4, class T5>
serfunc5<R,T1,T2,T3,T4,T5> make_serfunc(R(*f)(T1,T2,T3,T4,T5)){
  return serfunc5<R,T1,T2,T3,T4,T5>(f);
}

template <class T1, class T2, class T3, class T4, class T5>
serfuncv5<T1,T2,T3,T4,T5> make_serfunc(void(*f)(T1,T2,T3,T4,T5)){
  return serfuncv5<T1,T2,T3,T4,T5>(f);
}

template <class R, class T1, class T2, class T3, class T4, class T5, class T6>
serfunc6<R,T1,T2,T3,T4,T5,T6> make_serfunc(R(*f)(T1,T2,T3,T4,T5,T6)){
  return serfunc6<R,T1,T2,T3,T4,T5,T6>(f);
}

template <class T1, class T2, class T3, class T4, class T5, class T6>
serfuncv6<T1,T2,T3,T4,T5,T6> make_serfunc(void(*f)(T1,T2,T3,T4,T5,T6)){
  return serfuncv6<T1,T2,T3,T4,T5,T6>(f);
}

template <class R, class T1, class T2, class T3, class T4, class T5, class T6,
          class T7>
serfunc7<R,T1,T2,T3,T4,T5,T6,T7> make_serfunc(R(*f)(T1,T2,T3,T4,T5,T6,T7)){
  return serfunc7<R,T1,T2,T3,T4,T5,T6,T7>(f);
}

template <class T1, class T2, class T3, class T4, class T5, class T6, class T7>
serfuncv7<T1,T2,T3,T4,T5,T6,T7> make_serfunc(void(*f)(T1,T2,T3,T4,T5,T6,T7)){
  return serfuncv7<T1,T2,T3,T4,T5,T6,T7>(f);
}

template <class R, class T1, class T2, class T3, class T4, class T5, class T6,
          class T7, class T8>
serfunc8<R,T1,T2,T3,T4,T5,T6,T7,T8> 
make_serfunc(R(*f)(T1,T2,T3,T4,T5,T6,T7,T8)){
  return serfunc8<R,T1,T2,T3,T4,T5,T6,T7,T8>(f);
}

template <class T1, class T2, class T3, class T4, class T5, class T6, class T7,
          class T8>
serfuncv8<T1,T2,T3,T4,T5,T6,T7,T8>
make_serfunc(void(*f)(T1,T2,T3,T4,T5,T6,T7,T8)){
  return serfuncv8<T1,T2,T3,T4,T5,T6,T7,T8>(f);
}

template <class R, class T1, class T2, class T3, class T4, class T5, class T6,
          class T7, class T8, class T9>
serfunc9<R,T1,T2,T3,T4,T5,T6,T7,T8,T9>
make_serfunc(R(*f)(T1,T2,T3,T4,T5,T6,T7,T8,T9)){
  return serfunc9<R,T1,T2,T3,T4,T5,T6,T7,T8,T9>(f);
}

template <class T1, class T2, class T3, class T4, class T5, class T6, class T7,
          class T8, class T9>
serfuncv9<T1,T2,T3,T4,T5,T6,T7,T8,T9>
make_serfunc(void(*f)(T1,T2,T3,T4,T5,T6,T7,T8,T9)){
  return serfuncv9<T1,T2,T3,T4,T5,T6,T7,T8,T9>(f);
}

}
#endif
