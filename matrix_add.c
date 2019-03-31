#include<stdio.h>
#include<stdlib.h>
#include<sys/time.h>
#include<libspe2.h>
#include<pthread.h>
#include "matrix_add.h"

#define LOOP 5000

typedef struct{
    spe_context_ptr_t spe_c;
    float *sub_A;
    float *sub_B;
    float *sub_C;
    unsigned int start;
    unsigned int size;
} thread_arg_t;


float A[ROW][COL] __attribute__((aligned(128)));
float B[ROW][COL] __attribute__((aligned(128)));
float C[ROW][COL] __attribute__((aligned(128)));


DMA_params_t dp;


void check_answer(){

    int i,j;

    int flag = 0;
    printf("\nResult: ");
    for(i=0; i<ROW; i++){
        for(j=0; j<COL; j++){
            if(C[i][j] != 3.0){
                printf("C[%d][%d] is not correct\n", i, j);
                flag = 1;
                break;
            }
        }
    }
    if(!flag) puts("***** No Error *****");
    puts("");
}


void *run_spe(void *thread_arg){
    
    thread_arg_t *arg = (thread_arg_t*) thread_arg;

    int ret;
    unsigned int entry;
    spe_stop_info_t stop_info;

    entry = SPE_DEFAULT_ENTRY;
    dp.ea_a = (unsigned long)arg->sub_A;
    dp.ea_b = (unsigned long)arg->sub_B;
    dp.ea_c = (unsigned long)arg->sub_C;
    dp.start = arg->start;
    dp.size = arg->size;

    // exec SPU
    ret = spe_context_run(arg->spe_c, &entry, 0, &dp, NULL, &stop_info);

    if(ret < 0){
        perror("spe_context_run");
        exit(1);
    }

    return 0;
}



int main(int argc, char **argv){
    
    int n_spe;
    double time;
    int ret;
    int start, rem;
    float *pA, *pB, *pC;

    spe_context_ptr_t spe_c[4]; // max thread
    pthread_t pt[4];
    thread_arg_t arg[4];

    if(argc != 2){
        n_spe = 1;
    }
    else{
        int num = atoi(argv[1]);

        if(num == 1 || num == 2 || num == 4){
            n_spe = num;
        }
        else{
            printf("Error: %d is invalid number. select 1, 2, or 4\n", num);
            exit(1);
        }
    }

    // initialization
    int i, j;
    for(i=0; i<ROW; i++){
        for(j=0; j<COL; j++){
            A[i][j] = 1.0;
            B[i][j] = 2.0;
        }
    }
    
    // check
    //for(i=0; i<8; i++) printf("%f ", A[0][i]); puts("");
    //for(i=0; i<8; i++) printf("%f ", B[0][i]); puts("");

    printf("%d thread(s) running\n", n_spe);

    spe_program_handle_t *spe_prog = spe_image_open("spu_matrix_add");
    if(!spe_prog){
        perror("spe_image_open");
        exit(1);
    }

    
    struct timeval s, e;
    gettimeofday(&s, NULL);
    
    for(i = 0; i < n_spe; i++){
        
        spe_c[i] = spe_context_create(0, NULL);
        if(!spe_c[i]){
            perror("spe_context_create");
            exit(1);
        }
        
        ret = spe_program_load(spe_c[i], spe_prog);
        if(ret){
            perror("spe_program_load");
            exit(1);
        }
    }

    
    int block = ROW / n_spe;

    int loop;
    for(loop = 0; loop < LOOP; loop++){

        pA = A[0];
        pB = B[0];
        pC = C[0];
        start = 0;
        rem = ROW;

        // assign to each SPU
        for(i=0; i<n_spe; i++){
            
            arg[i].sub_A = pA;
            arg[i].sub_B = pB;
            arg[i].sub_C = pC;
            arg[i].start = start;

            if(rem > block)
                arg[i].size = block * COL * sizeof(float);
            else
                arg[i].size = rem * COL * sizeof(float); // this is for fraction

            arg[i].spe_c = spe_c[i];
            
            ret = pthread_create(&pt[i], NULL, run_spe, &arg[i]);
            if(ret){
                perror("pthred_create");
                exit(1);
            }

            pA += arg[i].size / sizeof(float);
            pB += arg[i].size / sizeof(float);
            pC += arg[i].size / sizeof(float);

            start += block;
            rem -= block;
        }

        for(i=0; i<n_spe; i++){
            pthread_join(pt[i], NULL);
        }
    }

    for(i=0; i<n_spe; i++){
        
        ret = spe_context_destroy(spe_c[i]);
        if(ret){
            perror("spe_context_destroy");
            exit(1);
        }
    }
    
    gettimeofday(&e, NULL);

    ret = spe_image_close(spe_prog);
    if(ret){
        perror("spe_image_close");
        exit(1);
    }

    // check if return value is correct
    check_answer();

    
    time = (e.tv_sec - s.tv_sec) + (e.tv_usec - s.tv_usec)*1.0E-6;
    printf("Total time : %lf [s]\n", time);
    time = time / (1.0*LOOP);
    printf("Average time : %lf [ms]\n", time * 1000);


    return 0;
}
