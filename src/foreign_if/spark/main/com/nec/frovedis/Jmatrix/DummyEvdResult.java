package com.nec.frovedis.Jmatrix;

public class DummyEvdResult implements java.io.Serializable {
  public DummyEvdResult() {}
  public DummyEvdResult(long sptr, long vptr,
                        int mm, int nn, int kk) {
    svec_ptr = sptr;
    vmat_ptr = vptr;
    m = mm; n = nn; k = kk;
  }
  public long svec_ptr, vmat_ptr;
  public int m, n, k;
}
