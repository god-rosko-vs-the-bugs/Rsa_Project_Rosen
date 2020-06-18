#include <cstddef>
#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <time.h>
#include <stdlib.h>
#include <assert.h>
//#include "huffman.h"

#define max_threads 64


uint64_t thr_buf_sizes[10]={512, 1024, 2048, 4096, 8192, 12288, 16384, 20480, 24576, 32768};

typedef struct{
    pthread_mutex_t masta_l;
    pthread_mutex_t slave_l;
    uint32_t        max_buffer;
    uint32_t        act_size;
    uint8_t*        buffer;
    uint64_t*       frequenc;
}thread_buffer;

typedef struct{
    uint32_t        rsp_thread_st;
    uint32_t        rsp_thread_en;
    uint64_t*       start_location;
    uint64_t*       to_read;
    char*           file_to_read;
}masta_data_t;

typedef struct{
    uint32_t        thr_id;
    double          proc_time;
    double          wasted_time;
}slave_data_t ;

thread_buffer   thread_data[max_threads];
uint32_t num_slaves = 0;
uint32_t num_mastas = 0;
slave_data_t*   slave_data = NULL;
masta_data_t*   masta_data = NULL;

void init_threads(  uint32_t n_slaves, uint32_t buf_size_ind, 
                    uint32_t n_masters,char* file){
    int i;
    uint32_t slaves_per_master = n_masters/n_slaves;
    uint64_t* read_size_pthr = malloc(sizeof(uint64_t)*n_slaves);

    //TODO read fiel get size and add give slave threads sizes and master 
    //     threads range arrays to read from 
    slave_data = (slave_data_t*)malloc(sizeof(slave_data_t)*n_slaves);
    assert(slave_data==NULL);
    for (i =0 ;i< n_slaves;i++) {

        slave_data[i].thr_id = i;
        slave_data[i].proc_time = 0;
        slave_data[i].wasted_time = 0;

        pthread_mutex_init(&thread_data[i].masta_l,NULL);
        pthread_mutex_init(&thread_data[i].slave_l,NULL);
        thread_data[i].max_buffer = thr_buf_sizes[buf_size_ind];
        thread_data[i].act_size = 0;

        thread_data[i].buffer = (uint8_t*)malloc((size_t)thread_data[i].max_buffer) ;
        assert(thread_data[i].buffer == NULL);

        thread_data[i].frequenc = (uint64_t*)malloc((size_t)sizeof(uint64_t)*255); 
        assert(thread_data[i].frequenc == NULL);
        }
    masta_data = (masta_data_t*)malloc(sizeof(masta_data_t)*n_masters);
    assert(masta_data == NULL);
    for (i = 0; i < n_masters;i++){
       masta_data[i].rsp_thread_st= i * slaves_per_master;
       masta_data[i].rsp_thread_en= (i+1) * slaves_per_master;
       }
    masta_data[n_slaves-1].rsp_thread_en += n_masters%n_slaves;

    return;

}
// @reader_thread takes slave_data_t cast to void pointer as arg
void reader_thread(void* args){
    slave_data_t* data = (slave_data_t*)args;
    // TODO make slave threads do work and take stats for efficiency
    return;
}

// @masta_thread takes masta_data_t* cast ot void pointer as arg
void masta_thread(void* args){
    masta_data_t* data = (masta_data_t*)args;
    // TODO make consimer threads do stuff
    return;
}


int main(int argc,char* argv[]){
    // TODO add args parsing call init and start threads properly, recall slave threads
    // first then recall master threads 
    return 0;
}
