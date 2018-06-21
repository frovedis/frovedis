#!/bin/sh

tot_count=0

for dir in * ; do
	if [ -d $dir ]; then
		cd $dir
		if [ ! -f ./test ]; then
			make 
		fi
		echo -n "\nRunning test $dir..."
                mpirun -np 4 ./test > .tmp
                fail_count=`grep error .tmp | wc -l`
		if [ ${fail_count} -eq 0 ]; then
			echo "[All Passed]"
                else
			echo "[${fail_count} Failed]"
		fi
		tot_count=`expr ${tot_count} + ${fail_count}`
                rm .tmp
		cd ..
	fi
done

if [ ${tot_count} -ne 0 ]; then
	echo "\nTotal ${tot_count} case(s) failed."
fi

