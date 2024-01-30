#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include "assert.h"
#include "../headers/functions.h"

int main() {
    double **original_matrix;
    double **test_matrix;
    int kern[3][3] = {{0, -1, 0}, {-1, 5, -1}, {0, -1, 0}};

    original_matrix = alloc_matrix();
    test_matrix = alloc_matrix();

    fill(original_matrix);
    compute(original_matrix, kern);
    fill(test_matrix);
    compute(test_matrix, kern);

    print(test_matrix);
    compare_matrix(original_matrix, test_matrix);

    // Free memory
    for (int i = 0; i < XDIM; i++) {
        free(test_matrix[i]);
        free(original_matrix[i]);
    }
    free(test_matrix);
    free(original_matrix);

    return 0;
}