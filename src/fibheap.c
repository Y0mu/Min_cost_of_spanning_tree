#include "fibheap.h"
#include <stdlib.h>
#include <string.h>

typedef struct FibNode
{                           // Структура узла Фибоначчиевой кучи
    int edge_id;            // edge_id - идентификатор ребра или элемента
    long long key;          // key - значение ключа для сравнения
    int degree;             // degree - число дочерних узлов в дереве
    struct FibNode *parent; // parent - указатель на родительский узел
    struct FibNode *left;
    struct FibNode *right; // left/right - соседние узлы в списке корневого списка или списка детей
    struct FibNode *child; // указатель на произвольного потомка
} FibNode;

struct FibHeap
{
    FibNode *min; // Указатель на минимальный элемент
    int size;     // Кол-во элементов
};

// Инициализирует новый узел Фибоначчиевой кучи.
static void fib_node_init(FibNode *node, int edge_id, long long key)
{
    node->edge_id = edge_id;
    node->key = key;
    node->degree = 0;
    node->parent = NULL;
    node->left = node;
    node->right = node;
    node->child = NULL;
}

// Добавляет узел в корневой список кучи.
static void fib_node_add_to_root_list(FibHeap *heap, FibNode *node)
{
    if (!heap->min)
    {
        heap->min = node;
        node->left = node;
        node->right = node;
    }
    else
    {
        node->right = heap->min;
        node->left = heap->min->left;
        heap->min->left->right = node;
        heap->min->left = node;
        if (node->key < heap->min->key)
        {
            heap->min = node;
        }
    }
}

// Сдвигает узел y в поддерево узла x при объединении.
static void fib_heap_link(FibNode *y, FibNode *x)
{
    y->left->right = y->right;
    y->right->left = y->left;
    y->parent = x;
    if (!x->child)
    {
        x->child = y;
        y->left = y;
        y->right = y;
    }
    else
    {   
        y->right = x->child;
        y->left = x->child->left;
        x->child->left->right = y;
        x->child->left = y;
    }
    x->degree++;
}

// Выполняет объединение корневого списка для уменьшения числа деревьев.
static FibNode *fib_heap_consolidate(FibHeap *heap)
{
    // Шаг 1: Если куча пуста, выходим
    if (!heap->min)
        return NULL;

    // Переменная для отслеживания максимальной степени
    int max_degree = 0;
    // Оцениваем максимальную возможную степень (ближайшая степень двойки > size)
    int estimated = 1;
    while (estimated <= heap->size)
        estimated <<= 1; // Удваиваем, пока не превысит size
    if (estimated > 64)
        estimated = 64; // Ограничиваем 64 (безопасный лимит)

    // Создаём массив "ящиков"
    FibNode **table = (FibNode **)calloc(estimated, sizeof(FibNode *));
    if (!table)
        return heap->min; // Если память не выделилась, возвращаем как есть

    // Создаём массив для временного хранения всех корневых узлов
    FibNode **root_list = (FibNode **)malloc(sizeof(FibNode *) * heap->size);
    if (!root_list)
    {
        free(table);
        return heap->min;
    }

    // Шаг 2: Собираем все корневые узлы в root_list
    int root_count = 0;
    FibNode *node = heap->min;
    do
    {
        root_list[root_count++] = node; // Кладём узел в список
        node = node->right;             // Переходим к следующему
        if (root_count > heap->size)
            break;
    } while (node != heap->min && root_count < heap->size);

    // Шаг 3: Объединяем деревья с одинаковой степенью
    for (int i = 0; i < root_count; ++i)
    {
        FibNode *current = root_list[i];
        int degree = current->degree;

        // Пока в ящике degree уже есть узел - объединяем
        while (degree < estimated && table[degree] != NULL)
        {
            FibNode *other = table[degree];
            // Делаем current меньшим (меньший ключ = корень)
            if (current->key > other->key)
            {
                FibNode *temp = current;
                current = other;
                other = temp;
            }
            // other становится ребёнком current
            fib_heap_link(other, current);
            table[degree] = NULL; // Очищаем ящик
            degree++;             // Степень current увеличилась, идём в следующий ящик
        }

        // Кладём current в ящик с его новой степенью
        if (degree < estimated)
        {
            table[degree] = current;
            if (degree > max_degree)
                max_degree = degree;
        }
    }

    // Шаг 4: Пересобираем корневой список из ящиков
    heap->min = NULL;
    for (int i = 0; i <= max_degree; ++i)
    {
        if (table[i] != NULL)
        {
            FibNode *entry = table[i];
            // Делаем узел отдельным кольцевым списком (сам с собой)
            entry->left = entry;
            entry->right = entry;

            if (!heap->min)
            {
                heap->min = entry; // Первый узел
            }
            else
            {
                // Вставляем в корневой список
                entry->right = heap->min;
                entry->left = heap->min->left;
                heap->min->left->right = entry;
                heap->min->left = entry;
                // Обновляем минимум
                if (entry->key < heap->min->key)
                    heap->min = entry;
            }
        }
    }

    // Шаг 5: Освобождаем временные массивы
    free(table);
    free(root_list);
    return heap->min; // Возвращаем новый минимум
}

// Создаёт пустую Фибоначчиеву кучу.
FibHeap *fibheap_create(void)
{
    FibHeap *heap = (FibHeap *)malloc(sizeof(FibHeap));
    if (heap)
    {
        heap->min = NULL;
        heap->size = 0;
    }
    return heap;
}

// Вставляет новый узел в Фибоначчиеву кучу. Каждый минимальный элемент обновляет указатель на минимум.
void fibheap_insert(FibHeap *heap, int edge_id, long long key)
{
    if (!heap)
        return;
    FibNode *node = (FibNode *)malloc(sizeof(FibNode));
    if (!node)
        return;
    fib_node_init(node, edge_id, key);
    if (!heap->min)
    {
        heap->min = node;
    }
    else
    {
        node->right = heap->min;
        node->left = heap->min->left;
        heap->min->left->right = node;
        heap->min->left = node;
        if (node->key < heap->min->key)
            heap->min = node;
    }
    heap->size++;
}

// Проверяет, пуста ли Фибоначчиева куча.
bool fibheap_empty(const FibHeap *heap)
{
    return !heap || heap->min == NULL;
}

// Возвращает минимальный элемент Фибоначчиевой кучи без удаления.
int fibheap_top(const FibHeap *heap, long long *key_out)
{
    if (!heap || !heap->min)
        return -1;
    if (key_out)
        *key_out = heap->min->key;
    return heap->min->edge_id;
}

// Удаляет и возвращает минимальный элемент из Фибоначчиевой кучи.
int fibheap_pop(FibHeap *heap, long long *key_out)
{
    if (!heap || !heap->min)
        return -1;

    FibNode *rem = heap->min;
    if (key_out)
        *key_out = rem->key;

    if (rem->child)
    {
        FibNode *child = rem->child;
        FibNode *start = child;
        do
        {
            FibNode *next = child->right;
            child->parent = NULL;
            fib_node_add_to_root_list(heap, child);
            child = next;
        } while (child != start);
    }

    rem->left->right = rem->right;
    rem->right->left = rem->left;
    if (rem == rem->right)
    {
        heap->min = NULL;
    }
    else
    {
        heap->min = rem->right;
        fib_heap_consolidate(heap);
    }

    int edge_id = rem->edge_id;
    free(rem);
    heap->size--;
    if (heap->size == 0)
        heap->min = NULL;
    return edge_id;
}

// Прибавляет значение ко всем ключам всех узлов кучи.
void fibheap_add_all(FibHeap *heap, long long delta)
{
    if (!heap || !heap->min)
        return;

    FibNode *stack[2048];
    int sp = 0;

    FibNode *start = heap->min;
    FibNode *node = start;
    do
    {
        stack[sp++] = node;
        node = node->right;
    } while (node != start);

    while (sp > 0)
    {
        FibNode *n = stack[--sp];
        n->key += delta;
        if (n->child)
        {
            FibNode *child = n->child;
            FibNode *child_start = child;
            do
            {
                stack[sp++] = child;
                child = child->right;
            } while (child != child_start);
        }
    }
}

// Объединяет две Фибоначчиевы кучи в одну, сохраняя минимальный элемент.
FibHeap *fibheap_merge(FibHeap *a, FibHeap *b)
{
    if (!a)
        return b;
    if (!b)
        return a;
    if (!a->min)
    {
        fibheap_free(a);
        return b;
    }
    if (!b->min)
    {
        fibheap_free(b);
        return a;
    }

    a->min->right->left = b->min->left;
    b->min->left->right = a->min->right;
    a->min->right = b->min;
    b->min->left = a->min;

    if (b->min->key < a->min->key)
        a->min = b->min;
    a->size += b->size;
    free(b);
    return a;
}

// Освобождает память, занятую Фибоначчиевой кучей и всеми её узлами.
void fibheap_free(FibHeap *heap)
{
    if (!heap)
        return;

    if (heap->min)
    {
        FibNode *stack[2048];
        int sp = 0;
        stack[sp++] = heap->min;

        FibNode *node = heap->min->right;
        while (node != heap->min)
        {
            stack[sp++] = node;
            node = node->right;
        }

        while (sp > 0)
        {
            FibNode *n = stack[--sp];
            if (n->child)
            {
                FibNode *child = n->child;
                FibNode *child_start = child;
                do
                {
                    stack[sp++] = child;
                    child = child->right;
                } while (child != child_start);
            }
            free(n);
        }
    }

    free(heap);
}
