#!/bin/bash
[ ! -d "./res2" ] && mkdir "./res2"

gcc -o ./main2 ./main2.c -lpthread
chmod +x ./main2
echo pass init
file=$1
d=0
    for thr in {1..32};do 
        [ ! -d "./res2/$thr" ] && mkdir "./res2/$thr"
        for batch in {0..9};do
            res_file=$(echo res2/$thr/bench-results-$file-$batch.csv)
            echo "" > $res_file
            for tests in {1..5};do
                res=$(./main -f $file -t $thr -m $batch -q)
                echo  "$res" >> $res_file   
            done
            d=$((d+1))
            echo -ne "done threads: $thr\t read size: $batch\t $d/320\t in $res_file\n"
        done
    done
