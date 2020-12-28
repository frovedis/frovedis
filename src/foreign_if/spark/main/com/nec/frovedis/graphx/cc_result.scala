package com.nec.frovedis.graphx;

import scala.util.control.Breaks._

class cc_result extends java.io.Serializable {
  var num_cc: Long = -1
  var num_nodes: Long = -1
  var nodes_dist: Array[Long] = Array()
  var nodes_in_which_cc: Array[Long] = Array()
  var root_with_cc_count: Array[Long] = Array()

  def this(num_cc: Long, num_nodes: Long, 
           nodes_dist: Array[Long], 
           nodes_in_which_cc: Array[Long], 
           root_with_cc_count: Array[Long]) {
    this()
    this.num_cc = num_cc
    this.num_nodes = num_nodes
    this.nodes_dist = nodes_dist
    this.nodes_in_which_cc = nodes_in_which_cc    // node, which_cc: 1-based
    this.root_with_cc_count = root_with_cc_count  // root: 1-based
  }
  def print_summary(print_limit: Int = 5): Unit = {
    println("-------- CC SUMMARY --------")
    println("Number of connected components = " + num_cc)

    var num_cc_printed: Int = scala.math.min(print_limit, num_cc.toInt)
    println("Root with its count of nodes in each connected component " 
            + "(root_id:count)")
    for(i <- 0 until (2 * num_cc_printed) by 2) {
      print(root_with_cc_count(i) + ":" + root_with_cc_count(i + 1) + "\t")
    }
    if(num_cc > print_limit) println("...")

    var count: Long = 1
    println("\nNodes in which cc: (node_id:root_id)")
    breakable {
      for(i <- 0 until num_nodes.toInt) {
        if (nodes_in_which_cc(i) != Long.MaxValue) { // skipping null vertices
          print((i+1) + ":" + nodes_in_which_cc(i) + "\t")
          count += 1
        }
        if (count > print_limit) {
          println("...") 
          break
        }
      }
    }

    count = 1
    println("\nNodes dist: (node:level_from_root)")
    breakable { 
      for(i <- 0 until num_nodes.toInt) {
        if (nodes_dist(i) != Long.MaxValue) { // skipping null vertices
          print((i+1) + ":" + nodes_dist(i) + "\t")
          count += 1
        }
        if (count > print_limit) {
          println("...") 
          break
        }
      }
    }

    println()
  }
}
