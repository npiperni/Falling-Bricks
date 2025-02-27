#pragma once
#include <stdbool.h>

typedef struct {
    void** items;   // Pointer to an array of pointers
    int size;        // Current number of elements
    int capacity;    // Max capacity before resizing
    void (*data_destroyer)(void*);
} DynamicArray;

DynamicArray* create_dynamic_array(int initial_capacity, void (*data_destroyer)(void*));

void add_to_dynamic_array(DynamicArray* array, void* item);

void remove_from_dynamic_array(DynamicArray* array, void* item);

void* get_from_dynamic_array(const DynamicArray* array, int index);

void clear_dynamic_array(DynamicArray* array);

bool dynamic_array_contains(const DynamicArray* array, const void* item);

void destroy_dynamic_array(DynamicArray* array);