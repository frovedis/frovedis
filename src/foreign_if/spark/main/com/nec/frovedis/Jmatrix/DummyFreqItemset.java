package com.nec.frovedis.Jmatrix;

public class DummyFreqItemset implements java.io.Serializable {
   public DummyFreqItemset () {}
   public DummyFreqItemset (int[] it, long ct) {
     item = it;
     count = ct;
   }
   public void debug_print() {
     System.out.print("DummyFreqItemset: ");
     for (int x:  item) System.out.print(x + " ");
     System.out.print(count + "\n");
   }
   public int[] item;
   public long count;
}
