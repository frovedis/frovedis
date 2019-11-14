package com.nec.frovedis.Jgraph;

public class DummyEdge implements java.io.Serializable {
  public DummyEdge() {}
  public DummyEdge(long srcId, long dstId, double attr) {
    this.srcId = srcId;
    this.dstId = dstId;
    this.attr = attr;
  }
  public long srcId, dstId;
  public double attr; 
}

