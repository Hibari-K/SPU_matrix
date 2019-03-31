# SPU_matrix_add
Matrix addition program with Cell Broadband Engine on PS3.  
This program uses one, two, or four SPU to caluculate 1024x1024 matrix add.

### demo

```
$ ./matrix_add 1
1 thread(s) running

Result: ***** No Error *****

Total time : 14.066828 [s]
Average time : 2.813366 [ms]

$ ./matrix_add 2
2 thread(s) running

Result: ***** No Error *****

Total time : 8.079125 [s]
Average time : 1.615825 [ms]

$ ./matrix_add 4
4 thread(s) running

Result: ***** No Error *****

Total time : 5.852762 [s]
Average time : 1.170552 [ms]
```
