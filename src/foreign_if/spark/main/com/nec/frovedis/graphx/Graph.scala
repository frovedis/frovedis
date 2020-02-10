package com.nec.frovedis.graphx;

import scala.util.control.Breaks._

import org.apache.spark.SparkContext
import org.apache.spark.rdd.RDD
import org.apache.spark.graphx.{Edge, VertexId, VertexRDD}
import org.apache.spark.mllib.recommendation.Rating
import com.nec.frovedis.Jexrpc.{FrovedisServer, JNISupport}
import com.nec.frovedis.Jgraph.DummyEdge;
import com.nec.frovedis.exrpc.FrovedisSparseData

class Graph extends java.io.Serializable {
  protected var fdata: Long = -1
  var numEdges: Long = -1
  var numVertices: Long = -1
  var vertices: VertexRDD[Double] = null

  def this(data: org.apache.spark.graphx.Graph[Int, Int]) = {
    this()
    load(data)
  }
  def load(data: org.apache.spark.graphx.Graph[Int, Int]) : Unit = {
    release() // releasing old graph data before loading new data
    this.numEdges = data.numEdges
    this.numVertices = data.numVertices
    // VertexRDD[Int] => VertexRDD[Double] 
    this.vertices = VertexRDD(data.vertices.map(x => (x._1, x._2.toDouble))) 
    val coo = data.edges.map(x => Rating(x.srcId.toInt, x.dstId.toInt, 
                                                                x.attr))
    val smat = new FrovedisSparseData()
    smat.loadcoo(coo) 
    val fs = FrovedisServer.getServerInstance()
    this.fdata = JNISupport.setGraphData(fs.master_node, smat.get()) 
    val info = JNISupport.checkServerException()
    if (info != "") throw new java.rmi.ServerException(info)
    smat.release() // releasing after graph creation 
                   //(graph contains transpose of smat))
  }
  def copy(): Graph = {
    val fs = FrovedisServer.getServerInstance()
    val gptr = JNISupport.copyGraph(fs.master_node, this.get())
    val info = JNISupport.checkServerException()
    if (info != "") throw new java.rmi.ServerException(info)
    val c_graph = new Graph()
    c_graph.fdata = gptr
    c_graph.numEdges = this.numEdges
    c_graph.numVertices = this.numVertices
    c_graph.vertices = this.vertices
    return c_graph
  }
  def to_spark_graph(): org.apache.spark.graphx.Graph[Double, Double] = {
    val fs = FrovedisServer.getServerInstance()
    // dummyArr: Array (dummyEdge => srcId, dstId, attr)
    val dummyArr = JNISupport.getGraphData(fs.master_node, this.get()) 
    val info = JNISupport.checkServerException();
    if (info != "") throw new java.rmi.ServerException(info)
    val context = vertices.context
    val npart = vertices.getNumPartitions
    val edgeArr = dummyArr.map(x => Edge(x.srcId, x.dstId, x.attr.toDouble))
    val edgeRDD = context.parallelize(edgeArr, npart)
    return org.apache.spark.graphx.Graph(vertices, edgeRDD)
  }
  def bfs(): com.nec.frovedis.graphx.bfs_result = {
    val fs = FrovedisServer.getServerInstance()

    //TODO: Need to discuss the cast from Long to Int
    val nodes_dist: Array[Int] = new Array(numVertices.toInt)
    //TODO: Need to discuss the cast from Long to Int
    val nodes_in_which_cc: Array[Long] = new Array(numVertices.toInt)
    val num_nodes_in_each_cc = JNISupport.callFrovedisBFS(fs.master_node,
                           this.get(), nodes_in_which_cc,
                           nodes_dist, numVertices)
    val info = JNISupport.checkServerException()
    if (info != "") throw new java.rmi.ServerException(info)
    val num_cc = num_nodes_in_each_cc.size
    return new bfs_result(num_cc, numVertices, nodes_dist, 
                          nodes_in_which_cc, num_nodes_in_each_cc)
  }
  def sssp(source_vertex: Long = 1): com.nec.frovedis.graphx.sssp_result = {
    require(source_vertex >= 0 && source_vertex < numVertices,
    s"Source Vertex should range from 0 to ${numVertices - 1}," + 
    " provided value is ${source_vertex}.")
    val fs = FrovedisServer.getServerInstance()

    //TODO: Need to discuss the cast from Long to Int
    val dist: Array[Int] = new Array(numVertices.toInt) 
    //TODO: Need to discuss the cast from Long to Int
    val pred: Array[Long] = new Array(numVertices.toInt) 
    JNISupport.callFrovedisSSSP(fs.master_node,
                   this.get(), dist, pred, numVertices, source_vertex)
    val info = JNISupport.checkServerException()
    if (info != "") throw new java.rmi.ServerException(info)
    return new sssp_result(pred, dist, numVertices, source_vertex)
  }
  def pageRank(tol: Double, 
             resetProb: Double = 0.15, 
             maxIter: Int = 100): com.nec.frovedis.graphx.Graph = {
    require(tol > 0,
        s"Tolerance should be greater than 0, but got ${tol}.")
    require(resetProb >= 0 && resetProb <= 1,
        s"Random reset probability should range from 0 to 1," + 
        "but got ${resetProb}.")
    require(maxIter > 0,
        s"Max iteration should be greater than 0, but got ${maxIter}.")
    val copy_gr = this.copy()
    val fs = FrovedisServer.getServerInstance()
    val result = JNISupport.callFrovedisPageRank(fs.master_node, 
                           copy_gr.get(), tol, resetProb, maxIter)
    val info = JNISupport.checkServerException()
    if (info != "") throw new java.rmi.ServerException(info)
    val nrank = result.size 
    val vertArr: Array[(VertexId, Double)] = new Array(nrank)
    for (i <- 0 to (nrank - 1)) vertArr(i) = ((i+1).toLong, result(i))
    val context = copy_gr.vertices.context
    val npart = copy_gr.vertices.getNumPartitions
    copy_gr.vertices = VertexRDD(context.parallelize(vertArr, npart))
    return copy_gr
  }
  def debug_print(): Unit = {
    if (fdata != -1) {
      println("Num Of Edges: " + numEdges)
      println("Num Of Vertices: " + numVertices)
      val fs = FrovedisServer.getServerInstance()
      JNISupport.showGraph(fs.master_node, this.get()) 
      val info = JNISupport.checkServerException()
      if (info != "") throw new java.rmi.ServerException(info)
    }
  }
  def release(): Unit = {
    if (fdata != -1) {
      val fs = FrovedisServer.getServerInstance()
      JNISupport.releaseGraph(fs.master_node, this.get()) 
      val info = JNISupport.checkServerException()
      if (info != "") throw new java.rmi.ServerException(info)
      this.fdata = -1
      this.numEdges = -1
      this.numVertices = -1
      this.vertices = null
    }
  }
  def save(fname: String): Unit = {
    if (fdata != -1) {
      val fs = FrovedisServer.getServerInstance()
      JNISupport.saveGraph(fs.master_node, this.get(), fname) 
      val info = JNISupport.checkServerException()
      if (info != "") throw new java.rmi.ServerException(info)
    }
  }
  def load_text(fname: String): Unit = {
    release()
    val fs = FrovedisServer.getServerInstance()
    val dummy_graph = JNISupport.loadGraphFromTextFile(fs.master_node, fname)
    val info = JNISupport.checkServerException()
    if (info != "") throw new java.rmi.ServerException(info)
    this.fdata = dummy_graph.dptr
    this.numEdges = dummy_graph.numEdges
    this.numVertices = dummy_graph.numVertices
    val vertArr: Array[(VertexId, Double)] = new Array(this.numVertices.toInt)
    for (i <- 0 to (this.numVertices.toInt - 1)) 
      vertArr(i) = ((i+1).toLong, 1.0)
    val context = SparkContext.getOrCreate()
    val npart = fs.worker_size
    this.vertices = VertexRDD(context.parallelize(vertArr, npart))
  }
  def get() = fdata
}
