#!/bin/sh
# sample server script for SX
# place this file and frovedis_server.nqs.tmpl into the same directory, 
# and frovedis_server binary into ~/bin (specified in the nqs.tmpl file)
# then call this script as the server program

cd $(dirname $0)
rm -f ./frovedis_server.nqs
sed -e "s/ARGS/$*/g" < ./frovedis_server.nqs.tmpl > ./frovedis_server.nqs
qsub frovedis_server.nqs
