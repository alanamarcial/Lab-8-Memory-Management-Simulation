#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "list.h"
#include "util.h"

// Helper function to re-sort the free list based on policy (DRY principle)
static void resort_freelist(node_t **freelist, int policy) {
    node_t *new_free = NULL;
    node_t *curr = *freelist;
    while (curr != NULL) {
        if (curr->blk != NULL) {
            block_t *b = (block_t *)malloc(sizeof(block_t));
            b->pid = curr->blk->pid;
            b->start = curr->blk->start;
            b->end = curr->blk->end;
            
            if (policy == 1 || policy == 2) {
                list_add_ascending_by_blocksize(&new_free, b);
            } else if (policy == 3) {
                list_add_descending_by_blocksize(&new_free, b);
            }
        }
        curr = curr->next;
    }
    list_free(*freelist);
    *freelist = new_free;
}

void coalese_memory(node_t **freelist, int policy) {
    if (freelist == NULL || *freelist == NULL) return;

    // 1. Sort free list by address first to align adjacent blocks
    node_t *sorted_free = NULL;
    node_t *current = *freelist;
    while (current != NULL) {
        if (current->blk != NULL) {
            block_t *b = (block_t *)malloc(sizeof(block_t));
            b->pid = current->blk->pid;
            b->start = current->blk->start;
            b->end = current->blk->end;
            list_add_ascending_by_address(&sorted_free, b);
        }
        current = current->next;
    }
    
    list_free(*freelist);
    *freelist = sorted_free;

    // 2. Perform coalescing on address-sorted list
    list_coalese_nodes(freelist);

    // 3. Re-sort free list based on policy using the DRY helper
    if (policy == 2) {
        resort_freelist(freelist, 2); // Best Fit (ascending size)
    } else if (policy == 3) {
        resort_freelist(freelist, 3); // Worst Fit (descending size)
    }
}

void allocate_memory(node_t **freelist, node_t **alloclist, int pid, int blocksize, int policy) {
    node_t *current = *freelist;
    node_t *prev = NULL;

    while (current != NULL) {
        if (current->blk != NULL) {
            int freesize = current->blk->end - current->blk->start + 1;
            if (freesize >= blocksize) {
                // Found a block large enough
                block_t *allocated_block = (block_t *)malloc(sizeof(block_t));
                allocated_block->pid = pid;
                allocated_block->start = current->blk->start;
                allocated_block->end = current->blk->start + blocksize - 1;

                // Add to allocated list in ascending address order
                list_add_ascending_by_address(alloclist, allocated_block);

                // Handle fragment if any remaining space
                if (freesize > blocksize) {
                    block_t *fragment = (block_t *)malloc(sizeof(block_t));
                    fragment->pid = 0;
                    fragment->start = current->blk->start + blocksize;
                    fragment->end = current->blk->end;

                    // Insert fragment back based on policy
                    if (policy == 1) { // First Fit (FIFO / back of list)
                        // Remove current node from free list
                        if (prev == NULL) {
                            *freelist = current->next;
                        } else {
                            prev->next = current->next;
                        }
                        free(current->blk);
                        free(current);

                        // Add fragment to back of freelist
                        list_add_to_back(freelist, fragment);
                        return;
                    } else if (policy == 2) { // Best Fit (ascending block size)
                        // Remove current node
                        if (prev == NULL) {
                            *freelist = current->next;
                        } else {
                            prev->next = current->next;
                        }
                        free(current->blk);
                        free(current);

                        list_add_ascending_by_blocksize(freelist, fragment);
                        return;
                    } else if (policy == 3) { // Worst Fit (descending block size)
                        if (prev == NULL) {
                            *freelist = current->next;
                        } else {
                            prev->next = current->next;
                        }
                        free(current->blk);
                        free(current);

                        list_add_descending_by_blocksize(freelist, fragment);
                        return;
                    }
                } else {
                    // Exact match, just remove node from free list
                    if (prev == NULL) {
                        *freelist = current->next;
                    } else {
                        prev->next = current->next;
                    }
                    free(current->blk);
                    free(current);
                    return;
                }
            }
        }
        prev = current;
        current = current->next;
    }

    printf("Error: Not Enough Memory\n");
}

void deallocate_memory(node_t **freelist, node_t **alloclist, int pid, int policy) {
    node_t *current = *alloclist;
    node_t *prev = NULL;

    while (current != NULL) {
        if (current->blk != NULL && current->blk->pid == pid) {
            // Remove from allocated list
            if (prev == NULL) {
                *alloclist = current->next;
            } else {
                prev->next = current->next;
            }

            block_t *blk = current->blk;
            blk->pid = 0; // Reset PID for free block
            free(current);

            // Insert back into free list based on policy
            if (policy == 1) {
                list_add_to_back(freelist, blk);
            } else if (policy == 2) {
                list_add_ascending_by_blocksize(freelist, blk);
            } else if (policy == 3) {
                list_add_descending_by_blocksize(freelist, blk);
            }
            return;
        }
        prev = current;
        current = current->next;
    }

    printf("Error: Can't deallocate PID %d\n", pid);
}