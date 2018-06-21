package com.nec.frovedis.Jexrpc;

public class Node implements java.io.Serializable {
  public Node() {}
  public Node(String hname, int pid) {
    hostname = hname; rpcport = pid;
  }
  public void display() {
    System.out.println("Hostname: " + hostname + " Port: " + rpcport + "\n");
  }
  public String get_host() { return hostname; }
  public int get_port() { return rpcport; }
  private String hostname;
  private int rpcport;
}

