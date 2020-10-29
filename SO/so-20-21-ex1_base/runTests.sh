#!/bin/bash

inputdir=${1}
outputdir=${2}
maxthreads=${3}

for input in $inputdir/*.txt
do 
i=${input##*/}
    for t in $(seq 1 $maxthreads)
    do
        echo InputFile=$i NumThreads=$t
        ./tecnicofs $input $outputdir/$i-$t $t |grep "TecnicoFS completed in"
    done
done