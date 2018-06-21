package com.nec.frovedis.Jmatrix;

public class DummyGetrfResult implements java.io.Serializable {
  public DummyGetrfResult() {}
  public DummyGetrfResult(long ptr, int stat, short mt) {
    ipiv_ptr = ptr;
    info = stat;
    mtype = mt;
  }
  public long ipiv_ptr;
  public int info;
  public short mtype;
}
