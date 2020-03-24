package com.nec.frovedis.Jmllib;

public class DummyLDAModel implements java.io.Serializable {
  public DummyLDAModel() {}
  public DummyLDAModel(int k, int vocab, int ndocs) {
    num_topics = k;
    vocabsz = vocab;
    num_docs = ndocs;
  }
  public int num_topics, vocabsz, num_docs;
}
