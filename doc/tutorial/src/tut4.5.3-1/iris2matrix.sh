#!/bin/sh
set -eu

input=$1
train_mat=$2
train_lab=$3
test_mat=$4
test_lab=$5

lines=$(sed '/^$/d' ${input})
nfeats=$(echo "${lines}" | head -n 1 | tr -dc ',' | wc -m)
ncols=$(expr ${nfeats} + 1)
klasses=$(echo "${lines}" | cut -d ',' -f ${ncols} | sort | uniq)
target=$(echo "${klasses}" | tail -n 1)

for outfile in ${train_mat} ${train_lab} ${test_mat} ${test_lab}; do
  echo -n > ${outfile}
done

kid=1
for klass in ${klasses}; do
  if [ ${klass} = ${target} ]; then
    kid=1
  else
    kid=-1
  fi

  klines=$(echo "${lines}" | grep "${klass}" | sed -e 's/,/ /g' -e "s/${klass}/${kid}/g")
  nrows=$(echo "${klines}" | wc -l)
  ntest=$(expr ${nrows} / 10)
  ntrain=$(expr ${nrows} - ${ntest})

  mat=$(echo "${klines}" | cut -d ' ' -f 1-${nfeats})
  lab=$(echo "${klines}" | cut -d ' ' -f ${ncols})

  echo "${mat}" | head -n ${ntrain} >> ${train_mat}
  echo "${lab}" | head -n ${ntrain} >> ${train_lab}
  echo "${mat}" | tail -n ${ntest} >> ${test_mat}
  echo "${lab}" | tail -n ${ntest} >> ${test_lab}
done
