package com.nec.frovedis.Jmllib;

public class DummyTSNEResult implements java.io.Serializable {
  public DummyTSNEResult() {}
  public DummyTSNEResult(long embedding_ptr,
                         int nrow_emb, int ncol_emb, 
                         int n_iter_, double kl_divergence_) {
    this.embedding_ptr = embedding_ptr;
    this.nrow_emb = nrow_emb;
    this.ncol_emb = ncol_emb;
    this.n_iter_ = n_iter_;
    this.kl_divergence_ = kl_divergence_;
  }
  public long embedding_ptr;
  public int nrow_emb, ncol_emb, n_iter_;
  public double kl_divergence_;
}
