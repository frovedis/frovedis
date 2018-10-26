package com.nec.frovedis.Jmatrix;

public class DummyPCAResult implements java.io.Serializable {
  public DummyPCAResult() {}
  public DummyPCAResult(long pc_ptr, int nr, int nc, 
                        short mt, long var_ptr, int k_) {
    mptr = pc_ptr;
    nrows = nr;
    ncols = nc;
    vptr = var_ptr;
    mtype = mt;
    k = k_;
  }
  public long mptr, vptr;
  public int nrows, ncols;
  public short mtype;
  public int k;
}
