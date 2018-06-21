#ifndef _ALS_H_
#define _ALS_H_

#include <iostream>
#include <string>

#include "../../matrix/crs_matrix.hpp"
#include "matrix_factorization_model.hpp"
#include "optimizer.hpp"

namespace frovedis {

class matrix_factorization_using_als {
public:
  template <typename T>
  static matrix_factorization_model<T> 
  train (crs_matrix<T>& data, 
         size_t factor, 
         int numIter = 100, 
         double alpha = 0.01, 
         double regParam = 0.01, 
         size_t seed = 0);
			
private:
  template <typename T>
  static std::vector<T>
  dtrain(crs_matrix_local<T>& data, std::vector<T>& modelMat,
         optimizer& alsOPT) {
    auto optimizedModelMat = alsOPT.optimize(data, modelMat);
    return optimizedModelMat;
  }
};

template <typename T>
matrix_factorization_model<T>
matrix_factorization_using_als::train(crs_matrix<T>& data,
                                      size_t factor,  
                                      int numIter,
                                      double alpha,
                                      double regParam,
                                      size_t seed) {
  checkAssumption(factor>0 && numIter>0 && alpha>0.0 && regParam>0.0);

  size_t nRows = data.num_row;
  size_t nCols = data.num_col;
	
  // Creating a new MatrixfactorizationModel object,
  // initialized with random value
  matrix_factorization_model<T> initModel(nRows, nCols, factor, seed);

  auto transData = data.transpose();

  // Creation of the optimizer object to optimize the model
  optimizer alsOpt(factor, alpha, regParam);
  auto distOPT = frovedis::make_node_local_broadcast(alsOpt);

  frovedis::time_spent t(DEBUG), t2(DEBUG);
  for (int i = 1; i <= numIter; i++) {
    auto distModelY = frovedis::make_node_local_broadcast(initModel.Y);
    auto locModelX = data.data.map(dtrain<T>, distModelY, distOPT);
    initModel.X = locModelX.template moveto_dvector<T>().gather();

    auto distModelX = frovedis::make_node_local_broadcast(initModel.X);
    auto locModelY = transData.data.map(dtrain<T>, distModelX,
                                        distOPT);
    initModel.Y = locModelY.template moveto_dvector<T>().gather(); 
    t.show("one iteration: ");
  }
  t2.show("all iteration: ");
	  
  return initModel;     
}

}
#endif
