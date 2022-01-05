name := "Spark TPC-H Queries"

version := "1.0"

scalaVersion := "2.12.14"

unmanagedJars in Compile += {
  Attributed.blank(file("../../lib/frovedis_client.jar"))
}

libraryDependencies += "org.apache.spark" %% "spark-core" % "3.1.2"
libraryDependencies += "org.apache.spark" %% "spark-sql" % "3.1.2"
