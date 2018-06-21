package com.nec.frovedis.Jmllib;

public class IntDoublePair implements java.io.Serializable {
  public IntDoublePair() {}
  public IntDoublePair(int x, double y) {
    key = x; value = y;
  }
  public int getKey()  { return key; }
  public double getValue() { return value; }

  private int key;
  private double value;
}
