#include "../headers/functions.h"

// We return the pointer
double **alloc_matrix(void) /* Allocate the array */
{
    /* Check if allocation succeeded. (check for NULL pointer) */
    int i, j, k; 
    double **array;
    array = malloc(XDIM*sizeof(double *));
    for(i = 0 ; i < XDIM ; i++)
        array[i] = malloc(YDIM*sizeof(double) );
  
    for(j=0; j<XDIM; j++)
        for(k=0; k<YDIM; k++)
            memset(&array[k][j], j, sizeof(double));
    return array;
}

void fill(double** arr) {
    /* fill the array with random values */
    int i, j;
    for(i = 0 ; i < XDIM ; i++)
        for(j = 0 ; j < YDIM ; j++)
            arr[i][j] = 5;
}

void compute(double** arr, int kern[3][3]){
    /* Computes the kernell */
    double tmp_sum[9];
    double dato;
    double accum=0;
    int i, j, k, l;
    for(i = 0 ; i < XDIM ; i++)
        for(j = 0 ; j < YDIM ; j++){
            // printf("processing: %d - %d \n", i, j);
            if(i >= 1 && j >=1 && i < XDIM-1 && j <YDIM-1){
                for(k = 0; k < 3; k++)
                    for(l = 0; l < 3; l++){
                        int x = i + (l-1);
                        int y = j + (k-1);
                        dato = arr[x][y];
                        tmp_sum[l*3+k] = 2*(2*kern[l][k]*dato)/1000 + 1;
                    }

                accum = 0;
                for(k = 0; k < 3; k++)
                    for(l = 0; l < 3; l++)
                        accum = accum + tmp_sum[k*3+l];
  
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

// We return the pointer
double **new_alloc_matrix(void) {
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

void new_compute(double** arr, int kern[3][3]){
    /* Computes the kernell */
    double tmp_sum[9];
    double dato;
    double accum=0;
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

int compare_matrix(double** arr1, double** arr2){
    for(int i = 0 ; i < XDIM ; i++){
        for(int j = 0 ; j < YDIM ; j++){
            if(arr1[i][j] != arr2[i][j]){
                printf("Failed test [%d][%d]: %f != %f\n", i, j, arr1[i][j], arr2[i][j]);
                return -1;
            }
        }
    }
    printf("All tests passed!\n");
    return 1;
}