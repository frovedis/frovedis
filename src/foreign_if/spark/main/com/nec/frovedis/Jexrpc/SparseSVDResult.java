package com.nec.frovedis.Jexrpc;

public class SparseSVDResult {
/*
  U:  m x k (<-m)
  S:  k(<-m) x k(<-n)
  Vt: k(<-n) x n / V: n x k
*/
  public double U[];
  public double V[];
  public double S[];
  public int m, n, k;

  public SparseSVDResult(int mm, int nn, int kk,
                         double[] UU, double[] VV, double[] SS) {
    m = mm; n = nn; k = kk;
    U = UU; V = VV; S = SS;
  }
  public void display() {
    System.out.println("m: " + m + ", n: " + n + ", k: " + k);
    System.out.println("U:");
    for(int i=0; i<m*k; ++i) System.out.print(U[i] + " ");
    System.out.println();
    System.out.println("V:");
    for(int i=0; i<n*k; ++i) System.out.print(V[i] + " ");
    System.out.println();
    System.out.println("S:");
    for(int i=0; i<k; ++i) System.out.print(S[i] + " ");
    System.out.println();
  }
}
