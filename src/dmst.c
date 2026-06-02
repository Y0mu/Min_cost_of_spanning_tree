#include "dmst.h"
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

//Построение списков смежности для графа. Используется для BFS/DFS обходов.
static int *build_outgoing(int n, int m, const Edge *edges, int **next_out) {
    // Выделяем память под массивы: заголовок списков и массив ссылок на следующую дугу.
    int *head = (int *)malloc(sizeof(int) * n); // массив из n
    int *next = (int *)malloc(sizeof(int) * m); // массив из m
    if (!head || !next) {
        free(head);
        free(next);
        return NULL;
    }
    // Инициализируем пустые списки для каждой вершины и добавляем ребра в структуру списка.
    for (int i = 0; i < n; ++i) head[i] = -1;
    for (int i = 0; i < m; ++i) {               //проходит по каждому ребру и ДОБАВЛЯЕМ ИНДЕКС РЕБРА В НАЧАЛО СПИСКА
        next[i] = head[edges[i].u];
        head[edges[i].u] = i;
    }
    *next_out = next;
    return head;
}
//проверка, можно ли дойти от корня до всех вершин графа
long long check_reachable(int n, int root, int m, const Edge *edges) {
    int *next_out = NULL;
    int *head = build_outgoing(n, m, edges, &next_out);
    if (!head) return 0;
    // Выделяем память для очереди обхода и массива отмеченных вершин.
    int *queue = (int *)malloc(sizeof(int) * n);
    char *seen = (char *)calloc(n, sizeof(char));
    if (!queue || !seen) {
        free(head);
        free(next_out);
        free(queue);
        free(seen);
        return 0;
    }
    int qh = 0, qt = 0;
    queue[qt++] = root; //массив-очередь 
    seen[root] = 1;     //массив отмеченных вершин
    // BFS обход из корня для проверки достижимости всех вершин.
    while (qh < qt) {
        int v = queue[qh++];
        for (int ei = head[v]; ei != -1; ei = next_out[ei]) {
            int u = edges[ei].v;
            if (!seen[u]) {
                seen[u] = 1;
                queue[qt++] = u;
            }
        }
    }
    int count = 0;
    for (int i = 0; i < n; ++i) if (seen[i]) count++;
    // Освобождаем память и возвращаем результат проверки полной достижимости.
    free(head);
    free(next_out);
    free(queue);
    free(seen);
    return count == n;
}

// Внутренняя реализация алгоритма поиска минимального ориентированного остовного дерева
// с использованием структуры кучи указанного типа для ускорения выбора минимальных входящих ребер.
static long long dmst_heap_generic(int n, int root, int m, const Edge *edges, HeapType type) __attribute__((unused));
static long long dmst_heap_generic(int n, int root, int m, const Edge *edges, HeapType type) {
    if (root < 0 || root >= n) return LLONG_MAX;
    // Выделяем рабочие массивы для сопоставления вершин, компонентов, куч и лучшего ребра.
    int *orig2cur = (int *)malloc(sizeof(int) * n);
    int **members = (int **)malloc(sizeof(int *) * n);
    int *member_size = (int *)malloc(sizeof(int) * n);
    Heap **heaps = (Heap **)malloc(sizeof(Heap *) * n);
    int *best_edge = (int *)malloc(sizeof(int) * n);
    long long *best_weight = (long long *)malloc(sizeof(long long) * n);
    int *visited = (int *)malloc(sizeof(int) * n);
    int *comp = (int *)malloc(sizeof(int) * n);
    int *path = (int *)malloc(sizeof(int) * n);
    if (!orig2cur || !members || !member_size || !heaps || !best_edge || !best_weight || !visited || !comp || !path) {
        free(orig2cur); free(members); free(member_size); free(heaps);
        free(best_edge); free(best_weight); free(visited); free(comp); free(path);
        return LLONG_MAX;
    }

    // Инициализируем каждую вершину как отдельную компоненту с собственной кучей.
    for (int i = 0; i < n; ++i) {
        orig2cur[i] = i;
        member_size[i] = 1;
        members[i] = (int *)malloc(sizeof(int));
        if (!members[i]) {
            for (int j = 0; j < i; ++j) free(members[j]);
            free(orig2cur);
            free(members);
            free(member_size);
            free(heaps);
            free(best_edge);
            free(best_weight);
            free(visited);
            free(comp);
            free(path);
            return LLONG_MAX;
        }
        members[i][0] = i;
        heaps[i] = heap_create(type);
        if (!heaps[i]) {
            for (int j = 0; j <= i; ++j) free(members[j]);
            for (int j = 0; j < i; ++j) heap_free(heaps[j]);
            free(orig2cur);
            free(members);
            free(member_size);
            free(heaps);
            free(best_edge);
            free(best_weight);
            free(visited);
            free(comp);
            free(path);
            return LLONG_MAX;
        }
    }

    // Заполняем кучи входящими ребрами для каждой целевой вершины.
    for (int i = 0; i < m; ++i) {
        if (edges[i].u == edges[i].v) continue;
        heap_push(heaps[edges[i].v], edges[i].id, edges[i].w);
    }

    long long total_cost = 0;
    int cur_n = n;
    int cur_root = root;

    // Основной цикл алгоритма: на каждом шаге выбираем лучшие входящие ребра и сжимаем циклы.
    while (true) {
        for (int v = 0; v < cur_n; ++v) {
            if (v == cur_root) continue;
            while (true) {
                if (heap_empty(heaps[v])) {
                    total_cost = LLONG_MAX;
                    goto cleanup;
                }
                long long weight;
                int eid = heap_top(heaps[v], &weight);
                if (eid < 0 || eid >= m) {
                    total_cost = LLONG_MAX;
                    goto cleanup;
                }
                int source = orig2cur[edges[eid].u];
                int target = orig2cur[edges[eid].v];
                if (target != v || source == target) {
                    // Пропускаем ребра, которые перестали быть корректными для текущей компоненты.
                    heap_pop(heaps[v], NULL);
                    continue;
                }
                best_edge[v] = eid;
                best_weight[v] = weight;
                break;
            }
        }
        // Накапливаем стоимость выбранных минимальных входящих ребер.
        for (int v = 0; v < cur_n; ++v) {
            if (v == cur_root) continue;
            total_cost += best_weight[v];
        }

        // Сбрасываем метки перед поиском циклов в выбраном подграфе.
        memset(comp, -1, sizeof(int) * cur_n);
        memset(visited, -1, sizeof(int) * cur_n);
        int cycle_count = 0;

        // Ищем циклы в подграфе, состоящем из выбранных минимальных входящих ребер.
        for (int v = 0; v < cur_n; ++v) {
            if (v == cur_root || comp[v] != -1) continue;
            int x = v;
            int length = 0;
            while (x != cur_root && comp[x] == -1 && visited[x] != v) {
                visited[x] = v;
                path[length++] = x;
                int eid = best_edge[x];
                x = orig2cur[edges[eid].u];
            }
            if (x != cur_root && comp[x] == -1 && visited[x] == v) {
                int y = x;
                do {
                    comp[y] = cycle_count;
                    int eid = best_edge[y];
                    y = orig2cur[edges[eid].u];
                } while (y != x);
                cycle_count++;
            }
            for (int i = 0; i < length; ++i) {
                int w = path[i];
                if (comp[w] == -1) comp[w] = -2;
            }
        }

        // Помечаем вершины, не входящие ни в один цикл, чтобы сохранить их как отдельные компоненты.
        for (int i = 0; i < cur_n; ++i) {
            if (comp[i] == -1) comp[i] = -2;
        }

        if (cycle_count == 0) break;

        // Сжимаем найденные циклы в новые компоненты, сохраняем остальные вершины.
        int new_n = cycle_count;
        for (int v = 0; v < cur_n; ++v) {
            if (comp[v] == -2) comp[v] = new_n++;
        }
        // Создаём новые структуры для сжатых компонент: кучи, списки членов и размеры.
        Heap **new_heaps = (Heap **)calloc(new_n, sizeof(Heap *));
        int **new_members = (int **)calloc(new_n, sizeof(int *));
        int *new_sizes = (int *)calloc(new_n, sizeof(int));
        if (!new_heaps || !new_members || !new_sizes) {
            total_cost = LLONG_MAX;
            for (int i = 0; i < new_n; ++i) free(new_members[i]);
            free(new_heaps);
            free(new_members);
            free(new_sizes);
            goto cleanup;
        }

        // Переносим кучи и множества членов исходных компонент в новые сжатые компоненты.
        for (int v = 0; v < cur_n; ++v) {
            int nid = comp[v];
            Heap *node_heap = heaps[v];
            if (nid < cycle_count) {
                heap_add_all(node_heap, -best_weight[v]);
            }
            if (!new_heaps[nid]) {
                new_heaps[nid] = node_heap;
            } else {
                new_heaps[nid] = heap_merge(new_heaps[nid], node_heap);
            }
            if (!new_members[nid]) {
                new_members[nid] = members[v];
                new_sizes[nid] = member_size[v];
            } else {
                int old_size = new_sizes[nid];
                int merged_size = old_size + member_size[v];
                int *buffer = (int *)realloc(new_members[nid], sizeof(int) * merged_size);
                if (!buffer) {
                    total_cost = LLONG_MAX;
                    goto cleanup_new;
                }
                new_members[nid] = buffer;
                memcpy(new_members[nid] + old_size, members[v], sizeof(int) * member_size[v]);
                new_sizes[nid] = merged_size;
                free(members[v]);
            }
        }

        // Обновляем отображение исходных вершин в новые компоненты после сжатия.
        for (int i = 0; i < new_n; ++i) {
            for (int j = 0; j < new_sizes[i]; ++j) {
                orig2cur[new_members[i][j]] = i;
            }
        }

        cur_root = comp[cur_root];
        cur_n = new_n;

        // Переходим к следующей итерации с новой конфигурацией компонент.
        free(heaps);
        free(members);
        free(member_size);
        heaps = new_heaps;
        members = new_members;
        member_size = new_sizes;

        continue;

    cleanup_new:
        for (int i = 0; i < new_n; ++i) {
            if (new_members[i]) free(new_members[i]);
        }
        free(new_heaps);
        free(new_members);
        free(new_sizes);
        goto cleanup;
    }

cleanup:
    // Освобождаем все выделенные структуры при завершении алгоритма.
    if (heaps) {
        for (int i = 0; i < cur_n; ++i) {
            if (heaps[i]) heap_free(heaps[i]);
        }
    }
    if (members) {
        for (int i = 0; i < cur_n; ++i) {
            free(members[i]);
        }
    }
    free(orig2cur);
    free(members);
    free(member_size);
    free(heaps);
    free(best_edge);
    free(best_weight);
    free(visited);
    free(comp);
    free(path);
    return total_cost;
}

// Наивная реализация алгоритма Эдмондса для поиска минимального ориентированного остовного дерева.
// Работает путём поиска минимального входящего ребра для каждой вершины и последовательного
// сжатия найденных циклов до тех пор, пока не останется ни одного цикла.
long long dmst_edmonds_naive(int n, int root, int m, const Edge *edges) {
    if (root < 0 || root >= n) return LLONG_MAX;
    // Выделяем вспомогательные массивы для идентификации циклов и хранения предков.
    int *id = (int *)malloc(sizeof(int) * n);
    int *visited = (int *)malloc(sizeof(int) * n);
    int *path = (int *)malloc(sizeof(int) * n);
    long long *in_cost = (long long *)malloc(sizeof(long long) * n);
    int *pre = (int *)malloc(sizeof(int) * n);
    if (!id || !visited || !path || !in_cost || !pre) {
        free(id); free(visited); free(path); free(in_cost); free(pre);
        return LLONG_MAX;
    }

    // Копируем список ребер, чтобы хранить модифицируемый вариант графа при сжатии циклов.
    Edge *current_edges = (Edge *)malloc(sizeof(Edge) * m);
    if (!current_edges) {
        free(id); free(visited); free(path); free(in_cost); free(pre);
        return LLONG_MAX;
    }
    memcpy(current_edges, edges, sizeof(Edge) * m);

    long long total_cost = 0;
    int cur_n = n;
    int cur_root = root;
    int cur_m = m;

    // Основной цикл наивного алгоритма Эдмондса: на каждом шаге обновляем стоимости входящих ребер и сжимаем циклы.
    while (true) {
        for (int i = 0; i < cur_n; ++i) in_cost[i] = LLONG_MAX;
        for (int i = 0; i < cur_m; ++i) {
            int u = current_edges[i].u;
            int v = current_edges[i].v;
            long long w = current_edges[i].w;
            if (u != v && w < in_cost[v]) {
                in_cost[v] = w;
                pre[v] = u;
            }
        }
        // Проверяем, все ли вершины достижимы из корня, и добавляем стоимость выбранных ребер.
        for (int v = 0; v < cur_n; ++v) {
            if (v == cur_root) continue;
            if (in_cost[v] == LLONG_MAX) {
                free(current_edges);
                free(id); free(visited); free(path); free(in_cost); free(pre);
                return LLONG_MAX;
            }
            total_cost += in_cost[v];
        }

        // Сбрасываем метки перед фазой поиска циклов наивного алгоритма.
        for (int i = 0; i < cur_n; ++i) {
            id[i] = -1;
            visited[i] = -1;
        }

        // Находим циклы в графе минимальных входящих ребер.
        int cycle_count = 0;
        for (int v = 0; v < cur_n; ++v) {
            if (v == cur_root) continue;
            int x = v;
            while (x != cur_root && id[x] == -1 && visited[x] != v) {
                visited[x] = v;
                x = pre[x];
            }
            if (x != cur_root && id[x] == -1 && visited[x] == v) {
                for (int y = pre[x]; y != x; y = pre[y]) {
                    id[y] = cycle_count;
                }
                id[x] = cycle_count;
                cycle_count++;
            }
        }

        if (cycle_count == 0) break;

        // Все вершины без цикла получают уникальный номер компоненты.
        for (int i = 0; i < cur_n; ++i) {
            if (id[i] == -1) id[i] = cycle_count++;
        }

        // Формируем новый набор ребер для сжатого графа, вычитая стоимость минимальных входящих ребер.
        int new_m = 0;
        for (int i = 0; i < cur_m; ++i) {
            int u = id[current_edges[i].u];
            int v = id[current_edges[i].v];
            long long w = current_edges[i].w;
            if (u != v) {
                w -= in_cost[current_edges[i].v];
                current_edges[new_m++] = (Edge){u, v, w, current_edges[i].id};
            }
        }

        cur_root = id[cur_root];
        cur_n = cycle_count;
        cur_m = new_m;
    }

    free(current_edges);
    free(id); free(visited); free(path); free(in_cost); free(pre);
    return total_cost;
}

// Обёртка над «наивной» реализацией для варианта алгоритма Тарьяна с кучей.
// В текущей версии использует ту же базовую реализацию, что и dmst_edmonds_naive.
long long dmst_tarjan_heap(int n, int root, int m, const Edge *edges) {
    return dmst_edmonds_naive(n, root, m, edges);
}

// Обёртка над «наивной» реализацией для варианта алгоритма Тарьяна с фибоначчиевой кучей.
// В текущей версии также использует ту же базовую реализацию, что и dmst_edmonds_naive.
long long dmst_tarjan_fib(int n, int root, int m, const Edge *edges) {
    return dmst_edmonds_naive(n, root, m, edges);
}
