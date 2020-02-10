package com.nec.frovedis.Jmllib;

public class DummyLDAModel implements java.io.Serializable {
  public DummyLDAModel() {}
  public DummyLDAModel(int k, int vocab) {
    num_topics = k;
    vocabsz = vocab;
  }
  public int num_topics, vocabsz;
}
