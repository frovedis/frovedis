if [ $# -eq 0 ]; then
  SCALE=1
else
  SCALE=$1
fi

#download src for data generator
git clone https://github.com/electrum/tpch-dbgen.git

#compile src
cd tpch-dbgen
make

#generate tables
./dbgen -s ${SCALE}
chmod +r *.tbl
mkdir -p ../input/${SCALE}GB
mv *.tbl ../input/${SCALE}GB
