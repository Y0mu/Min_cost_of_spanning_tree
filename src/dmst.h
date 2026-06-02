#ifndef DMST_H
#define DMST_H

#include "heap.h"

// Проверяет достижимость всех вершин из заданного корня.
long long check_reachable(int n, int root, int m, const Edge *edges);
// Наивная реализация алгоритма Эдмондса для минимального ориентированного остовного дерева.
long long dmst_edmonds_naive(int n, int root, int m, const Edge *edges);
// Реализация поиска минимального остовного дерева с использованием бинарной кучи.
long long dmst_tarjan_heap(int n, int root, int m, const Edge *edges);
// Реализация поиска минимального остовного дерева с использованием фибоначчиевой кучи.
long long dmst_tarjan_fib(int n, int root, int m, const Edge *edges);

#endif // DMST_H
