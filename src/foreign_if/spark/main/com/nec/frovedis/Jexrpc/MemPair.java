package com.nec.frovedis.Jexrpc;

public class MemPair implements java.io.Serializable {
  public MemPair() {}
  public MemPair(long x, long y) {
    mx = x; my = y;
  }
  public long first()  { return mx; }
  public long second() { return my; }

  private long mx, my;
}
