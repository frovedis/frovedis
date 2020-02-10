package com.nec.frovedis.graphx;

class bfs_result extends java.io.Serializable {
  var num_cc: Long = -1
  var num_nodes: Long = -1
  var nodes_dist: Array[Int] = Array()
  var nodes_in_which_cc: Array[Long] = Array()
  var num_nodes_in_each_cc: Array[Long] = Array()

  def this(num_cc: Long, num_nodes: Long, nodes_dist: Array[Int], 
          nodes_in_which_cc: Array[Long], num_nodes_in_each_cc: Array[Long]){
    this()
    this.num_cc = num_cc
    this.num_nodes = num_nodes
    this.nodes_dist = nodes_dist
    this.nodes_in_which_cc = nodes_in_which_cc
    this.num_nodes_in_each_cc = num_nodes_in_each_cc
  }
  def print_summary(): Unit = {
    println("*************SUMMARY***************")
    println("Number of Connected Components = " + this.num_cc)

    var num_cc_printed: Long = 20
    if (this.num_cc < num_cc_printed) num_cc_printed = this.num_cc

    println(
        "Number of nodes in each connected component: (printing the first " 
        + num_cc_printed + ")")
    for(i <- 0 until num_cc_printed.toInt){
        print(i + ":" + this.num_nodes_in_each_cc(i) + "\t")
    }

    println("\nNodes in which cc: ")
    for(i <- 0 until this.num_nodes.toInt){
        print(i + ":" + this.nodes_in_which_cc(i) + "\t")
    }
    println("\nNodes dist: ")
    for(i <- 0 until this.num_nodes.toInt){
        print(i + ":" + this.nodes_dist(i) + "\t")
    }
    println()
  }
}
