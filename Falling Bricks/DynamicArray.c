#include "DynamicArray.h"
#include <stdio.h>
#include <stdlib.h>

DynamicArray* create_dynamic_array(int initial_capacity, void (*data_destroyer)(void*)) {
	if (initial_capacity <= 0) {
		fprintf(stderr, "Error: Initial capacity must be greater than 0\n");
		return NULL;
	}
    DynamicArray* array = malloc(sizeof(DynamicArray));
    if (!array) {
        fprintf(stderr, "Error: Memory allocation failed for DynamicArray\n");
        return NULL;
    }
    array->items = malloc(sizeof(void*) * initial_capacity);
    if (!array->items) {
        fprintf(stderr, "Error: Memory allocation failed for items array\n");
        free(array);
        return NULL;
    }
    array->size = 0;
    array->capacity = initial_capacity;
	array->data_destroyer = data_destroyer;
    return array;
}

void add_to_dynamic_array(DynamicArray* array, void* item) {
    if (array->size >= array->capacity) {
        // Double the capacity
        int new_capacity = array->capacity * 2;
        void** new_items = realloc(array->items, sizeof(void*) * new_capacity);
        if (!new_items) {
            fprintf(stderr, "Error: Memory allocation failed during resizing\n");
            return;
        }
        array->items = new_items;
        array->capacity = new_capacity;
    }
    array->items[array->size] = item;
    array->size++;
}

void remove_from_dynamic_array(DynamicArray* array, void* item) {
    for (int i = 0; i < array->size; i++) {
        if (array->items[i] == item) {
            // Shift elements left
            for (int j = i; j < array->size - 1; j++) {
                array->items[j] = array->items[j + 1];
            }
            array->size--;
            return;
        }
    }
}

void* get_from_dynamic_array(const DynamicArray* array, int index) {
    if (!array || index < 0 || index >= array->size) return NULL;
    return array->items[index];
}

void clear_dynamic_array(DynamicArray* array) {
	for (int i = 0; i < array->size; i++) {
		if (array->data_destroyer) {
			array->data_destroyer(array->items[i]);
		}
	}
	array->size = 0;
}

bool dynamic_array_contains(const DynamicArray* array, const void* item) {
	for (int i = 0; i < array->size; i++) {
		if (array->items[i] == item) {
			return true;
		}
	}
	return false;
}

void destroy_dynamic_array(DynamicArray* array) {
    if (!array) return;

    if (array->data_destroyer) {
        for (int i = 0; i < array->size; i++) {
            array->data_destroyer(array->items[i]);
        }
    }

    free(array->items);
    free(array);
}