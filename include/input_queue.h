#pragma once
#include "../include/main.h"

typedef struct INPUT {
    neuron_idx_t target_index;
    neuron_mp_t value;
    struct INPUT* next;
    struct INPUT* prev;
} INPUT;

typedef struct INPUT_QUEUE {
    INPUT* head;
    INPUT* tail;
    neuron_idx_t size;
    neuron_idx_t max_size;
} INPUT_QUEUE;

INPUT_QUEUE* create_input_queue(neuron_idx_t max_size) {
    INPUT_QUEUE* input_queue = (INPUT_QUEUE*)malloc(sizeof(INPUT_QUEUE));
    input_queue->head = NULL;
    input_queue->tail = NULL;
    input_queue->size = 0;
    input_queue->max_size = max_size;
    return input_queue;
}

void insert_input(INPUT_QUEUE* input_queue, neuron_idx_t target_index, neuron_mp_t value) {

    if (input_queue->max_size -1 < target_index) {
        printf("Error: target index is out of range\n");
        return;
    }

    INPUT* new_input = (INPUT*)malloc(sizeof(INPUT));
    new_input->target_index = target_index;
    new_input->value = value;
    new_input->next = NULL;
    new_input->prev = NULL;

    if (input_queue->size == 0) {
        input_queue->head = new_input;
        input_queue->tail = new_input;
    } else {
        INPUT* cur_input = input_queue->head;
        while (cur_input != NULL) {
            if (cur_input->target_index == target_index) {
                cur_input->value += value;
                return;
            } else {
                cur_input = cur_input->next;
            }
        }
        input_queue->tail->next = new_input;
        new_input->prev = input_queue->tail;
        input_queue->tail = new_input;

    }
    input_queue->size++;
}

void clear_input_queue(INPUT_QUEUE* input_queue) {
    INPUT* cur_input = input_queue->head;
    while (cur_input != NULL) {
        INPUT* next_input = cur_input->next;
        free(cur_input);
        cur_input = next_input;
    }
    input_queue->head = NULL;
    input_queue->tail = NULL;
    input_queue->size = 0;
}

void print_input_queue(INPUT_QUEUE* input_queue) {
    INPUT* cur_input = input_queue->head;
    if (cur_input == NULL) {
        printf("Empty\n");
        return;
    }
    while (cur_input != NULL) {
        printf("target_index: %d, value: %d\n", cur_input->target_index, cur_input->value);
        cur_input = cur_input->next;
    }
}