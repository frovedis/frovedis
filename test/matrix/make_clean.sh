#!/bin/sh

for dir in * ; do
	if [ -d $dir ]; then
		cd $dir; make clean; cd ..
	fi
done
