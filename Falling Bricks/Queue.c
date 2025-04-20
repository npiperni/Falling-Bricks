#include "Queue.h"
#include <stdlib.h>

Queue* create_queue(void (*data_destroyer)(void*)) {
    Queue* queue = (Queue*)malloc(sizeof(Queue));
    queue->front = queue->rear = NULL;
	queue->size = 0;
	queue->data_destroyer = data_destroyer;
    return queue;
}

void enqueue(Queue* queue, void* data) {
    Node* new_node = (Node*)malloc(sizeof(Node));
    new_node->data = data;
    new_node->next = NULL;
    queue->size++;

    if (!queue->rear) {
        queue->front = queue->rear = new_node;
        return;
    }

    queue->rear->next = new_node;
    queue->rear = new_node;
}

void* dequeue(Queue* queue) {
    if (!queue || !queue->front) return NULL;

    Node* temp = queue->front;
    void* data = temp->data;
    queue->front = queue->front->next;
    if (!queue->front) queue->rear = NULL;

    free(temp);
    queue->size--;
    return data;
}

void clear_queue(Queue* queue) {
	if (!queue) return;
    while (queue->front) {
        void* data = dequeue(queue);
        if (queue->data_destroyer) {
            queue->data_destroyer(data);  // Call user-defined free function
        }
    }
    queue->rear = NULL;
    queue->size = 0;
}

void destroy_queue(Queue* queue) {
    if (!queue) return;
	clear_queue(queue);
    free(queue);
}
