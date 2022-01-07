package main.scala

import org.apache.spark.SparkContext
import org.apache.spark.SparkConf
import java.io.BufferedWriter
import java.io.File
import java.io.FileWriter
import org.apache.spark.sql._
import scala.collection.mutable.ListBuffer
import com.nec.frovedis.Jexrpc.FrovedisServer
import com.nec.frovedis.matrix.TimeSpent
import org.apache.log4j.Level

/**
 * Parent class for TPC-H queries.
 *
 * Defines schemas for tables and reads pipe ("|") separated text files into these tables.
 *
 * Savvas Savvides <savvas@purdue.edu>
 *
 */
abstract class TpchQuery {

  // get the name of the class excluding dollar signs and package
  private def escapeClassName(className: String): String = {
    className.split("\\.").last.replaceAll("\\$", "")
  }

  def getName(): String = escapeClassName(this.getClass.getName)

  /**
   *  implemented in children classes and hold the actual query
   */
  def execute(sc: SparkContext, tpchSchemaProvider: TpchSchemaProvider): DataFrame
}

object TpchQuery {

  def outputDF(df: DataFrame, outputDir: String, className: String): Unit = {

    if (outputDir == null || outputDir == "")
      df.collect().foreach(println)
    else
      //df.write.mode("overwrite").json(outputDir + "/" + className + ".out") // json to avoid alias
      df.write.mode("overwrite").format("com.databricks.spark.csv").option("header", "true").save(outputDir + "/" + className)
  }

  def executeQueries(sc: SparkContext, schemaProvider: TpchSchemaProvider, queryNum: Int): ListBuffer[(String, Float)] = {

    // if set write results to hdfs, if null write to stdout
    // val OUTPUT_DIR: String = "/tpch"
    val OUTPUT_DIR: String = "file://" + new File(".").getAbsolutePath() + "/output"

    val results = new ListBuffer[(String, Float)]

    var fromNum = 1;
    var toNum = 22;
    if (queryNum != 0) {
      fromNum = queryNum;
      toNum = queryNum;
    }

    for (queryNo <- fromNum to toNum) {
      val t0 = System.nanoTime()

      val query = Class.forName(f"main.scala.Q${queryNo}%02d").newInstance.asInstanceOf[TpchQuery]

      try {
        val tlog = new TimeSpent(Level.INFO)
        val ret_df = query.execute(sc, schemaProvider)
        tlog.show(query.getName() + ": exec time: ")
        outputDF(ret_df, OUTPUT_DIR, query.getName())
      } catch {
        case e: java.rmi.ServerException => println("[Exception in processsing: " + query.getName() + " => " + e.getMessage())
      }

      val t1 = System.nanoTime()
      val elapsed = (t1 - t0) / 1000000000.0f // second
      results += new Tuple2(query.getName(), elapsed)
    }

    return results
  }

  def main(args: Array[String]): Unit = {

    var command: String = null
    if (args.length == 0)
      throw new IllegalArgumentException("server instantiation command is not provided!")
    else
      command = args(0)

    var queryNum = 0;
    if (args.length > 1)
      queryNum = args(1).toInt

    val conf = new SparkConf().setAppName("Simple Application")
    val sc = new SparkContext(conf)
    FrovedisServer.initialize(command)
    
    // read files from local FS
    val INPUT_DIR = "file://" + new File(".").getAbsolutePath() + "/input/1GB"

    val schemaProvider = new TpchSchemaProvider(sc, INPUT_DIR)

    val output = new ListBuffer[(String, Float)]
    output ++= executeQueries(sc, schemaProvider, queryNum)

    val outFile = new File("TIMES.txt")
    val bw = new BufferedWriter(new FileWriter(outFile, true))

    output.foreach {
      case (key, value) => bw.write(f"${key}%s\t${value}%1.8f\n")
    }

    bw.close()
    FrovedisServer.shut_down()
  }
}
