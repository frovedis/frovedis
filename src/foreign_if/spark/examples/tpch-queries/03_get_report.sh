if [ $# -eq 0 ]; then
  echo "please provide the execution log file as first argument!"
  exit -1
fi

log=$1

grep "data transfer" ${log} | cut -d ' ' -f 7 >& .dt
grep "query processing" ${log} | cut -d ' ' -f 7 >& .qp
grep "to_spark_DF" ${log} | cut -d ' ' -f 6 >& .rc

echo "Data Transfer (sec),Query Processing (sec),to_spark_DF (sec)" > eval.csv
paste -d ',' .dt .qp .rc >> eval.csv
rm -rf .dt .qp .rc
exit 0
