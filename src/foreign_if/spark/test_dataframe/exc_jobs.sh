#!/bin/bash

all=`ls scala/test*`

for i in ${all}; do
  cp ${i} scala/FrovedisDataframeDemo.scala
  make -B
  name=`echo ${i} | cut -d '/' -f 2 | cut -d '.' -f 1`
  out="out/${name}.frovout"
  err="err/${name}.froverr"
  ./run.sh >${out} 2>${err}
done

rm -f scala/FrovedisDataframeDemo.scala
