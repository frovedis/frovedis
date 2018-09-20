include ../../../../Makefile.in.x86
#include ./Makefile.in.x86

CLIENT_JAR := ../lib/frovedis_client.jar
#CLIENT_JAR := ${INSTALLPATH}/lib/spark/frovedis_client.jar

all: 
	${SCALA_HOME}/bin/scalac -cp "${SPARK_HOME}/jars/*:${HDPATH}:${CLIENT_JAR}" -sourcepath . -d bin scala/Gtest.scala
	cd MANIFEST; make
	cd bin; ${JAVA_HOME}/bin/jar -cfm ../lib/spark2frovedis_gtest.jar ../MANIFEST/gtest.mf test/scala/GenericTest*

clean:
	rm -fr *~ bin/test scala/*~ MANIFEST/*.mf MANIFEST/append MANIFEST/*~ out/* err/* lib/*
