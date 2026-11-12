#include <stdio.h>
#include <stdlib.h>
#include "list.h"

// Check if PID is in list
int list_is_in_by_pid(node_t *head, int pid) {
    node_t *current = head;
    while (current != NULL) {
        if (current->blk != NULL && current->blk->pid == pid) {
            return 1;
        }
        current = current->next;
    }
    return 0;
}

// Add ascending by block size (Best Fit)
void list_add_ascending_by_blocksize(node_t **head, block_t *blk) {
    node_t *new_node = (node_t *)malloc(sizeof(node_t));
    new_node->blk = blk;
    new_node->next = NULL;

    int new_size = blk->end - blk->start + 1;

    if (*head == NULL || ((*head)->blk->end - (*head)->blk->start) >= new_size) {
        new_node->next = *head;
        *head = new_node;
        return;
    }

    node_t *current = *head;
    while (current->next != NULL && (current->next->blk->end - current->next->blk->start) < new_size) {
        current = current->next;
    }
    new_node->next = current->next;
    current->next = new_node;
}

// Add descending by block size (Worst Fit)
void list_add_descending_by_blocksize(node_t **head, block_t *blk) {
    node_t *new_node = (node_t *)malloc(sizeof(node_t));
    new_node->blk = blk;
    new_node->next = NULL;

    int new_size = blk->end - blk->start + 1;

    if (*head == NULL || ((*head)->blk->end - (*head)->blk->start) <= new_size) {
        new_node->next = *head;
        *head = new_node;
        return;
    }

    node_t *current = *head;
    while (current->next != NULL && (current->next->blk->end - current->next->blk->start) > new_size) {
        current = current->next;
    }
    new_node->next = current->next;
    current->next = new_node;
}

// Coalesce physically adjacent nodes
// Coalesce physically adjacent nodes in a single efficient pass
void list_coalese_nodes(node_t **head) {
    if (head == NULL || *head == NULL) return;

    node_t *current = *head;
    while (current != NULL && current->next != NULL) {
        if (current->blk->end + 1 == current->next->blk->start) {
            // Merge next block into current block
            current->blk->end = current->next->blk->end;
            current->blk->pid = 0; // Explicitly set pid to 0 as required
            
            node_t *temp = current->next;
            current->next = temp->next;
            free(temp->blk);
            free(temp);
            // Do NOT restart from head; continue checking next adjacent nodes from current
        } else {
            current = current->next;
        }
    }
}

// Improved list print with proper formatting
void list_print(node_t *head) {
    node_t *current = head;
    while (current != NULL) {
        if (current->blk != NULL) {
            printf("PID %d [%d, %d] ", current->blk->pid, current->blk->start, current->blk->end);
        }
        current = current->next;
    }
    printf("\\n");
}