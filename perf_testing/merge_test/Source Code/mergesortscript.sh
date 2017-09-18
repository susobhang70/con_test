#! /bin/sh

MAX_LINES_PER_CHUNK=9541047
ORIGINAL_FILE=$1
SORTED_FILE=$2
NUM_OF_LINES_IN_FILE=$(wc -l < $ORIGINAL_FILE)

usage ()
{
     echo Merge sort
     echo usage: thisscriptname file1 file2
     echo Sorts text file file1 and stores the output in file2
     echo Note: file1 will be split in chunks up to $MAX_LINES_PER_CHUNK lines
     echo  and each chunk will be sorted in parallel
}

# test if we have two arguments on the command line
if [ $# != 2 ]
then
    usage
    exit
fi

normalsort()
{
    sort -n -T /media/susobhan/EADA79CADA799413/Code/ $1 > $1.sorted &
    return
}

mergesort()
{
    if [ $2 -le $MAX_LINES_PER_CHUNK ]
    then
         normalsort $1
         return
    elif [ $2 -gt $MAX_LINES_PER_CHUNK ]
    then
        SPLIT_LINE=$(($2/2))
        split -l $SPLIT_LINE $1 $1

        for file in $1??*
        do
            LINE_BREAK=$(wc -l < $file)
            mergesort $file $LINE_BREAK
        done
        wait
    fi
}

mergesort $ORIGINAL_FILE $NUM_OF_LINES_IN_FILE
sort -n -m $1*.sorted > $SORTED_FILE

rm -f $1?* > /dev/null
