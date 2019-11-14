package com.nec.frovedis.Jgraph;

public class DummyGraph implements java.io.Serializable {
  public DummyGraph() {}
  public DummyGraph(long dptr, long numEdges, long numVertices) {
    this.dptr = dptr;
    this.numEdges = numEdges;
    this.numVertices = numVertices;
  }
  public long dptr, numEdges, numVertices;
}

