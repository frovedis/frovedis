name := "tut"

version := "1.0"

scalaVersion := "2.12.14"

unmanagedBase := file("/opt/nec/frovedis/x86/lib/spark/")

libraryDependencies += "org.apache.spark" %% "spark-core" % "3.1.2"
libraryDependencies += "org.apache.spark" %% "spark-mllib" % "3.1.2"
