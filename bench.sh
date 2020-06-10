
file=$1 
    for thr in {1..16};do 
        for batch in {0..10};do
            res_file=$(echo res/bench-results-$file-$thr-$batch.csv)
            echo "" > $res_file
            for tests in {1..10};do
                res=$(go run main.go -f $file -t $thr -m $batch -q)
                echo  "$thr,$batch,$res" >> $res_file   
            done
            d=$((batch*thr+1))
            echo -ne "done threads: $thr\t read size: $batch\t $d/160\t in $res_file \033[0K\r"
        done
    done
