#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "list.h"
#include "util.h"

void TOUPPER(char *arr) {
    for (size_t i = 0; arr[i] != '\0'; i++)
        arr[i] = (char)toupper((unsigned char)arr[i]);
}

void get_input(char *args[], int input[][2], int *n, int *size, int *policy) {
    FILE *input_file = fopen(args[1], "r");
    if (!input_file) {
        fprintf(stderr, "Error: Invalid filepath\n");
        exit(EXIT_FAILURE);
    }
    parse_file(input_file, input, n, size);
    fclose(input_file);
    TOUPPER(args[2]);
    if (!strcmp(args[2], "-F") || !strcmp(args[2], "-FIFO"))
        *policy = 1;
    else if (!strcmp(args[2], "-B") || !strcmp(args[2], "-BESTFIT"))
        *policy = 2;
    else if (!strcmp(args[2], "-W") || !strcmp(args[2], "-WORSTFIT"))
        *policy = 3;
    else {
        printf("usage: ./mmu <input file> -{F | B | W }  \n(F=FIFO | B=BESTFIT | W-WORSTFIT)\n");
        exit(EXIT_FAILURE);
    }
}

static void add_free_block(list_t *freelist, block_t *blk, int policy) {
    if (policy == 1)
        list_add_to_back(freelist, blk);
    else if (policy == 2)
        list_add_ascending_by_blocksize(freelist, blk);
    else
        list_add_descending_by_blocksize(freelist, blk);
}

void allocate_memory(list_t *freelist, list_t *alloclist, int pid,
                     int blocksize, int policy) {
    if (blocksize <= 0) {
        printf("Error: Not Enough Memory\n");
        return;
    }

    /* After coalescing, the free list is address-ordered, so select the
       smallest/largest qualifying block explicitly for best/worst fit. */
    int selected = -1;
    int selected_size = 0;
    int i = 0;
    for (node_t *c = freelist->head; c; c = c->next, i++) {
        int size = c->blk->end - c->blk->start + 1;
        if (size < blocksize) continue;
        if (selected == -1 ||
            (policy == 2 && size < selected_size) ||
            (policy == 3 && size > selected_size)) {
            selected = i;
            selected_size = size;
        }
        if (policy == 1) break; /* first fit: take first suitable block */
    }
    if (selected == -1) {
        printf("Error: Not Enough Memory\n");
        return;
    }

    block_t *blk = list_remove_at_index(freelist, selected);
    int original_end = blk->end;
    blk->pid = pid;
    blk->end = blk->start + blocksize - 1;
    list_add_ascending_by_address(alloclist, blk);

    if (blk->end < original_end) {
        block_t *fragment = malloc(sizeof(*fragment));
        if (!fragment) { perror("malloc"); exit(EXIT_FAILURE); }
        fragment->pid = 0;
        fragment->start = blk->end + 1;
        fragment->end = original_end;
        add_free_block(freelist, fragment, policy);
    }
}

void deallocate_memory(list_t *alloclist, list_t *freelist, int pid, int policy) {
    int index = list_get_index_of_by_Pid(alloclist, pid);
    if (index < 0) {
        printf("Error: Can't locate Memory Used by PID: %d\n", pid);
        return;
    }
    block_t *blk = list_remove_at_index(alloclist, index);
    blk->pid = 0;
    add_free_block(freelist, blk, policy);
}