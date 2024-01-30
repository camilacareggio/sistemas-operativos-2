#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define XDIM 100
#define YDIM 100

double **alloc_matrix(void);
void fill(double** arr);
void compute(double** arr, int kern[3][3]);
void print(double** arr);
double **new_alloc_matrix(void);
void new_compute(double** arr, int kern[3][3]);
int compare_matrix(double** arr1, double** arr2);

#endif