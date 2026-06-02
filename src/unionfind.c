#include "unionfind.h"
#include <stdlib.h>

// Создаёт структуру Union-Find для n элементов с начальной разметкой.
UnionFind *uf_create(int n) {
    UnionFind *uf = (UnionFind *)malloc(sizeof(UnionFind));
    if (!uf) return NULL;
    uf->n = n;
    uf->parent = (int *)malloc(sizeof(int) * n);
    uf->rank = (int *)malloc(sizeof(int) * n);
    if (!uf->parent || !uf->rank) {
        free(uf->parent);
        free(uf->rank);
        free(uf);
        return NULL;
    }
    for (int i = 0; i < n; ++i) {
        uf->parent[i] = i;
        uf->rank[i] = 0;
    }
    return uf;
}

// Освобождает память, выделенную под структуру Union-Find.
void uf_free(UnionFind *uf) {
    if (!uf) return;
    free(uf->parent);
    free(uf->rank);
    free(uf);
}

// Находит корень множества для элемента x с путевой компрессией.
int uf_find(UnionFind *uf, int x) {
    if (uf->parent[x] != x) {
        uf->parent[x] = uf_find(uf, uf->parent[x]);
    }
    return uf->parent[x];
}

// Объединяет множества, содержащие элементы a и b, по рангу.
int uf_union(UnionFind *uf, int a, int b) {
    int ra = uf_find(uf, a);
    int rb = uf_find(uf, b);
    if (ra == rb) return ra;
    if (uf->rank[ra] < uf->rank[rb]) {
        uf->parent[ra] = rb;
        return rb;
    }
    uf->parent[rb] = ra;
    if (uf->rank[ra] == uf->rank[rb]) {
        uf->rank[ra]++;
    }
    return ra;
}
