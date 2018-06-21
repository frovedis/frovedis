package com.nec.frovedis.Jmatrix;

public class DummyGesvdResult implements java.io.Serializable {
  public DummyGesvdResult() {}
  public DummyGesvdResult(long sptr, long uptr, long vptr,
                          int mm, int nn, int kk, 
                          int stat, short mt) {
    svec_ptr = sptr;
    umat_ptr = uptr; // -1, if wantU = false
    vmat_ptr = vptr; // -1, if wantV = false
    m = mm; n = nn; k = kk;
    info = stat; mtype = mt;
  }
  public long svec_ptr, umat_ptr, vmat_ptr;
  public int m, n, k, info;
  public short mtype;
}
