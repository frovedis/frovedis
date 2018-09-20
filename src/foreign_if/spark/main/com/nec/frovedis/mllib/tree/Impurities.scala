package com.nec.frovedis.mllib.tree

import org.apache.spark.mllib.tree.impurity._

object Impurities {
  def fromString(name: String): Impurity = name match {
    case "gini" => Gini
    case "entropy" => Entropy
    case "variance" => Variance
    case _ => throw new IllegalArgumentException(s"Did not recognize Impurity name: $name")
  }
  def toString(tt: Impurity): String = tt match {
    case Gini => "gini"
    case Entropy => "entropy"
    case Variance => "variance"
    case _ => throw new IllegalArgumentException(s"Did not recognize Impurity name: $tt")
  }
}
