package com.nec.frovedis.Jmllib;

public class DummyLDAResult implements java.io.Serializable {
  public DummyLDAResult() {}
  public DummyLDAResult(long dptr, long nr, long nc, double ppl, 
                        double llh) {
    mptr = dptr;
    nrow = nr;
    ncol = nc;
    perplexity = ppl;
    likelihood = llh;
  }
  public double perplexity, likelihood;
  public long mptr, nrow, ncol;
}
