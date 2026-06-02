#ifndef HEAP_H
#define HEAP_H

#include <stdbool.h>
#include <stddef.h>

// Описание ребра ориентированного графа с весом и уникальным идентификатором.
typedef struct Edge {
    int u;
    int v;
    long long w;
    int id;
} Edge;

typedef enum HeapType {
    HEAP_BINARY,
    HEAP_FIBONACCI
} HeapType;

typedef struct Heap Heap;

Heap *heap_create(HeapType type); // Создаёт кучу заданного типа.
void heap_free(Heap *heap); // Уничтожает кучу.
void heap_push(Heap *heap, int edge_id, long long key); // Вставляет элемент в кучу.
bool heap_empty(const Heap *heap); // Проверяет, пуста ли куча.
int heap_top(const Heap *heap, long long *key_out); // Получает минимальный элемент без удаления.
int heap_pop(Heap *heap, long long *key_out); // Удаляет и возвращает минимальный элемент.
void heap_add_all(Heap *heap, long long delta); // Прибавляет значение ко всем ключам.
Heap *heap_merge(Heap *heap_a, Heap *heap_b); // Объединяет две кучи одного типа.

#endif // HEAP_H
