#!/bin/bash


echo "\nShow result with sklearn and Frovedis \n" > test_result

for file_name in input/* ; do
	cp "$file_name"  test_data
	echo "$file_name" >> test_result
	python sklearn_demo.py >> test_result
        echo "\n"  >> test_result
	./spectral  >> test_result
        echo "----------------------------------------" >> test_result
done
