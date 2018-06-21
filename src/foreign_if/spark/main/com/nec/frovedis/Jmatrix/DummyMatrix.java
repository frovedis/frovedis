package com.nec.frovedis.Jmatrix;

public class DummyMatrix implements java.io.Serializable {
  public DummyMatrix() {}
  public DummyMatrix(long ptr, long nr, long nc, short mt) {
    this.mptr = ptr;
    this.nrow = nr;
    this.ncol = nc;
    this.mtype = mt;
  }
  public long mptr, nrow, ncol;
  public short mtype;
}
