#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <time.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/stat.h>
//#include "huffman.h"

#define MAX_THREADS 64


uint64_t thr_buf_sizes[10]={512, 1024, 2048, 4096, 8192, 12288, 16384, 20480 , 32768,65572};

uint8_t quietMode = 1;
typedef struct{
    pthread_mutex_t masta_l;
    pthread_mutex_t slave_l;
    uint16_t        signal_thr;
    int64_t        max_buffer;
    int64_t        act_size;
    uint64_t       frequenc[256];
    uint8_t        buffer[65600];
}thread_buffer;


typedef struct{
    uint32_t        thr_id;
    double          proc_time;
    double          worked_time;
}slave_data_t ;
thread_buffer   thread_data[MAX_THREADS]={0};
uint32_t num_slaves = 0;
uint32_t num_mastas = 0;
slave_data_t   slave_data[MAX_THREADS];

void init_threads(  uint32_t n_slaves, uint32_t buf_size_ind){
    int i;
    for ( i=0; i<n_slaves; i++) {
        slave_data[i].thr_id = i;
        slave_data[i].proc_time = 0;
        slave_data[i].worked_time = 0;

        pthread_mutex_init(&thread_data[i].masta_l,NULL);
        pthread_mutex_init(&thread_data[i].slave_l,NULL);
        thread_data[i].max_buffer = thr_buf_sizes[buf_size_ind];
        thread_data[i].act_size = 0;
    }
    return;
}

void deinit_threads(  uint32_t n_slaves, uint32_t buf_size_ind){
    int i;
    for ( i=0; i<n_slaves; i++) {
        free( thread_data[i].buffer );
        free(thread_data[i].frequenc);
    }
    return;
}

void*  reader_thread(void* args){
    slave_data_t* data = (slave_data_t*)args;
    time_t work_time = {0};
    data->worked_time = 0;
    data->proc_time = 0;
    time_t crnt_time;
    time_t start_time = clock();
    int32_t i;
    uint8_t chr;
    uint8_t sthr_doin = 1;
    while (sthr_doin){
        pthread_mutex_lock(&thread_data[data->thr_id].slave_l);
        sthr_doin = 0;
        crnt_time = clock();
        for(i=0;i<thread_data[data->thr_id].act_size;i++){
            sthr_doin = 1;
            chr = thread_data[data->thr_id].buffer[i];
            thread_data[data->thr_id].frequenc[chr]+=1;
        }
        data->worked_time += (double)(clock()-crnt_time);
        pthread_mutex_unlock(&thread_data[data->thr_id].masta_l);
    }
    if(quietMode) printf("thread finished: %d \n",data->thr_id);
    data->proc_time = (double)(clock()-start_time/CLOCKS_PER_SEC);
    data->worked_time= (double)(data->worked_time/CLOCKS_PER_SEC);
    if(quietMode) printf("%lf \n",data->proc_time);
    return NULL;
}
double mean_thr_time(int threads){
    int p;
    double result = 0;
    for (p = 0;p < threads;p++){
        result += slave_data[p].proc_time;
    }
    return  (result/threads)/CLOCKS_PER_SEC;
}
double mean_work(int threads){
    int p;
    double result = 0;
    for (p = 0;p < threads;p++){
        result += slave_data[p].worked_time;
    }
    return  result/threads;
}
int main(int argc,char* argv[]){
    double mean_proc_time;
    double mean_work_time;
    char filename[128]="./test.txt";
    int threads = 1;
    int i = 0;
    int buf = 0;
    int64_t read_size;
    int8_t reading = 1;
    uint8_t set_once = 1;
    uint8_t printTable = 0;
    void* result;
    uint64_t main_freq_table[256]={0};
    for (i = 0; i < argc; i++) {
        if (!strcmp(argv[i],"-f") || !strcmp(argv[i], "-file" )){
            strcpy(filename , argv[i+1]);
            i++;
        } else if (!strcmp( argv[i],"-q") || !strcmp(argv[i],"-quiet")) {
            quietMode = 0;
        } else if (!strcmp (argv[i], "-t") || !strcmp(argv[i],  "-threads")
                || !strcmp( argv[i], "-tasks")) {
            threads = atoi(argv[i+1]);
            i++;
        } else if (!strcmp( argv[i],"-m") || !strcmp(argv[i],"-meth")) {
            buf = atoi(argv[i+1]);
            i++;
        } else if (!strcmp(argv[i] , "-pt")) {
            printTable = 1;
        }
    }
    if (quietMode)printf("fname: %s \nthreads: %d buffer size: %d\n",filename,threads,(int)thr_buf_sizes[buf]);
    int fd = open(filename,O_RDONLY);

    pthread_t pool[MAX_THREADS];
    init_threads(threads, buf );
    for(i = 0;i<threads;i++) {
        pthread_mutex_lock(&thread_data[i].slave_l);
        pthread_create(&pool[i],NULL,reader_thread,&slave_data[i]);
    }
    while (reading){
endloop:
        for (i = 0;i<threads;i++){
            pthread_mutex_lock(&thread_data[i].masta_l);
            if (reading) {
                read_size = read(fd,thread_data[i].buffer,thr_buf_sizes[buf]);
                thread_data[i].act_size = read_size;
                if (read_size < thr_buf_sizes[buf]){
                    reading = 0;
                    pthread_mutex_unlock(&thread_data[i].slave_l);
                    goto endloop;
                }
            } else {
                thread_data[i].act_size = 0;
            }
            pthread_mutex_unlock(&thread_data[i].slave_l);
        }
    }
    for(i = 0;i<threads;i++) {
        pthread_join(pool[i],NULL);
    }

    int j=0;
    for (i =0;i<threads;i++){
        for (j=0;j<255;j++) {
            main_freq_table[j]+= thread_data[i].frequenc[j];
        }
    }
    uint64_t all_chars=0;
    if (printTable){
        for (j=0;j<255;j++) {
            if (main_freq_table[j]){
                all_chars  += main_freq_table[j];
                printf("%c - %lu \n",(char)j,main_freq_table[j]);
            }
        }
    }
    mean_proc_time = mean_thr_time(threads);
    mean_work_time = mean_work(threads);
    printf("%.2lf,%.2lf,%.2lf;\n",mean_proc_time,mean_work_time,mean_work_time/mean_proc_time);
    //deinit_threads(threads, buf );
    return 0;

}
