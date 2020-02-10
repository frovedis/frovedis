package com.nec.frovedis.Jmllib;

public class DummyKNNResult implements java.io.Serializable {
  public DummyKNNResult() {}
  public DummyKNNResult(long indices_ptr, long distances_ptr, 
                        int nrow_ind, int ncol_ind, 
                        int nrow_dist, int ncol_dist, int k) {
    this.indices_ptr = indices_ptr;
    this.distances_ptr = distances_ptr;
    this.nrow_ind = nrow_ind;
    this.ncol_ind = ncol_ind;
    this.nrow_dist = nrow_dist;
    this.ncol_dist = ncol_dist;
    this.k = k;
  }
  public long indices_ptr, distances_ptr;
  public int nrow_ind, ncol_ind, nrow_dist, ncol_dist ;
  public int k;
}
