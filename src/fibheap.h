#ifndef FIBHEAP_H
#define FIBHEAP_H

#include <stdbool.h>

// Абстракция Фибоначчиевой кучи для оптимальных операций слияния и извлечения.
typedef struct FibHeap FibHeap;

FibHeap *fibheap_create(void); // Создаёт новую Фибоначчиеву кучу.
void fibheap_free(FibHeap *heap); // Освобождает кучу.
void fibheap_insert(FibHeap *heap, int edge_id, long long key); // Вставляет элемент.
bool fibheap_empty(const FibHeap *heap); // Проверяет, пуста ли куча.
int fibheap_top(const FibHeap *heap, long long *key_out); // Возвращает минимальный элемент.
int fibheap_pop(FibHeap *heap, long long *key_out); // Удаляет минимальный элемент.
void fibheap_add_all(FibHeap *heap, long long delta); // Прибавляет значение ко всем ключам.
FibHeap *fibheap_merge(FibHeap *a, FibHeap *b); // Сливает две кучи.

#endif // FIBHEAP_H
