name := "tut"

version := "1.0"

scalaVersion := "2.11.12"

unmanagedBase := file("/opt/nec/nosupport/frovedis/x86/lib/spark/")

libraryDependencies += "org.apache.spark" %% "spark-core" % "2.2.1"
libraryDependencies += "org.apache.spark" %% "spark-mllib" % "2.2.1"
