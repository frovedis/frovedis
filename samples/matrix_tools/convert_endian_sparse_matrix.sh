#!/bin/sh
# quick script to convert endian of matrix

set -- `getopt n:i:o:s: $*`
if [ $? != 0 ]; then
    echo "Usage: $0 [-n nproc] [-i input] [-o ouput] [-s size]" 1>&2
    exit 1
fi
while [ $# -gt 0 ]; do
    case $1 in
        -n) NPROC=$2
            shift 
            ;;
        -i) INPUT=$2
            shift 
            ;;
        -o) OUTPUT=$2
            shift 
            ;;
        -s) SIZE=$2
            shift 
            ;;
        --) shift
            break
            ;;
    esac
	shift
done

if [ -z "$NPROC" ]; then
    echo "Usage: $0 [-n nproc] [-i input] [-o ouput] [-s size]" 1>&2
    exit 1
fi
if [ -z "$INPUT" ]; then
    echo "Usage: $0 [-n nproc] [-i input] [-o ouput] [-s size]" 1>&2
    exit 1
fi
if [ -z "$OUTPUT" ]; then
    echo "Usage: $0 [-n nproc] [-i input] [-o ouput] [-s size]" 1>&2
    exit 1
fi
if [ -z "$SIZE" ]; then
    echo "Usage: $0 [-n nproc] [-i input] [-o ouput] [-s size]" 1>&2
    exit 1
fi

DIR=`dirname $0`

if [ ! -d $OUTPUT ]; then
	mkdir $OUTPUT
fi
cp $INPUT/nums $OUTPUT/
mpirun -np $NPROC $DIR/convert_endian -i $INPUT/idx -o $OUTPUT/idx -s 8
mpirun -np $NPROC $DIR/convert_endian -i $INPUT/off -o $OUTPUT/off -s 8
mpirun -np $NPROC $DIR/convert_endian -i $INPUT/val -o $OUTPUT/val -s $SIZE
