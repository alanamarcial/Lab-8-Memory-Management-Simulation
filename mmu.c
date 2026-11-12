void coalese_memory(node_t **freelist, int policy) {
    if (freelist == NULL || *freelist == NULL) return;

    // 1. Sort free list by address first to make adjacent blocks side-by-side
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
    
    // Free the old list safely before overwriting pointer
    list_free(*freelist);
    *freelist = sorted_free;

    // 2. Perform coalescing on the address-sorted list
    list_coalese_nodes(freelist);

    // 3. If policy requires sorting by block size (Best Fit = 2, Worst Fit = 3)
    if (policy == 2) {
        node_t *resized_free = NULL;
        node_t *curr = *freelist;
        while (curr != NULL) {
            if (curr->blk != NULL) {
                block_t *b = (block_t *)malloc(sizeof(block_t));
                b->pid = curr->blk->pid;
                b->start = curr->blk->start;
                b->end = curr->blk->end;
                list_add_ascending_by_blocksize(&resized_free, b);
            }
            curr = curr->next;
        }
        list_free(*freelist);
        *freelist = resized_free;
    } else if (policy == 3) {
        node_t *resized_free = NULL;
        node_t *curr = *freelist;
        while (curr != NULL) {
            if (curr->blk != NULL) {
                block_t *b = (block_t *)malloc(sizeof(block_t));
                b->pid = curr->blk->pid;
                b->start = curr->blk->start;
                b->end = curr->blk->end;
                list_add_descending_by_blocksize(&resized_free, b);
            }
            curr = curr->next;
        }
        list_free(*freelist);
        *freelist = resized_free;
    }
}