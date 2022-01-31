package com.nec.frovedis.matrix;

import scala.reflect.ClassTag
import org.apache.log4j.{Level, Logger}

object GenericUtils extends java.io.Serializable {
  def ceil_div(x: Long, y: Long): Long = {
    if (x == 0) return 0
    else return (x - 1) / y + 1
  }

  def get_block_sizes(npart: Long, nproc: Int): Array[Long] = {
    val ret = new Array[Long](nproc)
    val each = ceil_div(npart, nproc)
    var size = npart
    for (i <- 0 until nproc) {
      if (size > each) {
        ret(i) = each
        size -= each
      } else {
        ret(i) = size
        size = 0
      }
    }
    return ret
  }

  def get_start_indices(sizes: Array[Long]): Array[Long] = {
    val sz = sizes.size
    val ret = new Array[Long](sz)
    ret(0) = 0
    for (i <- 1 until sz) ret(i) = ret(i - 1) + sizes(i - 1)
    return ret
  }

  def index2Dest(index: Long, sizes: Array[Long]): (Int, Long) = {
    val sz = sizes.size
    val starts = get_start_indices(sizes)
    val end_idx = starts(sz - 1) + sizes(sz - 1) - 1
    require(index >= 0 && index <= end_idx, "given index is out of range!")
    for (i <- 0 until (sz - 1)) { 
      if (index >= starts(i) && index < starts(i + 1)) return (i, starts(i))
    }
    return (sz - 1, starts(sz - 1))
  }

  def flatten[T: ClassTag](data: Array[Array[T]]): (Array[T], Array[Int]) = {
    var k = 0
    var tot_size = 0
    val nelem = data.size
    val sizes = new Array[Int](nelem)
    for (i <- 0 until nelem) {
      val sz = data(i).size
      sizes(i) = sz
      tot_size += sz
    }
 
    val ret = new Array[T](tot_size)
    for (i <- 0 until nelem) {
      val tmp = data(i)
      for(j <- 0 until tmp.size) ret(k + j) = tmp(j)
      k += tmp.size
    }
    return (ret, sizes)
  }

  def flatten_charArray_asIntArray(data: Array[Array[Char]]): (Array[Int], Array[Int]) = {
    var k = 0
    var tot_size = 0
    val nelem = data.size
    val sizes = new Array[Int](nelem)
    for (i <- 0 until nelem) {
      val sz = data(i).size
      sizes(i) = sz
      tot_size += sz
    }

    val ret = new Array[Int](tot_size)
    for (i <- 0 until nelem) {
      val tmp = data(i)
      for(j <- 0 until tmp.size) ret(k + j) = tmp(j).toInt
      k += tmp.size
    }
    return (ret, sizes)
  }
}

// TODO: use spark logger
object FrovedisLogger extends java.io.Serializable {
  Logger.getLogger("Frovedis").setLevel(Level.INFO)

  def getLevel = Logger.getLogger("Frovedis").getLevel()
  def setLevel(level: Level): Level = {
    Logger.getLogger("FrovedisLogger").setLevel(level)
    return level
  }
  def trace(msg: String) = Logger.getLogger("FrovedisLogger").trace(msg)
  def debug(msg: String) = Logger.getLogger("FrovedisLogger").debug(msg)
  def info(msg: String)  = Logger.getLogger("FrovedisLogger").info(msg)
  def warn(msg: String) = Logger.getLogger("FrovedisLogger").warn(msg)
}

class TimeSpent(level: Level) extends java.io.Serializable {
  private val tlog_level = level
  private var t0 = System.nanoTime()
  private var lap_sum = 0L

  def show_impl(msg: String, diff_t: Long): Unit = {
    val elapsed = diff_t / 1000000000.0f 
    val show_msg = msg + elapsed + " sec"
    tlog_level match {
      case Level.TRACE =>  FrovedisLogger.trace(show_msg)
      case Level.DEBUG =>  FrovedisLogger.debug(show_msg)
      case Level.INFO  =>  FrovedisLogger.info(show_msg)
      case Level.WARN  =>  FrovedisLogger.warn(show_msg)
      case _           =>  throw new IllegalArgumentException(
                           "Unsupported LogLevel: " + tlog_level)
    }
  }
  def lap_start(): Unit = { t0 = System.nanoTime() }
  def lap_stop(): Unit = { lap_sum += System.nanoTime() - t0}
  def show_lap(msg: String): Unit = show_impl(msg, lap_sum)
  def show(msg: String): Unit = {
    show_impl(msg, System.nanoTime() - t0)
    t0 = System.nanoTime()
  }
}

