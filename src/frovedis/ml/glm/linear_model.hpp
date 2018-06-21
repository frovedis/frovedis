#ifndef _LINEAR_MODEL_HPP_
#define _LINEAR_MODEL_HPP_

#include <iostream>
#include <fstream>
#include <iomanip>
#include <vector>
#include <string>

#define YES_RESPONSE 1
#define NO_RESPONSE -1

namespace frovedis {

template <class T> struct linear_regression_model; // forward declaration

template <class T, class MODEL>
void glm_save(const MODEL& m, 
              const std::string &outputPath) {
  //std::cout << "In generic model::save() \n";
  std::ofstream outFile;
  outFile.open(outputPath.c_str(), std::ofstream::out);
  outFile << std::setprecision(12);
  for(size_t i = 0; i < m.weight.size(); i++) {
    outFile << m.weight[i] << "\n";
  }
  outFile << m.intercept << std::endl;
  outFile << m.threshold << std::endl;
}

template <class T>
void glm_save(const linear_regression_model<T>& m, 
              const std::string &outputPath) {
  //std::cout << "In linear regression model::save() \n";
  std::ofstream outFile;
  outFile.open(outputPath.c_str(), std::ofstream::out);
  outFile << std::setprecision(12);
  for(size_t i = 0; i < m.weight.size(); i++) {
    outFile << m.weight[i] << "\n";
  }
  outFile << m.intercept << std::endl;
}

template <class T, class MODEL>
void glm_savebinary(const MODEL& m, 
                    const std::string &outputPath) {
  //std::cout << "In generic model::savebinary() \n";
  std::vector<T> tmp(m.weight.size() + 2);
  T* tp = &tmp[0];
  const T* wp = &m.weight[0];
  for(size_t i = 0; i < m.weight.size(); i++) tp[i] = wp[i];
  tp[m.weight.size()] = m.intercept;
  tp[m.weight.size() + 1] = m.threshold;
  make_dvector_scatter(tmp).savebinary(outputPath);
}

template <class T>
void glm_savebinary(const linear_regression_model<T>& m, 
                    const std::string &outputPath) {
  //std::cout << "In linear regression model::savebinary() \n";
  std::vector<T> tmp(m.weight.size() + 1);
  T* tp = &tmp[0];
  const T* wp = &m.weight[0];
  for(size_t i = 0; i < m.weight.size(); i++) tp[i] = wp[i];
  tp[m.weight.size()] = m.intercept;
  make_dvector_scatter(tmp).savebinary(outputPath);
}

template <class T, class MODEL>
void glm_load(MODEL& m, 
              const std::string &inputPath) {
  //std::cout << "In generic model::load() \n";
  std::ifstream inFile(inputPath.c_str());
  m.weight.clear();
  for(T x; inFile >> x;) {
    m.weight.push_back(x);
  }
  m.threshold = m.weight[m.weight.size() - 1];
  m.weight.pop_back();
  m.intercept = m.weight[m.weight.size() - 1];
  m.weight.pop_back();
}

template <class T>
void glm_load(linear_regression_model<T>& m, 
              const std::string &inputPath) {
  //std::cout << "In linear regression model::load() \n";
  std::ifstream inFile(inputPath.c_str());
  m.weight.clear();
  for(T x; inFile >> x;) {
    m.weight.push_back(x);
  }
  m.intercept = m.weight[m.weight.size() - 1];
  m.weight.pop_back();
}

template <class T, class MODEL>
void glm_loadbinary(MODEL& m, 
                    const std::string &inputPath) {
  //std::cout << "In generic model::loadbinary() \n";
  auto d = make_dvector_loadbinary<T>(inputPath);
  m.weight = d.gather();
  m.threshold = m.weight[m.weight.size() - 1];
  m.weight.pop_back();
  m.intercept = m.weight[m.weight.size() - 1];
  m.weight.pop_back();
}

template <class T>
void glm_loadbinary(linear_regression_model<T>& m, 
                    const std::string &inputPath) {
  //std::cout << "In linear regression model::loadbinary() \n";
  auto d = make_dvector_loadbinary<T>(inputPath);
  m.weight = d.gather();
  m.intercept = m.weight[m.weight.size() - 1];
  m.weight.pop_back();
}

template <class T, class MODEL>
void glm_debug_print(const MODEL& m) {
  //std::cout << "In generic model::debug_print() \n";
  std::cout << "-------- Weight Vector:: --------\n";
  for(size_t i = 0; i < m.weight.size(); i++)
    std::cout << " " << m.weight[i];
  std::cout << "\n";
  std::cout << "Intercept:: "   << m.intercept   << std::endl;
  std::cout << "Threshold:: "   << m.threshold   << std::endl;
}

template <class T>
void glm_debug_print(const linear_regression_model<T>& m) {
  //std::cout << "In linear regression model::debug_print() \n";
  std::cout << "-------- Weight Vector:: --------\n";
  for(size_t i = 0; i < m.weight.size(); i++)
    std::cout << " " << m.weight[i];
  std::cout << "\n";
  std::cout << "Intercept:: "   << m.intercept   << std::endl;
}

template <class T, class MODEL>
struct classification_model_bcast_helper {
  classification_model_bcast_helper() {}
  classification_model_bcast_helper(T icpt, T thr) :
    intercept(icpt), threshold(thr) {}
  MODEL operator()(std::vector<T>& w) {
    MODEL r;
    r.weight.swap(w);
    r.intercept = intercept;
    r.threshold = threshold;
    return r;
  }
  T intercept;
  T threshold;
  SERIALIZE(intercept, threshold)
};

template <class T>
struct regression_model_bcast_helper {
  regression_model_bcast_helper() {}
  regression_model_bcast_helper(T icpt): intercept(icpt) {}
  linear_regression_model<T> operator()(std::vector<T>& w) {
    linear_regression_model<T> r;
    r.weight.swap(w);
    r.intercept = intercept;
    return r;
  }
  T intercept;
  SERIALIZE(intercept)
};

template <class T, class MODEL>
node_local<MODEL> 
glm_broadcast(MODEL& m) {
  //std::cout << "In generic model::broadcast() \n";
  auto bweight = make_node_local_broadcast(m.weight);
  return bweight.template map<MODEL>
    (classification_model_bcast_helper<T,MODEL>(m.intercept, m.threshold));
}

template <class T>
node_local<linear_regression_model<T>> 
glm_broadcast(linear_regression_model<T>& m) {
  //std::cout << "In linear regression model::broadcast() \n";
  auto bweight = make_node_local_broadcast(m.weight);
  return bweight.template map<linear_regression_model<T>>
    (regression_model_bcast_helper<T>(m.intercept));
}

template <class T>
struct linear_regression_model {
  linear_regression_model(): intercept(0.0) {}
  linear_regression_model(size_t numFtr, T intercpt=0.0) :
    weight(numFtr,0), intercept(intercpt) {} 
  linear_regression_model(const linear_regression_model<T>& l) {
    weight = l.weight;
    intercept = l.intercept;
  }
  linear_regression_model(linear_regression_model<T>&& l) {
    weight.swap(l.weight);
    intercept = l.intercept;
  }
  linear_regression_model<T>&
  operator=(const linear_regression_model<T>& l) {
    weight = l.weight;
    intercept = l.intercept;
    return *this;
  }
  linear_regression_model<T>&
  operator=(linear_regression_model<T>&& l) {
    weight.swap(l.weight);
    intercept = l.intercept;
    return *this;
  }
  linear_regression_model<T>
  operator+(const linear_regression_model<T>& right_m) const {
    checkAssumption(weight.size() == right_m.weight.size());
    size_t numFtr = weight.size();
    linear_regression_model<T> ret(numFtr);
    ret.intercept = intercept + right_m.intercept;
    for(size_t i = 0; i < numFtr; i++) {
      ret.weight[i] = weight[i] + right_m.weight[i];
    }
    return ret;
  }
  linear_regression_model<T>
  operator-(const linear_regression_model<T>& right_m) const { 
    checkAssumption(weight.size() == right_m.weight.size());
    size_t numFtr = weight.size();
    linear_regression_model<T> ret(numFtr);
    ret.intercept = intercept - right_m.intercept;
    for(size_t i = 0; i < numFtr; i++) {
      ret.weight[i] = weight[i] - right_m.weight[i];
    }
    return ret;
  }
  void operator+=(const linear_regression_model<T>& right_m) {
    checkAssumption(weight.size() == right_m.weight.size()); 
    intercept += right_m.intercept;
    for(size_t i = 0; i < weight.size(); i++) {
      weight[i] += right_m.weight[i];
    }
  }
  void operator-=(const linear_regression_model<T>& right_m) {
    checkAssumption(weight.size() == right_m.weight.size());
    intercept -= right_m.intercept;
    for(size_t i = 0; i < weight.size(); i++) {
      weight[i] -= right_m.weight[i];
    }
  }
  template <class DATA_MATRIX>
  std::vector<T>
  predict(DATA_MATRIX& mat) {
    auto v = mat * weight;
    T* vp = &v[0];
    for(size_t i = 0; i < v.size(); i++) {
      vp[i] += intercept;
    }
    return v;
  }
  size_t get_num_features() const {return weight.size();}
  void save(const std::string &inputPath) const {
    glm_save<T>(*this,inputPath);
  }
  void savebinary(const std::string &inputPath) const {
    glm_savebinary<T>(*this,inputPath);
  }
  void load(const std::string &inputPath) {
    glm_load<T>(*this,inputPath);
  }
  void loadbinary(const std::string &inputPath) {
    glm_loadbinary<T>(*this,inputPath);
  }
  void debug_print() const {
    glm_debug_print<T>(*this);
  }
  node_local<linear_regression_model<T>> broadcast() {
    return glm_broadcast<T>(*this);
  }

  std::vector<T> weight;
  T intercept;
  SERIALIZE(weight, intercept)
};

template <class T>
struct logistic_regression_model {
  // default threshold = 0.5
  logistic_regression_model(): intercept(0.0), threshold(0.5) {} 
  logistic_regression_model(size_t numFtr, T intercpt=0.0, T thr=0.5):
    weight(numFtr,0), intercept(intercpt), threshold(thr) 
    { checkAssumption(thr >= 0); }
  logistic_regression_model(const logistic_regression_model<T>& l) {
    weight = l.weight;
    intercept = l.intercept;
    threshold = l.threshold;
  }
  logistic_regression_model(logistic_regression_model<T>&& l) {
    weight.swap(l.weight);
    intercept = l.intercept;
    threshold = l.threshold;
  }
  logistic_regression_model<T>&
  operator=(const logistic_regression_model<T>& l) {
    weight = l.weight;
    intercept = l.intercept;
    threshold = l.threshold;
    return *this;
  }
  logistic_regression_model<T>&
  operator=(logistic_regression_model<T>&& l) {
    weight.swap(l.weight);
    intercept = l.intercept;
    threshold = l.threshold;
    return *this;
  }
  logistic_regression_model<T>
  operator+(const logistic_regression_model<T>& right_m) const {  
    checkAssumption(weight.size() == right_m.weight.size() && 
                    threshold == right_m.threshold);
    size_t numFtr = weight.size();
    logistic_regression_model<T> ret(numFtr);
    ret.intercept = intercept + right_m.intercept;
    for(size_t i = 0; i < numFtr; i++) {
      ret.weight[i] = weight[i] + right_m.weight[i];
    }
    ret.threshold = threshold;
    return ret;
  }
  logistic_regression_model<T>
  operator-(const logistic_regression_model<T>& right_m) const {
    checkAssumption(weight.size() == right_m.weight.size() && 
                    threshold == right_m.threshold);
    size_t numFtr = weight.size();
    logistic_regression_model<T> ret(numFtr);
    ret.intercept = intercept - right_m.intercept;
    for(size_t i = 0; i < numFtr; i++) {
      ret.weight[i] = weight[i] - right_m.weight[i];
    }
    ret.threshold = threshold;
    return ret;
  }
  void operator+=(const logistic_regression_model<T>& right_m) {
    checkAssumption(weight.size() == right_m.weight.size() && 
                    threshold == right_m.threshold);
    intercept += right_m.intercept;
    for(size_t i = 0; i < weight.size(); i++) {
      weight[i] += right_m.weight[i];
    }
  }
  void operator-=(const logistic_regression_model<T>& right_m) {
    checkAssumption(weight.size() == right_m.weight.size() && 
                    threshold == right_m.threshold);
    intercept -= right_m.intercept;
    for(size_t i = 0; i < weight.size(); i++) {
      weight[i] -= right_m.weight[i];
    }
  }
  template <class DATA_MATRIX>
  std::vector<T>
  predict(DATA_MATRIX& mat) {
    auto tmp = predict_probability(mat);
    std::vector<T> ret(tmp.size());
    T* tmpp = &tmp[0];
    T* retp = &ret[0];
    for(size_t i = 0; i < tmp.size(); ++i) {
      retp[i] = (tmpp[i] >= threshold) ? YES_RESPONSE : NO_RESPONSE;
    } 
    return ret;
  }
  template <class DATA_MATRIX>
  std::vector<T>
  predict_probability(DATA_MATRIX& mat) {
    auto v = mat * weight;
    std::vector<T> ret(v.size());
    T* vp = &v[0];
    T* retp = &ret[0];
    for(size_t i = 0; i < v.size(); i++) {
      auto exptmp = -(vp[i] + intercept);
      // for SX exp...
      if(exptmp > 709) retp[i] = 0;
      else retp[i] = 1.0 / (1.0 + exp(exptmp));
    }
    return ret;
  }
  void set_threshold(T thr) {
    if(thr < 0) REPORT_ERROR(USER_ERROR,"Negative Threshold Value\n");
    threshold = thr;
  }
  size_t get_num_features() const {return weight.size();}
  void save(const std::string &inputPath) const {
    glm_save<T,logistic_regression_model<T>>(*this,inputPath);
  }
  void savebinary(const std::string &inputPath) const {
    glm_savebinary<T,logistic_regression_model<T>>(*this,inputPath);
  }
  void load(const std::string &inputPath) {
    glm_load<T,logistic_regression_model<T>>(*this,inputPath);
  }
  void loadbinary(const std::string &inputPath) {
    glm_loadbinary<T,logistic_regression_model<T>>(*this,inputPath);
  }
  void debug_print() const {
    glm_debug_print<T,logistic_regression_model<T>>(*this);
  }
  node_local<logistic_regression_model<T>> broadcast() {
    return glm_broadcast<T,logistic_regression_model<T>>(*this);
  }

  std::vector<T> weight;
  T intercept, threshold;
  SERIALIZE(weight, intercept, threshold)
};

template <class T>
struct svm_model {
  // default threshold = 0.0
  svm_model(): intercept(0.0), threshold(0.0) {} 
  svm_model(size_t numFtr, T intercpt=0.0, T thr=0.0) :
    weight(numFtr,0), intercept(intercpt), threshold(thr) 
    { checkAssumption(thr >= 0); }
  svm_model(const svm_model<T>& l) {
    weight = l.weight;
    intercept = l.intercept;
    threshold = l.threshold;
  }
  svm_model(svm_model<T>&& l) {
    weight.swap(l.weight);
    intercept = l.intercept;
    threshold = l.threshold;
  }
  svm_model<T>&
  operator=(const svm_model<T>& l) {
    weight = l.weight;
    intercept = l.intercept;
    threshold = l.threshold;
    return *this;
  }
  svm_model<T>&
  operator=(svm_model<T>&& l) {
    weight.swap(l.weight);
    intercept = l.intercept;
    threshold = l.threshold;
    return *this;
  }
  svm_model<T>
  operator+(const svm_model<T>& right_m) const {
    checkAssumption(weight.size() == right_m.weight.size() &&
                    threshold == right_m.threshold);
    size_t numFtr = weight.size();
    svm_model<T> ret(numFtr);
    ret.intercept = intercept + right_m.intercept;
    for(size_t i = 0; i < numFtr; i++) {
      ret.weight[i] = weight[i] + right_m.weight[i];
    }
    ret.threshold = threshold;
    return ret;
  }
  svm_model<T>
  operator-(const svm_model<T>& right_m) const { 
    checkAssumption(weight.size() == right_m.weight.size() &&
                    threshold == right_m.threshold);
    size_t numFtr = weight.size();
    svm_model<T> ret(numFtr);
    ret.intercept = intercept - right_m.intercept;
    for(size_t i = 0; i < numFtr; i++) {
      ret.weight[i] = weight[i] - right_m.weight[i];
    }
    ret.threshold = threshold;
    return ret;
  }
  void operator+=(const svm_model<T>& right_m) {
    checkAssumption(weight.size() == right_m.weight.size() &&
                    threshold == right_m.threshold);
    intercept += right_m.intercept;
    for(size_t i = 0; i < weight.size(); i++) {
      weight[i] += right_m.weight[i];
    }
  }
  void operator-=(const svm_model<T>& right_m) {
    checkAssumption(weight.size() == right_m.weight.size() &&
                    threshold == right_m.threshold);
    intercept -= right_m.intercept;
    for(size_t i = 0; i < weight.size(); i++) {
      weight[i] -= right_m.weight[i];
    }
  }
  template <class DATA_MATRIX>
  std::vector<T>
  predict(DATA_MATRIX& mat) {
    auto tmp = predict_probability(mat);
    std::vector<T> ret(tmp.size());
    T* tmpp = &tmp[0];
    T* retp = &ret[0];
    for(size_t i = 0; i < tmp.size(); i++) {
      retp[i] = (tmpp[i] >= threshold) ? YES_RESPONSE : NO_RESPONSE;
    }
    return ret;
  }
  template <class DATA_MATRIX>
  std::vector<T>
  predict_probability(DATA_MATRIX& mat) {
    auto v = mat * weight;
    T* vp = &v[0];
    for(size_t i = 0; i < v.size(); i++) {
      vp[i] += intercept;
    }
    return v;
  }
  void set_threshold(T thr) {
    if(thr < 0) REPORT_ERROR(USER_ERROR,"Negative Threshold Value\n");
    threshold = thr;
  }
  size_t get_num_features() const {return weight.size();}
  void save(const std::string &inputPath) const {
    glm_save<T,svm_model<T>>(*this,inputPath);
  }
  void savebinary(const std::string &inputPath) const {
    glm_savebinary<T,svm_model<T>>(*this,inputPath);
  }
  void load(const std::string &inputPath) {
    glm_load<T,svm_model<T>>(*this,inputPath);
  }
  void loadbinary(const std::string &inputPath) {
    glm_loadbinary<T,svm_model<T>>(*this,inputPath);
  }
  void debug_print() const {
    glm_debug_print<T,svm_model<T>>(*this);
  }
  node_local<svm_model<T>> broadcast() {
    return glm_broadcast<T,svm_model<T>>(*this);
  }

  std::vector<T> weight;
  T intercept, threshold;
  SERIALIZE(weight, intercept, threshold)
};

}
#endif
