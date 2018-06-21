#ifndef _BLOCK_CYCLIC_MATRIX_
#define _BLOCK_CYCLIC_MATRIX_

#include <vector>
#include <string>
#include <cmath>
#include <algorithm>
#include <boost/lexical_cast.hpp>
#include "../core/exceptions.hpp"
#include "scalapack_redist.hpp"
#include "colmajor_matrix.hpp"

#define ITOS boost::lexical_cast<std::string>
#define ARRAY_DESCRIPTOR_SIZE 11
#define VR_BSIZE 256

extern "C" {
  void sl_init_(int* ICONTEXT, int* NPROW, int* NPCOL);

  void blacs_gridinfo_(int* ICONTEXT, 
                       int* NPROW, int* NPCOL,
                       int* MYROW, int* MYCOL);

  int numroc_(int* M_, int* M_B, int* M, int* M_SRC, int* M_PROC);

  void descinit_(int* DESC_A, int* M_A, int* N_A,
                 int* MB_A, int* NB_A,
                 int* RSRC_A, int* CSRC_A, 
                 int* ICONTEXT, int* LLD_A, int* INFO);

  void blacs_gridexit_(int* ICONTEXT);
  void blacs_exit_(int* STAT);
}

namespace frovedis {

// enum for array descriptor field
enum {
  DTYPE_A = 0,
  CTXT_A,
  M_A,
  N_A,
  MB_A,
  NB_A,
  RSRC_A,
  CSRC_A,
  LLD_A
};

template <class T>
struct blockcyclic_matrix_local {
  blockcyclic_matrix_local() {
    set_local_num(0,0,2);
  }
  blockcyclic_matrix_local(size_t nrow, size_t ncol, size_t t=2) { 
    val.resize(nrow*ncol,0);
    set_local_num(nrow,ncol,t);
  }
  blockcyclic_matrix_local(blockcyclic_matrix_local<T>&& m) {
    val.swap(m.val);
    descA.swap(m.descA);
    local_num_row = m.local_num_row;
    local_num_col = m.local_num_col;    
    type = m.type;
  }
  blockcyclic_matrix_local<T>& 
  operator=(blockcyclic_matrix_local<T>&& m) {
    val.swap(m.val);
    descA.swap(m.descA);
    local_num_row = m.local_num_row;
    local_num_col = m.local_num_col;        
    type = m.type;
    return *this;
  }
  blockcyclic_matrix_local(const blockcyclic_matrix_local<T>& m) {
    val = m.val;
    descA = m.descA;
    local_num_row = m.local_num_row;
    local_num_col = m.local_num_col;    
    type = m.type;
  }
  blockcyclic_matrix_local<T>& 
  operator=(const blockcyclic_matrix_local<T>& m) {
    val = m.val;
    descA = m.descA;
    local_num_row = m.local_num_row;
    local_num_col = m.local_num_col;    
    type = m.type;
    return *this;
  }
  void set_local_num(size_t nrow, size_t ncol, size_t t=2) {
     local_num_row = nrow;
     local_num_col = ncol;
     type = t;
  }
  void set_descriptor(const std::vector<size_t>& desc) {
    for(size_t i = 0; i < descA.size(); i++) descA[i] = desc[i];
  }
  bool is_valid() const {
    return ((val.size() == local_num_row * local_num_col) && 
            (descA.size() == ARRAY_DESCRIPTOR_SIZE) && 
            (type >= 1 && type <= 2));
  }
  void debug_print() const {
    std::cout << "node = " << get_selfid()
              << ", local_num_row = " << local_num_row
              << ", local_num_col = " << local_num_col
              << ", type = " << type
              << ", descriptor = "; 
    for(auto i: descA){ std::cout << i << " "; }
    std::cout << "\nval = ";
    for(auto i: val){ std::cout << i << " "; }
    std::cout << std::endl;
  }
  std::vector<T> val;
  std::vector<int> descA;
  size_t local_num_row, local_num_col, type;
  SERIALIZE(val, descA, local_num_row, local_num_col, type)
};


/*
   distributed version
*/

//----------------------------------------------------------------------------
// Name: scalapack_distributor_engine<T>
// 
// Description: A distributor engine program which has the global number of rows 
// and global number cols information to be distributed over a process grid.
//
// How to start: This engine will automatically get started whenever any of the
// below mentioned operation will be requested by the user for the first time. 
//
// How to stop: To stop this engine, user has to invoke 
// its "stop" function, scalapack_distributor_engine<T>::stop()
//
// Input - Output of this engine program is described below in short:
// (1) Input: No input (actually a dummy boolean)
//  => Output: An empty distributed blockcyclic matrix with global dimension(nrow,ncol)
// (2) Input: colmajor matrix with global dimension(nrow, ncol)
//  => Output: corresponding blockcyclic matrix with global dimension(nrow,ncol)
// (3) Input: blockcyclic matrix with global dimension(nrow, ncol)
//  => Output: corresponding colmajor matrix with global dimension(nrow,ncol)
//----------------------------------------------------------------------------
template <class T>
struct scalapack_distributor_engine {
  private:
   static bool cmm_grid_created, bcm_grid_created;
   static int cmm_context, bcm_context;
   int nrow, ncol, MB_, NB_;

  public:
   scalapack_distributor_engine() : nrow(0), ncol(0), MB_(0), NB_(0) {}
   scalapack_distributor_engine(int nr, int nc, int mb = 0, int nb = 0) : 
                                nrow(nr), ncol(nc), MB_(mb), NB_(nb) {}

   // Operation (1): Returns an empty blockcycic_matrix_local<T>
   blockcyclic_matrix_local<T> 
   operator() (bool dummy, int type=2) {
      // quick return
      if(nrow == 0 && ncol == 0)
        return blockcyclic_matrix_local<T>(0,0,type);

      checkAssumption( nrow > 0 && ncol > 0 && type >= 1 && type <= 2);

      std::vector<int> desc;
      if(type == 1) {
#ifdef _DEBUG_STAT_
        std::cout << "invoking engine operation (1.1)\n";
#endif 
        if(!cmm_grid_created) create_grid(1);
        desc = create_array_descriptor(cmm_context,1);
      }
      else {
#ifdef _DEBUG_STAT_
        std::cout << "invoking engine operation (1.2)\n";
#endif 
        if(!bcm_grid_created) create_grid(2);
        desc = create_array_descriptor(bcm_context,2);
      }
      int local_num_row = desc[ARRAY_DESCRIPTOR_SIZE-2];
      int local_num_col = desc[ARRAY_DESCRIPTOR_SIZE-1];
      blockcyclic_matrix_local<T> outMat(local_num_row,local_num_col,type);
      outMat.descA.swap(desc);
      return outMat;
   }

   // Operation (2): Returns re-distributed blockcycic_matrix_local<T>
   blockcyclic_matrix_local<T> 
   operator() (colmajor_matrix_local<T>& inMat,
               int type=2, bool isMovable=false) {
      // quick return
      if(nrow == 0 && ncol == 0 && inMat.val.size()==0)
        return blockcyclic_matrix_local<T>(0,0,type);

     checkAssumption(nrow > 0  && ncol > 0 && type >= 1 && type <= 2);

     if(!cmm_grid_created) create_grid(1);
     std::vector<int> desc = create_array_descriptor(cmm_context,1);

     // tempMat is actually the input colmajor_matrix_local<T> with array descriptor
     blockcyclic_matrix_local<T> tempMat;
     if(isMovable) tempMat.val.swap(inMat.val);
     else tempMat.val = inMat.val;
     tempMat.descA.swap(desc);
     tempMat.set_local_num(inMat.local_num_row,inMat.local_num_col,1); // grid=1

     if(type == 1) {
#ifdef _DEBUG_STAT_
       std::cout << "invoking engine operation (2.1)\n";
#endif 
       return tempMat;
     }
     else {
#ifdef _DEBUG_STAT_
       std::cout << "invoking engine operation (2.2)\n";
#endif 
       blockcyclic_matrix_local<T> outMat = cmm_to_bcm(tempMat);
       return outMat;
     }
   }

   // Operation (3): Returns re-distributed colmajor_matrix_local<T>
   colmajor_matrix_local<T> 
   operator() (blockcyclic_matrix_local<T>& inMat,bool isMovable=false) {
     // quick return
     if(nrow == 0 && ncol == 0 && inMat.val.size()==0)
       return colmajor_matrix_local<T>(0,0);

     checkAssumption(nrow > 0 && ncol > 0);

     colmajor_matrix_local<T> outMat;
     if (inMat.type == 1){
#ifdef _DEBUG_STAT_
     std::cout << "invoking engine operation (3.1)\n";
#endif 
       if(isMovable) outMat.val.swap(inMat.val);
       else outMat.val = inMat.val;
       outMat.local_num_row = inMat.local_num_row;
       outMat.local_num_col = inMat.local_num_col;
     }
     else {
#ifdef _DEBUG_STAT_
     std::cout << "invoking engine operation (3.2)\n";
#endif 
       blockcyclic_matrix_local<T> tempMat = bcm_to_cmm(inMat);
       outMat.val.swap(tempMat.val);
       outMat.local_num_row = tempMat.local_num_row;
       outMat.local_num_col = tempMat.local_num_col;
     }
     return outMat;
   }

  // the function to stop the engine (if it is started)
  static void stop() {
    int nproc = get_nodesize();
    int nprow = std::sqrt(nproc);
    int npcol = nproc / nprow;
    // only process in bcm_grid, will invoke this exit call
    if(bcm_grid_created && (get_selfid() < nprow*npcol)) 
      blacs_gridexit_(&bcm_context);

    // considering (nrow > nproc), all processes are part of cmm_grid
    if(cmm_grid_created) 
      blacs_gridexit_(&cmm_context);
   
    //resetting static members
    cmm_grid_created = bcm_grid_created = false;
    cmm_context = bcm_context = -1;

    int stat = 0;
    blacs_exit_(&stat);
  }
  
// --- PRIVATE SECTION ---
  private:
//------------------------

   // input : blockcyclic matrix with NPROC x 1 grid (colmajor matrix)
   // output: blockcyclic matrix with NPROW x NPCOL grid
   blockcyclic_matrix_local<T> 
   cmm_to_bcm(blockcyclic_matrix_local<T>& inMat) {
      if(!check_input_validity(inMat)) 
        REPORT_ERROR(USER_ERROR,
	      "Input matrix size is incompatible with input global size!!\n");

      if (!cmm_grid_created) 
        REPORT_ERROR(USER_ERROR,"Invalid call: NPROC x 1 grid not created!!\n");

      if (!bcm_grid_created) create_grid(2);
      std::vector<int> desc = create_array_descriptor(bcm_context,2);

      int local_num_row = desc[ARRAY_DESCRIPTOR_SIZE-2];
      int local_num_col = desc[ARRAY_DESCRIPTOR_SIZE-1];
      blockcyclic_matrix_local<T> outMat(local_num_row,local_num_col,2); //grid=2

      // === Re-distribution colmajor matrix => blockcyclic matrix ===
      // https://software.intel.com/en-us/node/521674
      // The "context" encompassing at least the union of all processes in 
      // cmm_context and bcm_context should be passed in this routine call. 
      // Here cmm_context covers all the processes. Thus, it is passed.
      // All processes in the passed "context" must call this routine,
      // even if they do not own a piece of either matrix.
      int IJ = 1;
      T* aptr = &inMat.val[0];
      T* bptr = &outMat.val[0];
      int* DESCA = &inMat.descA[0];
      int* DESCB = &desc[0];
      scalapack_redist<T>(&nrow, &ncol,
                          aptr, &IJ, &IJ, DESCA,
                          bptr, &IJ, &IJ, DESCB, &cmm_context);

      outMat.descA.swap(desc);
      return outMat;
   }

   // input : blockcyclic matrix with NPROW x NPCOL grid 
   // output: blockcyclic matrix with NPROC x 1 grid (colmajor matrix)
   blockcyclic_matrix_local<T> 
   bcm_to_cmm(blockcyclic_matrix_local<T>& inMat) {
      if(!check_input_validity(inMat)) 
        REPORT_ERROR(USER_ERROR,
	      "Input matrix size is incompatible with input global size!!\n");

      if (!bcm_grid_created) 
        REPORT_ERROR(USER_ERROR,"Invalid call: NPROW x NPCOL grid not created!!\n");

      if (!cmm_grid_created) create_grid(1);
      std::vector<int> desc = create_array_descriptor(cmm_context,1);

      int local_num_row = desc[ARRAY_DESCRIPTOR_SIZE-2];
      int local_num_col = desc[ARRAY_DESCRIPTOR_SIZE-1];
      // outMat is actually the output colmajor_matrix_local<T> with array descriptor
      blockcyclic_matrix_local<T> outMat(local_num_row,local_num_col,1); //grid=1

      // === Re-distribution blockcyclic matrix => colmajor matrix ===
      // https://software.intel.com/en-us/node/521674
      // The "context" encompassing at least the union of all processes in 
      // cmm_context and bcm_context should be passed in this routine call. 
      // Here cmm_context covers all the processes. Thus, it is passed.
      // All processes in the passed "context" must call this routine,
      // even if they do not own a piece of either matrix.
      int IJ = 1;
      T* aptr = &inMat.val[0];
      T* bptr = &outMat.val[0];
      int* DESCA = &inMat.descA[0];
      int* DESCB = &desc[0];
      scalapack_redist<T>(&nrow, &ncol, 
                          aptr, &IJ, &IJ, DESCA,
                          bptr, &IJ, &IJ, DESCB, &cmm_context); 

      outMat.descA.swap(desc);
      return outMat;
   }

   // if all the inputs of the functor call are valid for re-distribution,
   // then this function returns true, else it returns false 
   bool check_input_validity(blockcyclic_matrix_local<T>& inMat) {
     if(!inMat.is_valid())
       REPORT_ERROR(USER_ERROR,"Invalid input matrix!!\n");

     int icontext = inMat.descA[CTXT_A];
     int global_row_size = inMat.descA[M_A];
     int global_col_size = inMat.descA[N_A];
     int MB = inMat.descA[MB_A];
     int NB = inMat.descA[NB_A];
     int RSRC = inMat.descA[RSRC_A];
     int CSRC = inMat.descA[CSRC_A];

     bool is_valid = false;
     int nproc = get_nodesize();
     int l_num_row = 0, l_num_col = 0;
     int myrow = -1, mycol = -1;
     int nprow = -1, npcol = -1;
     blacs_gridinfo_(&icontext, &nprow, &npcol, &myrow, &mycol);
   
     if(myrow >= 0 && myrow < nprow &&
        mycol >= 0 && mycol < npcol) { // which are in process grid
       l_num_row = numroc_(&nrow, &MB, &myrow, &RSRC, &nprow);
       l_num_col = numroc_(&ncol, &NB, &mycol, &CSRC, &npcol);
       // adjustment for blockcyclic matrix (as per array descriptor creation)
       if(nprow != nproc) {
         l_num_row = l_num_row > 0 ? l_num_row : 1;
         l_num_col = l_num_col > 0 ? l_num_col : 1;
       }
       // for blockcyclic matrix and colmajor matrix loaded using loadbinary()
       bool t1 = (inMat.local_num_row == l_num_row  &&
                 inMat.local_num_col == l_num_col);
       // for colmajor matrix loaded using load()
       bool t2 = (inMat.local_num_row == 0 && inMat.local_num_col == 0);
       if(nrow == global_row_size && ncol == global_col_size && (t1 || t2))
         is_valid = true;
     }
     else { // which are not in process grid
       if(inMat.val.size() == 0)
         is_valid = true;
     }

#ifdef _BCM_DEBUG_
     std::cout << "[check_input_validity] node: " << get_selfid()
               << ", nrow: " << nrow
               << ", ncol: " << ncol
               << ", global_row_size: " << global_row_size
               << ", global_col_size: " << global_col_size
               << ", context: " << icontext
               << ", local_num_row: " << inMat.local_num_row
               << ", local_num_col: " << inMat.local_num_col
               << ", l_num_row: " << l_num_row
               << ", l_num_col: " << l_num_col << std::endl;
#endif
     return is_valid;
   }

   // to create the requested process grid (only two types are supported)
   // Type 1 Grid: NPROC x 1
   // Type 2 Grid: NPROW x NPCOL
   void create_grid(int type) {
     int node_size = get_nodesize();
     checkAssumption (node_size > 0);
     int nprow = -1, npcol = -1;
     switch(type) {
       case 1: if(!cmm_grid_created) {
                 nprow = node_size;
	         npcol = 1;
	         sl_init_(&cmm_context, &nprow, &npcol);
	         cmm_grid_created = true;
                 if((nrow < node_size) && (get_selfid() == 0)) {
                   std::string msg;
                   msg = "\n #nrow: "  + ITOS(nrow) + 
                         " #nproc: " + ITOS(node_size) + " - ";
                   msg += "Too small problem size (usage of serial version is recommended)!!\n";
                   REPORT_WARNING(WARNING_MESSAGE, msg);
                 }
	       }
	       break;
       case 2: if(!bcm_grid_created) { 
                 nprow = std::sqrt(node_size);
	         npcol = node_size / nprow;
	         sl_init_(&bcm_context, &nprow, &npcol);
	         bcm_grid_created = true;

                 int np_proc = nprow * npcol;
                 if( (np_proc != node_size) && (get_selfid() == 0)) {
	           std::string msg;
                   msg = "\n Total number of processes in NPROW x NPCOL grid: " + 
                          ITOS(np_proc) + "\n";
	           REPORT_WARNING(WARNING_MESSAGE, msg);
	         }
	       }
	       break;
       default: REPORT_ERROR(USER_ERROR,"Unsupported Grid Type!!\n");
     }
   }

   // to create the array descriptor for the requested grid type
   std::vector<int>
   create_array_descriptor(int icontext, int type) {

     int node_size = get_nodesize();
     checkAssumption(node_size > 0);

     std::vector<int> desc(ARRAY_DESCRIPTOR_SIZE,0);
     // https://software.intel.com/en-us/node/521674
     // if the calling process is not a part of the input context, 
     // it must set the descriptor-context-field [1] as -1.
     desc[CTXT_A] = -1;

     // blacs query on on the input context to know 
     // the grid dimension and current process co-ordinates
     int nprow = -1, npcol = -1, myrow = -1, mycol = -1;
     blacs_gridinfo_(&icontext, &nprow, &npcol, &myrow, &mycol);

     int local_num_row = 0, local_num_col = 0;
     int i_zero = 0;
     int LLD = 1, MB = 0, NB = 0, INFO = -1;

     // === The array descriptor initialization ===
     if (type == 1) {
       // All the participating processes must correctly define 
       // all the parameters describing the respective array (colmajor matrix).
       MB = ceil_div(nrow, node_size);
       NB = ncol;
       local_num_row = numroc_(&nrow, &MB, &myrow, &i_zero, &nprow);
       local_num_col = ncol;
       LLD = local_num_row > 0 ? local_num_row : 1;

       descinit_(&desc[0], &nrow, &ncol,
                 &MB, &NB,
                 &i_zero, &i_zero, 
                 &icontext, &LLD, &INFO);
       if(INFO)
         REPORT_ERROR(INTERNAL_ERROR,
                      "Descriptor routine for NPROC x 1 grid aborted!!\n");
     }
     else if(type == 2) {
       if(myrow >= 0 && myrow < nprow &&
          mycol >= 0 && mycol < npcol) { // processes which are in bcm_grid
         // Only processes in this input context must correctly define all parameters
         // describing the blockcyclic matrix.
  
         checkAssumption(MB_ >= 0 && NB_ >= 0);

         if (MB_ == 0 || NB_ == 0) {
            MB = blockset(nrow, nprow, npcol);
            NB = blockset(ncol, nprow, npcol);
         }
         // Using user blocksizes, when both MB and NB is supplied as non-zero
         else { 
            MB = MB_;
            NB = NB_;
         }
         local_num_row = numroc_(&nrow, &MB, &myrow, &i_zero, &nprow);
         local_num_row = local_num_row > 0 ? local_num_row : 1;
         local_num_col = numroc_(&ncol, &NB, &mycol, &i_zero, &npcol);
         local_num_col = local_num_col > 0 ? local_num_col : 1;
         LLD = local_num_row;

         descinit_(&desc[0], &nrow, &ncol,
                   &MB, &NB,
                   &i_zero, &i_zero, 
                   &icontext, &LLD, &INFO);
         if(INFO)
           REPORT_ERROR(INTERNAL_ERROR,
                        "Descriptor routine for NPROW x NPCOL grid aborted!!\n");
       }
       else { // to make it valid for sliced_matrix
         desc[M_A] = nrow;
         desc[N_A] = ncol;
       }
     }
     else
       REPORT_ERROR(USER_ERROR,"Unsupported Grid Type!!\n");

     // NOTE: Processes, not in bcm-grid, will have 0 for the below fields
     desc[ARRAY_DESCRIPTOR_SIZE-2] = local_num_row; 
     desc[ARRAY_DESCRIPTOR_SIZE-1] = local_num_col; 

#ifdef _BCM_DEBUG_
     std::cout << "Descriptor created: type: " << type 
               << " cntxt: " << icontext 
               << " val: ";
     for(int i=0; i<ARRAY_DESCRIPTOR_SIZE; i++) std::cout << desc[i] << " ";
     std::cout << std::endl;
#endif

     return desc;
   }

   // the blockset routine to compute the optimum block-size (MB/NB)
   int blockset(int n, int nprow, int npcol, int user_bsize = VR_BSIZE) {
     int bsize = std::min(ceil_div(n,nprow),ceil_div(n,npcol));
     if(user_bsize > 0) bsize = std::min(bsize, user_bsize);
     bsize = (bsize < 1) ? 1 : bsize;
     return bsize;
   }

   SERIALIZE(nrow, ncol, MB_, NB_, 
             cmm_context, cmm_grid_created,
             bcm_context, bcm_grid_created)
};

// Initialization of the static data members...
template <class T>
int scalapack_distributor_engine<T>::cmm_context = -1;
template <class T>
bool scalapack_distributor_engine<T>::cmm_grid_created = false;

template <class T>
int scalapack_distributor_engine<T>::bcm_context = -1;
template <class T>
bool scalapack_distributor_engine<T>::bcm_grid_created = false;

template <class T>
blockcyclic_matrix_local<T>
copy_local_data(const blockcyclic_matrix_local<T>& m) {
  return blockcyclic_matrix_local<T>(m);
}

template <class T>
void transpose_impl(blockcyclic_matrix_local<T>& inMat,
                    blockcyclic_matrix_local<T>& outMat) {
   T* aptr = &inMat.val[0];
   int* DESCA = &inMat.descA[0];
   T* bptr = &outMat.val[0];
   int* DESCB = &outMat.descA[0];
   int IJA = 1, IJB = 1;
   T ALPHA = 1.0;
   T BETA = 0.0;
   char TRANS = 'T';
   int M = outMat.descA[M_A];
   int N = outMat.descA[N_A];
   geadd_wrapper<T>(&TRANS,
                    &M, &N,
                    &ALPHA,
                    aptr, &IJA, &IJA, DESCA,
                    &BETA,
                    bptr, &IJB, &IJB, DESCB);
}

template <class T>
struct blockcyclic_matrix {
  blockcyclic_matrix() : num_row(0), num_col(0), type(2) {
     auto dummy = frovedis::make_node_local_allocate<bool>();
     data = dummy.template map<blockcyclic_matrix_local<T>>
              (scalapack_distributor_engine<T>(0,0),broadcast(type));
  }
  blockcyclic_matrix(size_t nrow, size_t ncol, size_t t=2) : 
     num_row(nrow), num_col(ncol), type(t) {
     auto dummy = frovedis::make_node_local_allocate<bool>();
     data = dummy.template map<blockcyclic_matrix_local<T>>
              (scalapack_distributor_engine<T>(nrow, ncol),broadcast(type));
  }
  blockcyclic_matrix(frovedis::node_local<blockcyclic_matrix_local<T>>&& d) :
    data(std::move(d)) {}

  blockcyclic_matrix(const blockcyclic_matrix<T>& m) {
#ifdef _DEBUG_STAT_
    std::cout << "invoking copy ctor of blockcyclic-matrix\n";
#endif
    auto& m2 = const_cast<blockcyclic_matrix<T>&>(m);
    data = m2.data.map(copy_local_data<T>);
    set_num(m2.num_row,m2.num_col,m2.type);
  }

  blockcyclic_matrix<T>& operator=(const blockcyclic_matrix<T>& m) {
#ifdef _DEBUG_STAT_
    std::cout << "invoking copy op= of blockcyclic-matrix\n";
#endif
    auto& m2 = const_cast<blockcyclic_matrix<T>&>(m);
    data = m2.data.map(copy_local_data<T>);
    set_num(m2.num_row,m2.num_col,m2.type);
    return *this;
  }

  blockcyclic_matrix(blockcyclic_matrix<T>&& m): data(std::move(m.data)) {
#ifdef _DEBUG_STAT_
    std::cout << "invoking move ctor of blockcyclic-matrix\n";
#endif
    set_num(m.num_row,m.num_col,m.type);
  }
  blockcyclic_matrix<T>& operator=(blockcyclic_matrix<T>&& m) {
#ifdef _DEBUG_STAT_
    std::cout << "invoking move op= of blockcyclic-matrix\n";
#endif
    data = std::move(m.data);
    set_num(m.num_row,m.num_col,m.type);
    return *this;
  }

  // implicit conversion:
  // (lvalue) colmajor_matrix<T> => blockcyclic_matrix<T>
  blockcyclic_matrix(const colmajor_matrix<T>& c_mat,size_t type=2,
                     size_t MB = 0, size_t NB = 0) {
#ifdef _DEBUG_STAT_
    std::cout << "(lvalue) colmajor_matrix -> blockcyclic_matrix (type): " 
              << type << "\n";
#endif
    checkAssumption(type>=1 && type<=2);
    if(type == 1) { MB = NB = 0; } // ignoring user input
    auto& c_mat2 = const_cast<colmajor_matrix<T>&>(c_mat);
    size_t nrow = c_mat2.num_row;
    size_t ncol = c_mat2.num_col;
    data = std::move(c_mat2.data.template map<blockcyclic_matrix_local<T>>
                    (scalapack_distributor_engine<T>(nrow,ncol,MB,NB),
                     broadcast(type),broadcast(false))); 
    set_num(nrow,ncol,type);
  }
  // implicit conversion:
  // (rvalue) colmajor_matrix<T> => blockcyclic_matrix<T>
  blockcyclic_matrix(colmajor_matrix<T>&& c_mat,size_t type=2,
                     size_t MB = 0, size_t NB = 0) {
#ifdef _DEBUG_STAT_
    std::cout << "(rvalue) colmajor_matrix -> blockcyclic_matrix (type): "
              << type << "\n";
#endif
    checkAssumption(type>=1 && type<=2);
    if(type == 1) { MB = NB = 0; } // ignoring user input
    size_t nrow = c_mat.num_row;
    size_t ncol = c_mat.num_col;
    data = std::move(c_mat.data.template map<blockcyclic_matrix_local<T>>
                    (scalapack_distributor_engine<T>(nrow,ncol,MB,NB),
                     broadcast(type),broadcast(true)));
    set_num(nrow,ncol,type);
  }
  // implicit conversion:
  // (lvalue) vector<T> => blockcyclic_matrix<T>
  blockcyclic_matrix(const std::vector<T>& vec,size_t type=1,
                     size_t MB = 0, size_t NB = 0) {
#ifdef _DEBUG_STAT_
    std::cout << "(lvalue) vector -> blockcyclic_matrix (type): "
              << type << "\n";
#endif
    if(vec.size() == 0)
      REPORT_ERROR(USER_ERROR,"Empty input vector!!\n");
    checkAssumption(type>=1 && type<=2);
    if(type == 1) { MB = NB = 0; } // ignoring user input
    size_t nrow = vec.size();
    size_t ncol = 1;
    rowmajor_matrix_local<T> r_mat_loc(vec);
    colmajor_matrix<T> c_mat(make_rowmajor_matrix_scatter<T>(r_mat_loc)); 
    data = std::move(c_mat.data.template map<blockcyclic_matrix_local<T>>
                    (scalapack_distributor_engine<T>(nrow,ncol,MB,NB),
                     broadcast(type),broadcast(true)));
    set_num(nrow,ncol,type); 
  }
  // implicit conversion:
  // (rvalue) vector<T> => blockcyclic_matrix<T>
  blockcyclic_matrix(std::vector<T>&& vec,size_t type=1,
                     size_t MB = 0, size_t NB = 0) {
#ifdef _DEBUG_STAT_
    std::cout << "(rvalue) vector -> blockcyclic_matrix (type): "
              << type << "\n";
#endif
    checkAssumption(type>=1 && type<=2);
    if(type == 1) { MB = NB = 0; } // ignoring user input
    if(vec.size() == 0)
      REPORT_ERROR(USER_ERROR,"Empty input vector!!\n");
    size_t nrow = vec.size();
    size_t ncol = 1;
    rowmajor_matrix_local<T> r_mat_loc(std::move(vec));
    colmajor_matrix<T> c_mat(make_rowmajor_matrix_scatter<T>(r_mat_loc)); 
    data = std::move(c_mat.data.template map<blockcyclic_matrix_local<T>>
                    (scalapack_distributor_engine<T>(nrow,ncol,MB,NB),
                     broadcast(type),broadcast(true)));
    set_num(nrow,ncol,type);
  }
  std::vector<T> to_vector() {
    if(num_col > 1) 
      REPORT_ERROR(USER_ERROR,
        "input is a matrix, can't be converted to a vector!!\n");
    auto rmat = to_colmajor().moveto_rowmajor().gather();
    return rmat.val;
  }
  std::vector<T> moveto_vector() {
    if(num_col > 1) 
      REPORT_ERROR(USER_ERROR,
        "input is a matrix, can't be converted to a vector!!\n");
    if(type != 1) REPORT_ERROR(USER_ERROR,"Invalid move operation requested!\n");
    auto rmat = moveto_colmajor().moveto_rowmajor().gather();
    return rmat.val;
  }
  colmajor_matrix<T> to_colmajor() {
    colmajor_matrix<T> outMat(data.template map<colmajor_matrix_local<T>>
                          (scalapack_distributor_engine<T>(num_row,num_col),
                           broadcast(false)));
    outMat.set_num(num_row, num_col);
    return outMat;
  }
  colmajor_matrix<T> moveto_colmajor() {
    if(type != 1) REPORT_ERROR(USER_ERROR,"Invalid move operation requested!\n");
    colmajor_matrix<T> outMat(data.template map<colmajor_matrix_local<T>>
                          (scalapack_distributor_engine<T>(num_row,num_col),
                           broadcast(true)));
    outMat.set_num(num_row, num_col);
    return outMat;
  }
  rowmajor_matrix<T> to_rowmajor() {
    if (num_col == 1)  return to_colmajor().moveto_rowmajor();
    else               return to_colmajor().to_rowmajor();
  }
  blockcyclic_matrix<T> transpose() {
    blockcyclic_matrix<T> ret(num_col, num_row);
    data.mapv(transpose_impl<T>,ret.data); // use pblas p_geadd()
    return ret;
  }
  void set_num(size_t nrow, size_t ncol, size_t t=2) {
    num_row = nrow; num_col = ncol; type = t;
  }
  void save(const std::string& file){
    to_rowmajor().save(file);
  }
  void savebinary(const std::string& dir) {
    to_rowmajor().savebinary(dir);
  }
  void debug_print() {data.mapv(call_debug_print<blockcyclic_matrix_local<T>>);}
  size_t get_nrows() { return num_row; }
  size_t get_ncols() { return num_col; }

  frovedis::node_local<blockcyclic_matrix_local<T>> data;
  size_t num_row, num_col, type;
};

// make a blockcyclic matrix by loading binary data, stored in input directory
template <class T>
blockcyclic_matrix<T>
make_blockcyclic_matrix_loadbinary(const std::string& dir, size_t type = 2,
                                   size_t MB = 0, size_t NB = 0) {
  colmajor_matrix<T> c_mat (make_rowmajor_matrix_loadbinary<T>(dir));
  return blockcyclic_matrix<T>(std::move(c_mat),type,MB,NB);
}

// make a blockcyclic matrix by loading textual data, stored in input file
template <class T>
blockcyclic_matrix<T>
make_blockcyclic_matrix_load(const std::string& file, size_t type = 2,
                             size_t MB = 0, size_t NB = 0) {
  colmajor_matrix<T> c_mat (make_rowmajor_matrix_load<T>(file));
  return blockcyclic_matrix<T>(std::move(c_mat),type,MB,NB);
}

// make a blockcyclic matrix by scattering a rowmajor_matrix_local<T>
template <class T>
blockcyclic_matrix<T>
make_blockcyclic_matrix_scatter(rowmajor_matrix_local<T>& r_mat, size_t type=2,
                                size_t MB = 0, size_t NB = 0) {
  colmajor_matrix<T> c_mat (make_rowmajor_matrix_scatter<T>(r_mat));
  return blockcyclic_matrix<T>(std::move(c_mat),type,MB,NB);
}

// make a blockcyclic matrix by scattering a (lvalue) std::vector<T>
// deprecated method: kept for backward compatibility
template <class T>
blockcyclic_matrix<T>
make_blockcyclic_matrix_scatter(const std::vector<T>& vec, size_t type=2,
                                size_t MB = 0, size_t NB = 0) {
  if(vec.size() == 0)
    REPORT_ERROR(USER_ERROR,"Empty input vector!!\n");
  rowmajor_matrix_local<T> tmp(vec);
  return make_blockcyclic_matrix_scatter <T> (tmp,type,MB,NB);
}

// make a blockcyclic matrix by scattering a (rvalue) std::vector<T>
// deprecated method: kept for backward compatibility
template <class T>
blockcyclic_matrix<T>
make_blockcyclic_matrix_scatter(std::vector<T>&& vec, size_t type=2,
                                size_t MB = 0, size_t NB = 0) {
  if(vec.size() == 0)
    REPORT_ERROR(USER_ERROR,"Empty input vector!!\n");
  rowmajor_matrix_local<T> tmp(std::move(vec));
  return make_blockcyclic_matrix_scatter<T>(tmp,type,MB,NB);
}

// make a blockcyclic matrix from a std::vector<T> (lValue)
template <class T>
blockcyclic_matrix<T>
inline vec_to_bcm(const std::vector<T>& vec, size_t type=1,
                  size_t MB = 0, size_t NB = 0) { 
  if(vec.size() == 0)
    REPORT_ERROR(USER_ERROR,"Empty input vector!!\n");
  return blockcyclic_matrix<T>(vec,type,MB,NB);
}

// make a blockcyclic matrix from a std::vector<T> (rValue)
template <class T>
blockcyclic_matrix<T>
inline vec_to_bcm(std::vector<T>&& vec, size_t type=1,
                  size_t MB = 0, size_t NB = 0) { 
  if(vec.size() == 0)
    REPORT_ERROR(USER_ERROR,"Empty input vector!!\n");
  return blockcyclic_matrix<T>(std::move(vec),type,MB,NB);
}

// T can be sliced_blockcyclic_matrix_local<T> or 
// sliced_blockcyclic_vector_local<T> or blockcyclic_matrix_local<T>
// to determine whether the current process
// is to be participated in PBLAS/ScaLAPACK routine computations
template <class T>
inline bool is_active(const T& d) {
  if(!(&d.descA[0])) 
    REPORT_ERROR(USER_ERROR,"Input slice has NULL parameter\n");
  return (d.descA[CTXT_A] != -1);    
}

// to check whether the input arguments of PBLAS/ScaLAPACK routines
// are of the same distribution (with same context)
// since inter-context computations can't be performed using PBLAS/ScaLAPACK
template <class T1, class T2>
inline bool is_same_distribution(const T1& d1, const T2& d2) {
  bool not_null = (&d1.descA[0]) && (&d2.descA[0]);
  if(!not_null) REPORT_ERROR(USER_ERROR,"Input slice has NULL parameter\n");
  return (d1.descA[CTXT_A] == d2.descA[CTXT_A]);
}

template <class T1, class T2, class T3>
inline bool is_same_distribution(const T1& d1, const T2& d2,
                                 const T3& d3) {
  bool not_null = (&d1.descA[0]) && (&d2.descA[0]) && (&d3.descA[0]);
  if(!not_null) REPORT_ERROR(USER_ERROR,"Input slice has NULL parameter\n");
  return (d1.descA[CTXT_A] == d2.descA[CTXT_A] && 
          d1.descA[CTXT_A] == d3.descA[CTXT_A]);
}

template <class T1, class T2, class T3, class T4>
inline bool is_same_distribution(const T1& d1, const T2& d2,
                                 const T3& d3, const T4& d4) {
  bool not_null = (&d1.descA[0]) && (&d2.descA[0]) && 
                  (&d3.descA[0]) && (&d4.descA[0]);
  if(!not_null) REPORT_ERROR(USER_ERROR,"Input slice has NULL parameter\n");
  return (d1.descA[CTXT_A] == d2.descA[CTXT_A] && 
          d1.descA[CTXT_A] == d3.descA[CTXT_A] &&
          d1.descA[CTXT_A] == d4.descA[CTXT_A]);
}

}

#endif
