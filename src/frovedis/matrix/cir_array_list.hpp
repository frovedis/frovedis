#ifndef _C_ARRAY_LIST_HPP_
#define _C_ARRAY_LIST_HPP_

#include <vector>
#include "../core/exceptions.hpp"

#include <boost/lexical_cast.hpp>
#define ITOS boost::lexical_cast<std::string>

namespace frovedis {

template <class T>
struct cir_array_list {
  explicit cir_array_list(size_t size=10): 
    rear(-1), front(-1), totActiveElements(0), len(size) 
      { val.resize(size); }
  cir_array_list(const cir_array_list& l) {
    val = l.val;
    rear  = l.rear;
    front = l.front;
    len = l.len;
    totActiveElements = l.totActiveElements;
  }
  cir_array_list(cir_array_list&& l) {
    val.swap(l.val);
    rear  = l.rear;
    front = l.front;
    len = l.len;
    totActiveElements = l.totActiveElements;
    l.reset();
  }
  cir_array_list& operator=(const cir_array_list& l) {
    val = l.val;
    rear  = l.rear;
    front = l.front;
    len = l.len;
    totActiveElements = l.totActiveElements;
    return *this;
  }
  cir_array_list& operator=(cir_array_list&& l) {
    val.swap(l.val);
    rear  = l.rear;
    front = l.front;
    len = l.len;
    totActiveElements = l.totActiveElements;
    l.reset();
    return *this;
  }
  bool isFull() const {
    return (totActiveElements == len);
  }
  bool isEmpty() const {
    return (totActiveElements == 0);
  }
  size_t size() const { 
    return len; 
  }
  void debug_print() const {
    for (size_t i = 0; i < totActiveElements; ++i) { 
      std::cout << "[" << i << "] " << operator[](i) << "\n";
    }
    std::cout << std::endl;
  }
  void reset() {
    rear = front = -1;
    len = totActiveElements = 0;
    val.clear();
  }
  void push_back_impl() {
    if(isEmpty()) {
      rear = front = 0;
    }
    else if (isFull()) { // insertion is possible, even if list is full 
      rear  = (rear+1)  % len;
      front = (front+1) % len;
    }
    else {
      front = (front+1) % len;
    }
    totActiveElements = (totActiveElements < len) ? totActiveElements + 1 : len;
  }

  // inserts data after last active element in the list (at front)
  void push_back(const T& data);
  void push_back(T&& data);
  // removes oldest active data from the list (from rear) and returns it
  T pop_front();
  // returns the ith active element from the list
  const T& operator[](size_t index) const;
  T& operator[](size_t index);
  
  std::vector<T> val;
  size_t rear, front, totActiveElements, len;
  SERIALIZE(val,rear,front,totActiveElements,len)
};

template <class T>
void cir_array_list<T>::push_back(const T& data) {
  push_back_impl();
  val[front] = data;
}

template <class T>
void cir_array_list<T>::push_back(T&& data) {
  push_back_impl();
  val[front] = std::move(data);
}

template <class T>
T cir_array_list<T>::pop_front() {
  if(isEmpty()) REPORT_ERROR(USER_ERROR,"Invalid operation: Empty list!\n");
  T ret = val[rear];
  if (rear == front) {rear = front = -1;}
  else rear = (rear + 1) % len;
  totActiveElements--;
  return ret;
}

template <class T>
const T& cir_array_list<T>::operator[] (size_t index) const {
  std::string msg = ITOS(index) + ": Invalid index!\n";
  if(index >= totActiveElements) REPORT_ERROR(USER_ERROR,msg);
  size_t a_index = (rear + index) % len;
  return val[a_index];
}

template <class T>
T& cir_array_list<T>::operator[] (size_t index) {
  std::string msg = ITOS(index) + ": Invalid index!\n";
  if(index >= totActiveElements) REPORT_ERROR(USER_ERROR,msg);
  size_t a_index = (rear + index) % len;
  return val[a_index];
}

}

#endif
