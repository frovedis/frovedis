
#ifndef _LDA_CGS_CONFIG_HPP_
#define _LDA_CGS_CONFIG_HPP_

#include "boost/serialization/map.hpp"
#include "frovedis/matrix/rowmajor_matrix.hpp"
#include "frovedis/matrix/crs_matrix.hpp"
#include "frovedis/core/frovedis_init.hpp"
#include "frovedis/core/node_local.hpp"
#include "frovedis/core/dvector.hpp"
#include "frovedis/core/radix_sort.hpp"
#include "frovedis/dataframe/set_operations.hpp"
#include "frovedis/core/mpihelper.hpp"
// #include "typed_funcs.hpp"
#include <map>
#include <stdlib.h>
#include <vector>
#include <math.h>
#include <chrono>

#if (defined(_SX) || defined(__ve__))
#include <asl.h>
#include <ftrace.h>
#endif

//#define LDA_CGS_SPARSE_MODEL
// #define LDA_CGS_TIMING
// #define LDA_CGS_TIMING_SAVE
#define LDA_CGS_VLEN 256
#if !(defined(LDA_MH_STEP))
#define LDA_MH_STEP 2
#endif
// #define LDA_CGS_USE_VREG

namespace frovedis {
namespace lda {

class lda_config{
public:
    enum {
        original_cgs = 0, //original LDA
        dp_cgs = 1,       //doc-proposal LDA
        wp_cgs = 2,       //word-proposal LDA
        cp_cgs = 3,       //cycle-proposal LDA
        sparse_cgs = 4    //sparse LDA
    };

    double beta;
    double alpha;
    double betaW;
    double alphaK;
    size_t num_voc;
    size_t num_topics;
    size_t max_token;
    int exp_iter;
    int num_itr;
    int algorithm;
    bool train;
    bool delay_update;
    
    lda_config(){}
    ~lda_config(){}
    
    lda_config(double beta, double alpha, size_t num_voc, size_t num_topics, size_t max_token, bool delay_update,
        std::string algorithm, int exp_iter, bool train, int num_itr){
        this->alpha = alpha; 
        this->beta  = beta; 
        this->alphaK= alpha*(double)num_topics;
        this->betaW = beta*(double)num_voc;
        this->num_voc = num_voc;
        this->num_topics = num_topics;
        this->max_token = max_token;
        this->train = train;
        this->exp_iter = exp_iter;
        if(algorithm=="original") this->algorithm = original_cgs;
        else if (algorithm=="dp")this->algorithm = dp_cgs;
        else if (algorithm=="wp")this->algorithm = wp_cgs;
        else if (algorithm=="cp")this->algorithm = cp_cgs;
        else if (algorithm=="sparse")this->algorithm = sparse_cgs;
        this->num_itr = num_itr;
        this->delay_update = delay_update;
    }
    SERIALIZE(alpha,beta,alphaK,betaW,num_voc,num_topics,max_token,train,algorithm,exp_iter,delay_update)
};

}
}

#endif /* _LDA_CGS_CONFIG_HPP_ */

