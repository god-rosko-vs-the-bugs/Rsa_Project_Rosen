#!/bin/bash
[ ! -d "./res3" ] && mkdir "./res3"

gcc -o ./main ./main.c -lpthread
chmod +x ./main
echo pass init
file=$1
d=0
    for thr in {1..32};do 
        [ ! -d "./res3/$thr" ] && mkdir "./res3/$thr"
        for batch in {0..9};do
            res_file=$(echo res3/$thr/bench-results-$file-$batch.csv)
            echo "" > $res_file
            #for tests in {1..5};do

                res=$(./main -f $file -t $thr -m $batch -q)
                echo  "$res" >> $res_file   
            #done
            d=$((d+1))
            echo -ne "done threads: $thr\t read size: $batch\t $d/320\t in $res_file\t time: $res\n"
        done
    done
