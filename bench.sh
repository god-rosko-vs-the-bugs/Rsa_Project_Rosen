
[ ! -d "./res3" ] && mkdir "./res"

file=$1
d=0
    for thr in {1..16};do 
        [ ! -d "./res/$thr" ] && mkdir "./res/$thr"
        for batch in {0..9};do
            res_file=$(echo res/$thr/bench-results-$file-$batch.csv)
            echo "" > $res_file
            for tests in {1..10};do
                res=$(go run main.go -f $file -t $thr -m $batch -q)
                echo  "$res" >> $res_file   
            done
            d=$((d+1))
            echo -ne "done threads: $thr\t read size: $batch\t $d/160\t in $res_file\n"
        done
    done
