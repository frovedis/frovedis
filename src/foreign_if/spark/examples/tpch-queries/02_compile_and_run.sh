sbt clean

sbt package

COMMAND="mpirun -np 8 ../../../server/frovedis_server"
LIBRARY_PATH="../../lib"
JARS="../../lib/frovedis_client.jar"
export LD_LIBRARY_PATH="../../lib"
#COMMAND="mpirun -np 8 $FROVEDIS_SERVER" 
#LIBRARY_PATH=$X86_INSTALLPATH/lib
#JARS=$X86_INSTALLPATH/lib/spark/frovedis_client.jar
#export LD_LIBRARY_PATH="/opt/nec/frovedis/x86/lib"

QID=0
mkdir -p output
spark-submit --class "main.scala.TpchQuery" --master local[12] --jars ${JARS} --conf spark.driver.memory=100g target/scala-2.12/spark-tpc-h-queries_2.12-1.0.jar "${COMMAND}" ${QID}
