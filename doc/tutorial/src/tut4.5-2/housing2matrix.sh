#!/bin/sh
set -eu

input=$1
train_mat=$2
train_lab=$3
test_mat=$4
test_lab=$5

lines=$(sed -e 's/^ *\| *$//g' -e 's/ \+/ /g' ${input})
nrows=$(echo "${lines}" | wc -l)
nfeats=$(echo "${lines}" | head -n 1 | tr -dc ' ' | wc -m)
ncols=$(expr ${nfeats} + 1)
ntest=$(expr ${nrows} \* 4 / 100)
ntrain=$(expr ${nrows} - ${ntest})

mat=$(echo "${lines}" | cut -d ' ' -f 1-${nfeats})
lab=$(echo "${lines}" | cut -d ' ' -f ${ncols})

echo "${mat}" | head -n ${ntest} > ${test_mat}
echo "${lab}" | head -n ${ntest} > ${test_lab}
echo "${mat}" | tail -n ${ntrain} > ${train_mat}
echo "${lab}" | tail -n ${ntrain} > ${train_lab}
