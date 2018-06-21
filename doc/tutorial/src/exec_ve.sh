#!/bin/sh

for dir in * ; do
	if [ -d $dir ]; then
		echo --- $dir ---
		cd $dir; mpirun -np 4 -x ./tut; cd ..
	fi
done
