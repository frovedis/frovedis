#!/bin/sh

if [ ! -f Makefile.each ]
then
	echo "Please 'ln -s Makefile.each.[x86, ve, or sx] Makefile.each' according to your architecture"
	exit 1
fi

if [ ! -f Makefile.each.omp ]
then
	echo "Please 'ln -s Makefile.each.omp.[x86, ve, or sx] Makefile.each.omp' according to your architecture"
	exit 1
fi

for dir in * ; do
	if [ -d $dir ]; then
		cd $dir; make; cd ..
	fi
done
