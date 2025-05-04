#pragma once

typedef struct Node {
    void* data;
    struct Node* next;
} Node;

typedef struct {
    Node* front;
    Node* rear;
    int size;
    void (*data_destroyer)(void*);
} Queue;

Queue* create_queue(void (*data_destroyer)(void*));

void enqueue(Queue* queue, void* data);

void* dequeue(Queue* queue);

void clear_queue(Queue* queue);

void destroy_queue(Queue* queue);
