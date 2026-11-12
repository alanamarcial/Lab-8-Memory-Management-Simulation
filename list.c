#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "list.h"

/* All ranges are inclusive: size = end - start + 1. */
static int block_size(const block_t *blk) {
    return blk->end - blk->start + 1;
}

list_t *list_alloc(void) {
    list_t *list = malloc(sizeof(*list));
    if (!list) { perror("malloc"); exit(EXIT_FAILURE); }
    list->head = NULL;
    return list;
}

node_t *node_alloc(block_t *blk) {
    node_t *node = malloc(sizeof(*node));
    if (!node) { perror("malloc"); exit(EXIT_FAILURE); }
    node->next = NULL;
    node->blk = blk;
    return node;
}

void node_free(node_t *node) { free(node); }

void list_free(list_t *l) {
    if (!l) return;
    node_t *cur = l->head;
    while (cur) {
        node_t *next = cur->next;
        free(cur->blk);
        free(cur);
        cur = next;
    }
    free(l);
}

void list_print(list_t *l) {
    node_t *current = l->head;
    if (!current) printf("list is empty\n");
    while (current) {
        block_t *b = current->blk;
        printf("PID=%d START:%d END:%d", b->pid, b->start, b->end);
        current = current->next;
    }
}

int list_length(list_t *l) {
    int i = 0;
    for (node_t *c = l->head; c; c = c->next) i++;
    return i;
}

void list_add_to_back(list_t *l, block_t *blk) {
    node_t *newNode = node_alloc(blk);
    node_t **where = &l->head;
    while (*where) where = &(*where)->next;
    *where = newNode;
}

void list_add_to_front(list_t *l, block_t *blk) {
    node_t *newNode = node_alloc(blk);
    newNode->next = l->head;
    l->head = newNode;
}

void list_add_at_index(list_t *l, block_t *blk, int index) {
    if (index <= 0) { list_add_to_front(l, blk); return; }
    node_t **where = &l->head;
    for (int i = 0; i < index && *where; i++) where = &(*where)->next;
    node_t *node = node_alloc(blk);
    node->next = *where;
    *where = node;
}

void list_add_ascending_by_address(list_t *l, block_t *newblk) {
    node_t **where = &l->head;
    while (*where && (*where)->blk->start <= newblk->start)
        where = &(*where)->next;
    node_t *newNode = node_alloc(newblk);
    newNode->next = *where;
    *where = newNode;
}

void list_add_ascending_by_blocksize(list_t *l, block_t *newblk) {
    int size = block_size(newblk);
    node_t **where = &l->head;
    while (*where && block_size((*where)->blk) <= size)
        where = &(*where)->next;
    node_t *newNode = node_alloc(newblk);
    newNode->next = *where;
    *where = newNode;
}
