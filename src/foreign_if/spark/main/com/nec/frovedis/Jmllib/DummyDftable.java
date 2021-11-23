package com.nec.frovedis.Jmllib; // TODO: move to JdataFrame package

public class DummyDftable implements java.io.Serializable {
  public DummyDftable() {}
  public DummyDftable(long dfptr, String[] names, short[] types) {
    this.dfptr = dfptr;
    this.names = names;
    this.types = types;
  }
  public long dfptr;
  public String[] names;
  public short[] types;
}

