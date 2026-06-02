#ifndef UNIONFIND_H
#define UNIONFIND_H

#include <stddef.h>

// Структура Union-Find для поддержания разбиения на непересекающиеся множества.
typedef struct UnionFind {
    int n;
    int *parent;
    int *rank;
} UnionFind;

UnionFind *uf_create(int n); // Создаёт структуру Union-Find для n элементов.
void uf_free(UnionFind *uf); // Уничтожает структуру Union-Find.
int uf_find(UnionFind *uf, int x); // Находит представителя множества элемента x.
int uf_union(UnionFind *uf, int a, int b); // Объединяет множества, содержащие a и b.

#endif // UNIONFIND_H
