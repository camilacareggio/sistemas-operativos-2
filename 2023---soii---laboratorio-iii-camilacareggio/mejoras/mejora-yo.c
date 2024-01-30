#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include "assert.h"

int XDIM = 100;
int YDIM = 100;

// We return the pointer
double **alloc_matrix(void) {
    /* Check if allocation succeeded. (check for NULL pointer) */
    double **array = (double **) malloc((size_t)XDIM * sizeof(double *));
    if (array == NULL) {
        return NULL; // Return NULL if the allocation fails
    }
    for (int i = 0; i < XDIM; i++) {
        array[i] = (double *) malloc((size_t)YDIM * sizeof(double));
    }
    return array;
}


void fill(double** arr) {
    /* fill the array with random values */
    int i, j;
    time_t t1; 
    srand ( (unsigned) time (&t1));
    for(i = 0 ; i < XDIM ; i++)
        for(j = 0 ; j < YDIM ; j++)
            arr[i][j] = (double)(rand() % 100);
}

void compute(double** arr, int kern[3][3]){
    /* Computes the kernell */
    double tmp_sum[9];
    double dato, accum=0;
    int i, j, k, l;
    for(i = 0 ; i < XDIM ; i++)
        for(j = 0 ; j < YDIM ; j++){
            if(i >= 1 && j >=1 && i < XDIM-1 && j <YDIM-1){

                for(k = 0; k < 3; k++){
                    int y = j + (k-1); // saco este calculo de un for
                    for(l = 0; l < 3; l++){
                        int x = i + (l-1);
                        dato = arr[x][y];
                        tmp_sum[l*3+k] = 0.004*kern[l][k]*dato + 1; // 2*2/1000 = 0.004
                    }
                }
                accum = 0;
                for(k = 0; k < 3; k++){
                    int factor = 3*k; // outside l for loop
                    for(l = 0; l < 3; l++)
                        accum = accum + tmp_sum[factor+l];
                }
            }
            arr[i][j] = accum;
        }    
}


void print(double** arr) {
    /* Print the array */
    int i, j;
    for(i = 0 ; i < XDIM ; i++)
        for(j = 0 ; j < YDIM ; j++)
            printf("array[%d][%d] = %f\n", i, j, arr[i][j]);
}

int main(void)
{
    double **arr;
    int kern[3][3] = {{0, -1, 0}, {-1, 5, -1}, {0, -1, 0}};

    arr = alloc_matrix();
    fill(arr);
    compute(arr, kern);
    print(arr);

    // for (int i = 0; i < XDIM; i++) {
    //     free(arr[i]);
    // }
    // free(arr);

    //unit_test();

    return 0;
}