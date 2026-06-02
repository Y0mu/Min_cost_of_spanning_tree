#include "dmst.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <windows.h>

// Возвращает текущее время в секундах с высокой точностью.
static double get_time_seconds(void) {
    LARGE_INTEGER counter, frequency;
    QueryPerformanceFrequency(&frequency);
    QueryPerformanceCounter(&counter);
    return (double)counter.QuadPart / (double)frequency.QuadPart;
}

// Настраивает консоль Windows для корректного вывода UTF-8.
static void set_utf8_console(void) {
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
}

// Ожидает нажатия клавиши перед завершением программы.
static void wait_for_exit(void) {
    printf("Нажмите любую клавишу, чтобы выйти...\n");
    fflush(stdout);
    system("pause > nul");
}

// Считывает граф из файла с форматом: n m root, затем m строк u v w.
static int read_graph_from_file(const char *filename, int *n, int *m, int *root, Edge **edges) {
    FILE *input = fopen(filename, "r");
    if (!input) {
        perror("Не удалось открыть файл ввода");
        return 0;
    }

    if (fscanf(input, "%d %d %d", n, m, root) != 3) {
        fprintf(stderr, "Формат: <n> <m> <root> затем m строк u v w\n");
        fclose(input);
        return 0;
    }
    if (*n <= 0 || *m < 0) {
        fprintf(stderr, "Неверный размер графа\n");
        fclose(input);
        return 0;
    }

    *edges = (Edge *)malloc(sizeof(Edge) * (*m));
    if (!*edges) {
        fprintf(stderr, "Не удалось выделить память для рёбер\n");
        fclose(input);
        return 0;
    }

    for (int i = 0; i < *m; ++i) {
        int u, v;
        long long w;
        if (fscanf(input, "%d %d %lld", &u, &v, &w) != 3) {
            fprintf(stderr, "Ожидалось %d рёбер, но ввод закончился раньше\n", *m);
            free(*edges);
            fclose(input);
            return 0;
        }
        (*edges)[i].u = u;
        (*edges)[i].v = v;
        (*edges)[i].w = w;
        (*edges)[i].id = i;
    }

    fclose(input);
    return 1;
}

// Выводит рёбра графа в консоль для отладки и визуальной проверки.
static void print_graph_edges(int m, const Edge *edges) {
    printf("Список рёбер:\n");
    for (int i = 0; i < m; ++i) {
        printf(" %d: %d -> %d, вес=%lld\n", i, edges[i].u, edges[i].v, edges[i].w);
    }
}

// Генерирует случайный ориентированный граф с заданными параметрами.
static int generate_random_graph(int *n, int *m, int *root, Edge **edges) {
    const int min_n = 3;
    const int max_n = 10;
    const int min_weight = 1;
    const int max_weight = 20;

    srand((unsigned int)time(NULL));
    *n = min_n + rand() % (max_n - min_n + 1);
    int max_m = (*n) * (*n - 1);
    int min_m = *n - 1;
    *m = min_m + rand() % (max_m - min_m + 1);
    *root = rand() % *n;

    // Выделяем память под случайно сгенерированные рёбра графа.
    *edges = (Edge *)malloc(sizeof(Edge) * (*m));
    if (!*edges) {
        fprintf(stderr, "Не удалось выделить память для рёбер\n");
        return 0;
    }

    int edge_index = 0;
    // Формируем базовый остов, чтобы каждая вершина, кроме корня, была достижима.
    for (int v = 0; v < *n; ++v) {
        if (v == *root) {
            continue;
        }
        int u = rand() % *n;
        while (u == v) {
            u = rand() % *n;
        }
        (*edges)[edge_index].u = u;
        (*edges)[edge_index].v = v;
        (*edges)[edge_index].w = min_weight + rand() % (max_weight - min_weight + 1);
        (*edges)[edge_index].id = edge_index;
        edge_index++;
    }

    // Добавляем дополнительные случайные рёбра, чтобы заполнить граф.
    while (edge_index < *m) {
        int u = rand() % *n;
        int v = rand() % *n;
        if (u == v) {
            continue;
        }
        (*edges)[edge_index].u = u;
        (*edges)[edge_index].v = v;
        (*edges)[edge_index].w = min_weight + rand() % (max_weight - min_weight + 1);
        (*edges)[edge_index].id = edge_index;
        edge_index++;
    }

    printf("Сгенерирован случайный граф: n=%d, m=%d, root=%d\n", *n, *m, *root);
    return 1;
}

int main(int argc, char *argv[]) {
    set_utf8_console();

    // Инициализация параметров графа и режимов запуска.
    const char *filename = NULL;
    int n = 0, m = 0, root = 0;
    Edge *edges = NULL;
    int random_graph = 0;

    if (argc == 1) {
        // Если аргументы не переданы, предлагаем пользователю выбор источника данных.
        printf("Выберите источник графа:\n");
        printf(" 1) Использовать файл sample.txt (по умолчанию)\n");
        printf(" 2) Сгенерировать случайный граф\n");
        printf(" 3) Выбрать другой файл\n");
        printf("Введите 1, 2 или 3 и нажмите Enter [1]: ");

        char buffer[64];
        int choice = 1;
        if (fgets(buffer, sizeof(buffer), stdin) && buffer[0] != '\n') {
            choice = atoi(buffer);
        }

        if (choice == 2) {
            // Пользователь выбрал генерацию случайного графа.
            random_graph = 1;
            printf("Генерируется случайный граф с случайными n, m и root...\n");
            if (!generate_random_graph(&n, &m, &root, &edges)) {
                return EXIT_FAILURE;
            }
        } else if (choice == 3) {
            printf("Введите имя файла для входных данных: ");
            if (!fgets(buffer, sizeof(buffer), stdin)) {
                fprintf(stderr, "Ошибка ввода\n");
                return EXIT_FAILURE;
            }
            buffer[strcspn(buffer, "\r\n")] = '\0';
            if (!read_graph_from_file(buffer, &n, &m, &root, &edges)) {
                return EXIT_FAILURE;
            }
        } else {
            // По умолчанию загружаем образец из sample.txt.
            filename = "sample.txt";
            if (!read_graph_from_file(filename, &n, &m, &root, &edges)) {
                return EXIT_FAILURE;
            }
        }
    } else if (argc == 2) {
        // Если аргумент указан, выбираем режим по ключу или читаем из файла.
        if (strcmp(argv[1], "-r") == 0 || strcmp(argv[1], "--random") == 0) {
            random_graph = 1;
            if (!generate_random_graph(&n, &m, &root, &edges)) {
                return EXIT_FAILURE;
            }
        } else if (strcmp(argv[1], "--sample") == 0) {
            filename = "sample.txt";
            if (!read_graph_from_file(filename, &n, &m, &root, &edges)) {
                return EXIT_FAILURE;
            }
        } else {
            filename = argv[1];
            if (!read_graph_from_file(filename, &n, &m, &root, &edges)) {
                return EXIT_FAILURE;
            }
        }
    } else {
        fprintf(stderr, "Использование: %s [<файл-ввода> | -r | --random | --sample]\n", argc > 0 ? argv[0] : "dmst");
        return EXIT_FAILURE;
    }

    // Проверяем, что граф успешно загружен.
    if (edges == NULL) {
        fprintf(stderr, "Внутренняя ошибка: граф не загружен\n");
        return EXIT_FAILURE;
    }

    if (random_graph) {
        // Если граф был сгенерирован, выводим его ребра на экран.
        print_graph_edges(m, edges);
    }

    if (root < 0 || root >= n) {
        fprintf(stderr, "Корень должен быть в диапазоне от 0 до n-1\n");
        free(edges);
        return EXIT_FAILURE;
    }

    // Проверяем, что все вершины достижимы из корня.
    if (!check_reachable(n, root, m, edges)) {
        fprintf(stderr, "Граф недостижим из корня %d\n", root);
        free(edges);
        return EXIT_FAILURE;
    }

    printf("Граф: n=%d, m=%d, root=%d\n", n, m, root);

    long long cost;
    double start, end;

    // Запускаем и замеряем три варианта алгоритма поиска минимального остовного дерева.
    start = get_time_seconds();
    cost = dmst_edmonds_naive(n, root, m, edges);
    end = get_time_seconds();
    printf("[Наивный O(nm)] стоимость=%lld, время=%.6f сек\n", cost, end - start);

    start = get_time_seconds();
    cost = dmst_tarjan_heap(n, root, m, edges);
    end = get_time_seconds();
    printf("[Куча O(m log n)] стоимость=%lld, время=%.6f сек\n", cost, end - start);

    start = get_time_seconds();
    cost = dmst_tarjan_fib(n, root, m, edges);
    end = get_time_seconds();
    printf("[Фибоначчиева куча O(m + n log n)] стоимость=%lld, время=%.6f сек\n", cost, end - start);

    free(edges);
    wait_for_exit();
    return EXIT_SUCCESS;
}
