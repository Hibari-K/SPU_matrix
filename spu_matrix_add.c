#include<stdio.h>
#include<spu_intrinsics.h>
#include<spu_mfcio.h>
#include "matrix_add.h"


DMA_params_t dp_ls __attribute__((aligned(16)));

//float A_ls[SPU_BUF/8] __attribute__((aligned(128)));
//float B_ls[SPU_BUF/8] __attribute__((aligned(128)));
//float C_ls[SPU_BUF/8] __attribute__((aligned(128)));
float A_ls[16][1024] __attribute__((aligned(128)));
float B_ls[16][1024] __attribute__((aligned(128)));
float C_ls[16][1024] __attribute__((aligned(128)));

int main(unsigned long long spe, unsigned long long argp){
  
    int i, dma_loop;
    int rem, n_dma_blk;
    int dma_size = SPU_BUF; // DMA transfer size at once

    //n_dma_blk = (((dp_ls.size)+((SPU_BUF)-1)) / (SPU_BUF));

    mfc_get(&dp_ls, argp, sizeof(DMA_params_t), TAG_P, 0, 0);
    mfc_write_tag_mask(1 << TAG_P);
    mfc_read_tag_status_any();

    rem = dp_ls.size;
    n_dma_blk = rem / dma_size;

    for(dma_loop=0; dma_loop < n_dma_blk; dma_loop++){

        vec_float4 *v_a = (vec_float4*) A_ls;
        vec_float4 *v_b = (vec_float4*) B_ls;
        vec_float4 *v_c = (vec_float4*) C_ls;
    
        mfc_get(&A_ls, dp_ls.ea_a+(dma_size*dma_loop), dma_size, TAG_A, 0, 0);
        mfc_get(&B_ls, dp_ls.ea_b+(dma_size*dma_loop), dma_size, TAG_B, 0, 0);
        mfc_write_tag_mask(1 << TAG_A);
        mfc_write_tag_mask(1 << TAG_B);
        mfc_read_tag_status_all();

        for(i=0; i<SPU_BUF/16; i++){ // 16byte per operation
            v_c[i] = spu_add(v_a[i], v_b[i]);
        }

        mfc_put(&C_ls, dp_ls.ea_c+(dma_size*dma_loop), dma_size, TAG_C, 0, 0);
        mfc_write_tag_mask(1 << TAG_C);
        mfc_read_tag_status_any();
    }

    return 0;
}
