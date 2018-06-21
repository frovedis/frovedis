#!/bin/sh

for dir in * ; do
	if [ -d $dir ]; then
		echo --- $dir ---
		cd $dir; mpirun -np 4 ./tut; cd ..
	fi
done
