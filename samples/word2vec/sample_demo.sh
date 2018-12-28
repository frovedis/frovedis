#!/bin/sh

#
# Note
#
# - Input data should be text file which is compatible to ASCII-format. Text contains bag of documents which are separated by new line, and words in document are separated by space.
# - Output (vectors_$input.txt in this script) is same to that of original word2vec implementation from google (https://code.google.com/archive/p/word2vec/). 
#


# data
input=hoge  # CHANGE according to name of your data
input_path=/path/for/data/dir/$input


# traninig parameter
threads=8
min_count=2
size=256
window=5
negative=5
sample=1e-4
iter=5
alpha=0.05

# CHANGE below according to path for frovedis samples
ve_frovedis_path=/opt/nec/nosupport/frovedis/ve
x86_frovedis_path=/opt/nec/nosupport/frovedis/x86

build_vocab_path=$x86_frovedis_path/samples/word2vec/build_vocab
ve_train_word_path=$ve_frovedis_path/samples/word2vec/ve_train_word
make_word_vector_path=$x86_frovedis_path/samples/word2vec/make_word_vector


echo "build vocabulary ..."
$build_vocab_path \
    -i $input_path \
    -e encoded_$input.bin \
    -v vocab_$input.bin \
    -c vocab_count_$input.bin \
    --min-count $min_count


ve_range=0
num_proc=1
exec="mpirun -ve $ve_range -np $num_proc"
echo "train word ..."
$exec $ve_train_word_path \
    -i encoded_${input}.bin \
    -v vocab_${input}.bin \
    -c vocab_count_${input}.bin \
    -o weight_$input.bin \
    --threads $threads \
    --size $size --window $window --negative $negative --sample $sample --iter $iter --alpha $alpha


# if you want to save word vector as binary, pass "--binary" option below.
# min_count should be same to than passed to build_vocab
echo "make word vector ..."
$make_word_vector_path \
    -w weight_$input.bin \
    -v vocab_$input.bin \
    -o vectors_$input.txt \
    --min-count $min_count
