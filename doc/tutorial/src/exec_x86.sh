#!/bin/sh

for dir in * ; do
	if [ -d $dir ]; then
# exclude dnn tutorial, which needs input file
		if [ $dir != "tut4.7-1" -a $dir != "tut4.7-2" ]; then
			echo --- $dir ---
			cd $dir; mpirun -np 4 ./tut; cd ..
		fi
	fi
done
