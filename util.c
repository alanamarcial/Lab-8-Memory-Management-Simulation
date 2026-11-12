#include <stdio.h>
#include <stdlib.h>
#include "util.h"

void parse_file(FILE *f, int input[][2], int *n, int *PARTITION_SIZE) {
    if (fscanf(f, "%d", PARTITION_SIZE) != 1 || *PARTITION_SIZE <= 0) {
        fprintf(stderr, "Error: Invalid partition size\n");
        exit(EXIT_FAILURE);
    }
    printf("PARTITION_SIZE = %d\n", *PARTITION_SIZE);

    /* fscanf returns 2 only when both PID and requested size were read. */
    while (*n < 200 && fscanf(f, "%d %d", &input[*n][0], &input[*n][1]) == 2)
        (*n)++;
}
