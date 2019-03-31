#define ROW	1024
#define COL	1024

#define SPU_BUF	16*1024

#define TAG_P	1
#define TAG_A   2
#define TAG_B   3
#define TAG_C   4

typedef struct {
	unsigned long long ea_a;
	unsigned long long ea_b;
	unsigned long long ea_c;
	unsigned int start;
	unsigned int size;
} DMA_params_t;


