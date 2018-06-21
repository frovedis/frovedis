package com.nec.frovedis.Jexrpc;

import com.nec.frovedis.Jmatrix.DummyMatrix;
import com.nec.frovedis.Jmatrix.DummyGetrfResult;
import com.nec.frovedis.Jmatrix.DummyGesvdResult;
import com.nec.frovedis.Jmllib.DummyGLM;
import com.nec.frovedis.Jmllib.IntDoublePair;
//import org.apache.spark.mllib.linalg.SparseVector;

public class JNISupport {
 
  static {
    // Load native library libfrovedis_client.so at runtime
    System.loadLibrary("frovedis_client_spark");
  }
 
  // ---
  public static native MemPair loadFrovedisWorkerGLMData(Node t_node, 
                                                       long nrows, long ncols,
                                                       double lbl[],
                                                       int off[], 
                                                       int idx[], 
                                                       double val[]);
  public static native MemPair createFrovedisSparseGLMData(Node master_node,
                                                         MemPair eps[],
                                                         long nrows, long ncols);
  public static native void releaseFrovedisSparseGLMData(Node master_node, 
                                                       MemPair fdata);
  public static native void showFrovedisSparseGLMData(Node master_node, 
                                                    MemPair fdata);
  // ---
  public static native long loadFrovedisWorkerVectorStringData(Node t_node, 
                                                             String val[], 
                                                             int size);
  public static native long loadFrovedisWorkerData(Node t_node, 
                                                 long nrows, long ncols,
                                                 int off[], 
                                                 int idx[], 
                                                 double val[]);
  public static native DummyMatrix createFrovedisSparseMatrix(Node master_node,
                                                            long eps[],
                                                            short mtype);
  public static native long createFrovedisSparseData(Node master_node,
                                                   long eps[],
                                                   long nrows, long ncols);
  public static native void releaseFrovedisSparseData(Node master_node, 
                                                    long fdata); 
  public static native void showFrovedisSparseData(Node master_node, 
                                                 long fdata);
  // ---
  public static native long loadFrovedisWorkerRmajorData(Node t_node, 
                                                       long nrows, long ncols,
                                                       double val[]);
  public static native DummyMatrix getFrovedisRowmajorMatrix(Node master_node,
                                                           long fdata, 
                                                           short mtype);
  public static native long[] getAllLocalPointers(Node master_node, 
                                                  long dptr, short mtype);
  public static native double[] getLocalArray(Node t_node, 
                                              long l_dptr, short mtype);
  public static native long createFrovedisDenseData(Node master_node,
                                                  long eps[],
                                                  long nrows, long ncols,
                                                  short mtype);
  public static native void releaseFrovedisDenseData(Node master_node, 
                                                   long fdata, short mtype); 
  public static native void showFrovedisDenseData(Node master_node, 
                                                long fdata, short mtype);
  public static native void saveFrovedisDenseData(Node master_node, 
                                                long fdata, short mtype,
                                                String path, boolean isbinary);
  public static native DummyMatrix loadFrovedisDenseData(Node master_node, 
                                                       short mtype,
                                                       String path, 
                                                       boolean isbinary);
  // --- TODO ---
  public static native DummyMatrix transposeFrovedisDenseData(Node master_node,
                                                            long fdata, 
                                                            short mtype);
  public static native double[] getRowmajorArray(Node master_node, 
                                                 long fdata, short mtype);
  public static native double[] getColmajorArray(Node master_node, 
                                                 long fdata, short mtype);
  // ---
  public static native Node getMasterInfo(String cmd);
  public static native int getWorkerSize(Node master_node);
  public static native Node[] getWorkerInfo(Node master_node);
  public static native void finalizeFrovedisServer(Node master_node);
  public static native void cleanUPFrovedisServer(Node master_node);

  // -------- Logistic Regression --------
  public static native void callFrovedisLRSGD(Node master_node,
                                            MemPair fdata,
                                            int numIter, 
                                            double stepSize,
                                            double miniBatchFraction,
                                            double regParam, 
                                            int mid, boolean movable);

  public static native void callFrovedisLRLBFGS(Node master_node,
                                              MemPair fdata,
                                              int numIter, 
                                              double stepSize,
                                              int histSize,
                                              double regParam,
                                              int mid, boolean movable);

  // -------- Linear SVM Regression --------
  public static native void callFrovedisSVMSGD(Node master_node,
                                             MemPair fdata,
                                             int numIter,
                                             double stepSize,
                                             double miniBatchFraction,
                                             double regParam,
                                             int mid, boolean movable);

  public static native void callFrovedisSVMLBFGS(Node master_node,
                                               MemPair fdata,
                                               int numIter,
                                               double stepSize,
                                               int histSize,
                                               double regParam,
                                               int mid, boolean movable);

  // -------- Linear Regression --------
  public static native void callFrovedisLNRSGD(Node master_node,
                                             MemPair fdata,
                                             int numIter,
                                             double stepSize,
                                             double miniBatchFraction,
                                             int mid, boolean movable);

  public static native void callFrovedisLNRLBFGS(Node master_node,
                                               MemPair fdata,
                                               int numIter,
                                               double stepSize,
                                               int histSize,
                                               int mid, boolean movable);

  // -------- Lasso Regression --------
  public static native void callFrovedisLassoSGD(Node master_node,
                                               MemPair fdata,
                                               int numIter,
                                               double stepSize,
                                               double miniBatchFraction,
                                               double regParam,
                                               int mid, boolean movable);

  public static native void callFrovedisLassoLBFGS(Node master_node,
                                                 MemPair fdata,
                                                 int numIter,
                                                 double stepSize,
                                                 int histSize,
                                                 double regParam,
                                                 int mid, boolean movable);
  
  // -------- Ridge Regression --------
  public static native void callFrovedisRidgeSGD(Node master_node,
                                               MemPair fdata,
                                               int numIter,
                                               double stepSize,
                                               double miniBatchFraction,
                                               double regParam,
                                               int mid, boolean movable);

  public static native void callFrovedisRidgeLBFGS(Node master_node,
                                                 MemPair fdata,
                                                 int numIter,
                                                 double stepSize,
                                                 int histSize,
                                                 double regParam,
                                                 int mid, boolean movable);

  // -------- Matrix Factorization Using ALS --------
  public static native void callFrovedisMFUsingALS(Node master_node,
                                                 long fdata,
                                                 int rank,
                                                 int numIter,
                                                 double alpha,
                                                 double lambda,
                                                 long seed,
                                                 int mid, boolean movable);

  // -------- KMeans --------
  public static native void callFrovedisKMeans(Node master_node,
                                             long fdata,
                                             int k,
                                             int numIter,
                                             long seed,
                                             double epsilon,
                                             int mid, boolean movable);

  // -------- Compute SVD --------
  public static native DummyGesvdResult computeSVD(Node master_node,
                                                   long fdata,
                                                   int k,
                                                   boolean movable);
  public static native DummyGesvdResult getSVDResultFromFiles(Node master_node,
                                                              short mtype,
                                                              String s_file,
                                                              String u_file,
                                                              String v_file,
                                                              boolean wantU,
                                                              boolean wantV,
                                                              boolean isbinary);

  public static native void showFrovedisModel(Node master_node, int mid, short mkind);
  public static native void releaseFrovedisModel(Node master_node, int mid, short mkind);
  public static native long[] broadcast2AllWorkers(Node master_node, int mid, short mkind);

  // [GLM] for multiple test vectors (prediction done in parallel in Frovedis worker nodes)
  public static native double[] doParallelGLMPredict(Node t_node, long mptr, short mkind,
                                                     long nrows, long ncols,
                                                     int off[], int idx[],
                                                     double val[]);
  // [GLM] for single test vector (prediction done in master node)
  public static native double doSingleGLMPredict(Node master_node, int mid, short mkind,
                                                 long nrows, long ncols,
                                                 int off[], int idx[],
                                                 double val[]);
  // [KMM] for multiple test vectors (prediction done in parallel in Frovedis worker nodes)
  public static native int[] doParallelKMMPredict(Node t_node, long mptr, short mkind,
                                                  long nrows, long ncols,
                                                  int off[], int idx[],
                                                  double val[]);
  // [KMM] for single test vector (prediction done in master node)
  public static native int doSingleKMMPredict(Node master_node, int mid, short mkind,
                                              long nrows, long ncols,
                                              int off[], int idx[],
                                              double val[]);
  // [MFM] for multiple test vectors (prediction done in parallel in Frovedis worker nodes)
  public static native double[] doParallelALSPredict(Node t_node, long mptr, short mkind,
                                                     int uids[], int pids[]);
  // [MFM] for single test vector (prediction done in master node)
  public static native double doSingleALSPredict(Node master_node, int mid, short mkind,
                                                 int uid, int pid);

  // [MFM] for recommending 'num' no. of products (with rating) for a given user
  public static native IntDoublePair[] recommendProducts(Node master_node, 
                                                         int mid, short mkind,
                                                         int user, int num);
  // [MFM] for recommending 'num' no. of users (with rating) for a given product
  public static native IntDoublePair[] recommendUsers(Node master_node, 
                                                      int mid, short mkind,
                                                      int product, int num);

  public static native void setFrovedisGLMThreshold(Node master_node, 
                                                  int mid, short mkind, double thr);

  public static native DummyGLM loadFrovedisGLM(Node master_node, 
                                              int mid, short mkind, String path);
  public static native int loadFrovedisMFM(Node master_node, 
                                         int mid, short mkind, String path);
  public static native int loadFrovedisKMM(Node master_node, 
                                         int mid, short mkind, String path);
  public static native void saveFrovedisModel(Node master_node, int mid, 
                                            short mkind, String path);

  // [p]blas level 1 routines   
  public static native void swap(Node master_node, short mtype, 
                                 long vptr1, long vptr2);
  public static native void copy(Node master_node, short mtype,
                                 long vptr1, long vptr2);
  public static native void scal(Node master_node, short mtype,
                                 long vptr,  double alpha);
  public static native void axpy(Node master_node, short mtype,
                                 long vptr1, long vptr2, double alpha);
  public static native double dot(Node master_node, short mtype,
                                  long vptr1, long vptr2);
  public static native double nrm2(Node master_node, short mtype,
                                   long vptr);

  // [p]blas level 2 routines   
  public static native DummyMatrix gemv(Node master_node, short mtype,
                                        long mptr, long vptr,
                                        boolean isTrans, 
                                        double alpha, double beta);
  public static native DummyMatrix ger(Node master_node, short mtype,
                                       long vptr1, long vptr2,
                                       double alpha);

  // [p]blas level 3 routines   
  public static native DummyMatrix gemm(Node master_node, short mtype,
                                        long mptr1, long mptr2,
                                        boolean isTransM1, boolean isTransM2, 
                                        double alpha, double beta);
  public static native void geadd(Node master_node, short mtype,
                                  long mptr1, long mptr2,
                                  boolean isTrans, 
                                  double alpha, double beta);
  
  // [sca]lapack results
  public static native void saveAsFrovedisDiagMatrixLocal(Node master_node, 
                                                        long dptr, String path,
                                                        boolean isbinary);   //SVAL
  public static native double[] getDoubleArray(Node master_node, long dptr); //SVAL
  public static native void releaseDoubleArray(Node master_node, long dptr); //SVAL
  public static native void releaseIPIV(Node master_node, short mtype, long dptr);
 
  // [sca]lapack routines  
  public static native DummyGetrfResult getrf(Node master_node, short mtype,
                                              long mptr);
  public static native int getri(Node master_node, short mtype,
                                 long mptr, long ipiv_ptr);
  public static native int getrs(Node master_node, short mtype,
                                 long mptrA, long mptrB, 
                                 long ipiv_ptr, boolean isTrans);
  public static native int gesv(Node master_node, short mtype,
                                long mptrA, long mptrB);
  public static native int gels(Node master_node, short mtype,
                                long mptrA, long mptrB, 
                                boolean isTrans);
  public static native DummyGesvdResult gesvd(Node master_node, short mtype,
                                              long mptr, 
                                              boolean wantU, boolean wantV); 
}
