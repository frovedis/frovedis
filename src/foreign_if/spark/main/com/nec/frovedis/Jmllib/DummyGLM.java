package com.nec.frovedis.Jmllib;

public class DummyGLM implements java.io.Serializable {
  public DummyGLM() {}
  public DummyGLM(int id, short kind, long nftr, int ncls,
                  double thr) {
    this.mid = id;
    this.mkind = kind;
    this.numFeatures = nftr;
    this.numClasses = ncls;
    this.threshold = thr;
  }
  @Override
  public String toString() {
    String str = "mid: " + mid +
                 ", mkind: " + mkind +
                 ", numFeatures: " + numFeatures +
                 ", numClasses: " + numClasses +
                 ", threshold: " + threshold;
    return str;
  }
  public void display() {
    System.out.println(toString());
  }

  public int mid;
  public short mkind;
  public long numFeatures;
  public int  numClasses;
  public double threshold;
}
