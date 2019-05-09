#!/bin/sh

for dir in * ; do
	if [ -d $dir ]; then
# exclude dnn tutorial, which needs input file
# exclude w2v
		if [ $dir != "tut4.7-1" -a $dir != "tut4.7-2" -a $dir != "tut4.6" ]; then
			echo --- $dir ---
			cd $dir; qsub tut.nqs; cd ../
		fi
	fi
done

