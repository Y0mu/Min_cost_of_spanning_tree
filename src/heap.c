#include "heap.h"
#include "fibheap.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

typedef struct BinaryHeap {
    int capacity;
    int size;
    long long delta;
    int *edge_ids;
    long long *keys;
} BinaryHeap;

struct Heap {
    HeapType type;
    void *impl;
};

// Создаёт пустую бинарную кучу и инициализирует её служебные поля.
static BinaryHeap *binary_heap_create(void) {
    BinaryHeap *h = (BinaryHeap *)malloc(sizeof(BinaryHeap));
    if (!h) return NULL;
    h->capacity = 16;
    h->size = 0;
    h->delta = 0;
    h->edge_ids = (int *)malloc(sizeof(int) * h->capacity);
    h->keys = (long long *)malloc(sizeof(long long) * h->capacity);
    if (!h->edge_ids || !h->keys) {
        free(h->edge_ids);
        free(h->keys);
        free(h);
        return NULL;
    }
    return h;
}

// Уничтожает бинарную кучу и освобождает связанную память.
static void binary_heap_destroy(BinaryHeap *h) {
    if (!h) return;
    free(h->edge_ids);
    free(h->keys);
    free(h);
}

// Меняет местами два элемента кучи по индексам.
static void binary_heap_swap(BinaryHeap *h, int i, int j) {
    int temp_id = h->edge_ids[i];
    long long temp_key = h->keys[i];
    h->edge_ids[i] = h->edge_ids[j];
    h->keys[i] = h->keys[j];
    h->edge_ids[j] = temp_id;
    h->keys[j] = temp_key;
}

// Поднимает элемент вверх по дереву, чтобы восстановить свойство кучи.
static void binary_heap_sift_up(BinaryHeap *h, int idx) {
    while (idx > 0) {
        int parent = (idx - 1) / 2;
        if (h->keys[idx] < h->keys[parent]) {
            binary_heap_swap(h, idx, parent);
            idx = parent;
        } else {
            break;
        }
    }
}

// Опускает элемент вниз по дереву, чтобы восстановить порядок после удаления корня.
static void binary_heap_sift_down(BinaryHeap *h, int idx) {
    while (true) {
        int left = idx * 2 + 1;
        int right = idx * 2 + 2;
        int best = idx;
        if (left < h->size && h->keys[left] < h->keys[best]) best = left;
        if (right < h->size && h->keys[right] < h->keys[best]) best = right;
        if (best == idx) break;
        binary_heap_swap(h, idx, best);
        idx = best;
    }
}

// Добавляет элемент в бинарную кучу с возможным ростом массива.
static void binary_heap_push(BinaryHeap *h, int edge_id, long long key) {
    if (h->size >= h->capacity) {
        int new_capacity = h->capacity * 2;
        int *new_ids = (int *)realloc(h->edge_ids, sizeof(int) * new_capacity);
        long long *new_keys = (long long *)realloc(h->keys, sizeof(long long) * new_capacity);
        if (!new_ids || !new_keys) return;
        h->edge_ids = new_ids;
        h->keys = new_keys;
        h->capacity = new_capacity;
    }
    h->edge_ids[h->size] = edge_id;
    h->keys[h->size] = key - h->delta;
    binary_heap_sift_up(h, h->size);
    h->size++;
}

// Возвращает идентификатор минимального элемента в бинарной куче без удаления.
static int binary_heap_top(const BinaryHeap *h, long long *key_out) {
    if (h->size == 0) return -1;
    if (key_out) *key_out = h->keys[0] + h->delta;
    return h->edge_ids[0];
}

// Удаляет и возвращает минимальный элемент из бинарной кучи.
static int binary_heap_pop(BinaryHeap *h, long long *key_out) {
    if (h->size == 0) return -1;
    int edge_id = h->edge_ids[0];
    if (key_out) *key_out = h->keys[0] + h->delta;
    h->size--;
    if (h->size > 0) {
        h->edge_ids[0] = h->edge_ids[h->size];
        h->keys[0] = h->keys[h->size];
        binary_heap_sift_down(h, 0);
    }
    return edge_id;
}

// Смещает все ключи бинарной кучи на заданное значение без перебора элементов.
static void binary_heap_add_all(BinaryHeap *h, long long delta) {
    h->delta += delta;
}

// Объединяет две бинарные кучи в одну, сохраняя инвариант минимальной кучи.
static BinaryHeap *binary_heap_merge(BinaryHeap *a, BinaryHeap *b) {
    if (!a) return b;
    if (!b) return a;
    if (b->size == 0) {
        binary_heap_destroy(b);
        return a;
    }
    if (a->size == 0) {
        free(a->edge_ids);
        free(a->keys);
        a->edge_ids = b->edge_ids;
        a->keys = b->keys;
        a->size = b->size;
        a->capacity = b->capacity;
        a->delta = b->delta;
        free(b);
        return a;
    }
    long long target_delta = a->delta;
    long long source_delta = b->delta;
    long long delta_shift = source_delta - target_delta;
    for (int i = 0; i < b->size; ++i) {
        b->keys[i] += delta_shift;
    }
    int total_size = a->size + b->size;
    if (total_size > a->capacity) {
        int new_capacity = a->capacity;
        while (new_capacity < total_size) new_capacity *= 2;
        int *new_ids = (int *)realloc(a->edge_ids, sizeof(int) * new_capacity);
        long long *new_keys = (long long *)realloc(a->keys, sizeof(long long) * new_capacity);
        if (!new_ids || !new_keys) {
            return a;
        }
        a->edge_ids = new_ids;
        a->keys = new_keys;
        a->capacity = new_capacity;
    }
    memcpy(a->edge_ids + a->size, b->edge_ids, sizeof(int) * b->size);
    memcpy(a->keys + a->size, b->keys, sizeof(long long) * b->size);
    a->size = total_size;
    for (int i = (a->size / 2) - 1; i >= 0; --i) {
        binary_heap_sift_down(a, i);
    }
    free(b->edge_ids);
    free(b->keys);
    free(b);
    return a;
}

// Создаёт обёртку Heap для выбранного типа кучи.
Heap *heap_create(HeapType type) {
    Heap *heap = (Heap *)malloc(sizeof(Heap));
    if (!heap) return NULL;
    heap->type = type;
    heap->impl = NULL;
    if (type == HEAP_BINARY) {
        heap->impl = binary_heap_create();
    } else {
        heap->impl = fibheap_create();
    }
    if (!heap->impl) {
        free(heap);
        return NULL;
    }
    return heap;
}

// Освобождает память для обёртки Heap и её внутренней реализации.
void heap_free(Heap *heap) {
    if (!heap) return;
    if (heap->type == HEAP_BINARY) {
        binary_heap_destroy((BinaryHeap *)heap->impl);
    } else {
        fibheap_free((FibHeap *)heap->impl);
    }
    free(heap);
}

// Вставляет элемент в выбранную реализацию кучи.
void heap_push(Heap *heap, int edge_id, long long key) {
    if (!heap) return;
    if (heap->type == HEAP_BINARY) {
        binary_heap_push((BinaryHeap *)heap->impl, edge_id, key);
    } else {
        fibheap_insert((FibHeap *)heap->impl, edge_id, key);
    }
}

// Проверяет, пуста ли куча.
bool heap_empty(const Heap *heap) {
    if (!heap) return true;
    if (heap->type == HEAP_BINARY) {
        return ((BinaryHeap *)heap->impl)->size == 0;
    }
    return fibheap_empty((FibHeap *)heap->impl);
}

// Возвращает корневой элемент кучи без удаления.
int heap_top(const Heap *heap, long long *key_out) {
    if (!heap) return -1;
    if (heap->type == HEAP_BINARY) {
        return binary_heap_top((BinaryHeap *)heap->impl, key_out);
    }
    return fibheap_top((FibHeap *)heap->impl, key_out);
}

// Удаляет корневой элемент из кучи и возвращает его идентификатор.
int heap_pop(Heap *heap, long long *key_out) {
    if (!heap) return -1;
    if (heap->type == HEAP_BINARY) {
        return binary_heap_pop((BinaryHeap *)heap->impl, key_out);
    }
    return fibheap_pop((FibHeap *)heap->impl, key_out);
}

// Сдвигает все ключи в куче на одно и то же значение.
void heap_add_all(Heap *heap, long long delta) {
    if (!heap) return;
    if (heap->type == HEAP_BINARY) {
        binary_heap_add_all((BinaryHeap *)heap->impl, delta);
    } else {
        fibheap_add_all((FibHeap *)heap->impl, delta);
    }
}

// Объединяет две кучи одного типа в одну, возвращая новую кучу.
Heap *heap_merge(Heap *heap_a, Heap *heap_b) {
    if (!heap_a) return heap_b;
    if (!heap_b) return heap_a;
    if (heap_a->type != heap_b->type) {
        return heap_b;
    }
    if (heap_a->type == HEAP_BINARY) {
        heap_a->impl = binary_heap_merge((BinaryHeap *)heap_a->impl, (BinaryHeap *)heap_b->impl);
        free(heap_b);
        return heap_a;
    }
    FibHeap *a_impl = (FibHeap *)heap_a->impl;
    FibHeap *b_impl = (FibHeap *)heap_b->impl;
    heap_a->impl = fibheap_merge(a_impl, b_impl);
    free(heap_b);
    return heap_a;
}
