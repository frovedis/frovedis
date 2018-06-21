#!/bin/sh

for dir in * ; do
	if [ -d $dir ]; then
		echo --- $dir ---
		cd $dir; qsub tut.nqs; cd ../
	fi
done

