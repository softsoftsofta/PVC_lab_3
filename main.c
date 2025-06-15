#include <limits.h>
#include <stdint.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>

const int DEFAULT_ARRAY_SIZE = 1000000;
const int DEFAULT_RUNS = 1000;
const int DEFAULT_HW_THREADS = 1;

int* CreateArray(const int SIZE) {
    int* int_array = (int*) malloc(sizeof(int) * SIZE);
    for (int i = 0; i < SIZE; i++) {
        int_array[i] = rand();
    }
    return int_array;
}

void PrintArray(const int* array, const int SIZE) {
    for (int i = 0; i < SIZE; i++) {
        printf("%d ",array[i]);
    }
}

int GetEnvArraySize() {
    char* array_size_char = getenv("ARRAY_SIZE");
    int array_size_int = DEFAULT_ARRAY_SIZE;
    if (array_size_char != NULL) {
        array_size_int = atoi(array_size_char);
    } else {
        printf(
            "Переменная среды ARRAY_SIZE не получена, "
            "используем значение по умолчанию: %d \n", DEFAULT_ARRAY_SIZE
        );
    }
    return array_size_int;
}

int GetEnvHwThreads() {
    char* hw_thread_char = getenv("HW_THREADS");
    int hw_thread_int = DEFAULT_HW_THREADS;
    if (hw_thread_char != NULL) {
        hw_thread_int = atoi(hw_thread_char);
    } else {
        printf(
            "Переменная среды HW_THREADS не получена, "
            "используем значение по умолчанию: %d \n", DEFAULT_HW_THREADS
        );
    }
    return hw_thread_int;
}

int GetEnvRuns() {
    char* runs_char = getenv("RUNS");
    int runs_int = DEFAULT_RUNS;
    if (runs_char != NULL) {
        runs_int = atoi(runs_char);
    } else {
        printf(
            "Переменная среды RUNS не получена, "
            "используем значение по умолчанию: %d \n", DEFAULT_RUNS
        );
    }
    return runs_int;
}

int64_t SumElementsOfArray(const int* array, const int SIZE) {
    int64_t result = 0;
    for (int i = 0; i < SIZE; i++) {
        result += array[0];
    }
    return result;
}

int64_t SumElementsOfArrayINT64(const int64_t* array, const int SIZE) {
    int64_t result = 0;
    for (int i = 0; i < SIZE; i++) {
        result += array[0];
    }
    
    return result;
}

int main(int argc, char** argv) {
    srand(time(0));
    //srand(1);
    const int ARRAY_SIZE = GetEnvArraySize();
    const int RUNS = GetEnvRuns();
    const int HW_THREADS = GetEnvHwThreads();
    bool parallel_mode = false; 

    if (HW_THREADS > 1) {
        parallel_mode = true;
        printf("Программа отработает в параллельном "
                "режиме (используемых аппаратных потоков > 1)\n"
        );
    } else {
        printf("Программа отработает в последовательном "
                "режиме (используемых аппаратных потоков = 1)\n"
        );
    }
    
    // Таймер
    struct timespec begin, end;
    double exec_time = 0.0;

    // Информация о процессе
    int process_rank = 0;
    int size_of_cluster = 1;

    // Инициализация MPI
    if (parallel_mode) {
        MPI_Init(&argc, &argv);
        MPI_Comm_size(MPI_COMM_WORLD, &size_of_cluster);
        MPI_Comm_rank(MPI_COMM_WORLD, &process_rank);
        printf("Процесс с рангом %d заработал \n", process_rank);
        MPI_Barrier(MPI_COMM_WORLD); // Для синхронизации
    }

    // Чтобы эту инфу писал только главный процесс
    if (process_rank == 0) {
        printf("Размер массива: %d \n", ARRAY_SIZE);
        printf("Повторений: %d \n", RUNS);
        printf("Используемые аппаратные потоки: %d \n", HW_THREADS);
        printf("Количество процессов: %d \n", size_of_cluster);
        printf("Рассчёты начаты...\n");
    }

    int* int_array = NULL; // Заполнится и используется только главным процессом
    if (process_rank == 0) {
        int_array = CreateArray(ARRAY_SIZE);
    }

    // Цикл выполнения задачи и подсчёта времени её выполнени
    for (int i = 0; i < RUNS; i++) {
        
        clock_gettime(CLOCK_REALTIME, &begin); // Начало таймера

        int* buffer_array = NULL; // Заполнится в параллельном режиме
        // Распределение массива
        if (parallel_mode) {
            buffer_array = malloc(sizeof(int) * ARRAY_SIZE/HW_THREADS);
            MPI_Barrier(MPI_COMM_WORLD); // Для синхронизации
            MPI_Scatter(int_array, 
                        ARRAY_SIZE/HW_THREADS, 
                        MPI_INT,
                        buffer_array,
                        ARRAY_SIZE/HW_THREADS, 
                        MPI_INT,
                        0, 
                        MPI_COMM_WORLD
            );
        }
        
        int64_t sum_result = 0;
        int64_t final_sum = 0;
        // Вычисление суммы
        if (parallel_mode) {
            // Cумма в параллельном режиме
            sum_result = SumElementsOfArray(buffer_array, ARRAY_SIZE/HW_THREADS);
            //printf("Сумма процесса %d: %ld \n", process_rank, sum_result);

            int64_t *all_sums = NULL; // Для сбора сумм со всех процессов
            if (process_rank == 0) {
                all_sums = malloc(sizeof(int64_t) * HW_THREADS);
            }
            MPI_Barrier(MPI_COMM_WORLD); // Для синхронизации
            // Сбор всех сумм в процессе 0
            MPI_Gather(&sum_result,
                        1,
                        MPI_INT64_T,
                        all_sums,
                        1,
                        MPI_INT64_T,
                        0,
                        MPI_COMM_WORLD
            );
            // Рассчёт финальной суммы
            if (process_rank == 0) {
                final_sum = SumElementsOfArrayINT64(all_sums, HW_THREADS);
                free(all_sums);
            }
        } else {
            // Сумма в последовательном режиме
            sum_result = SumElementsOfArray(int_array, ARRAY_SIZE);
        }
        //printf("Результат: %ld \n", sum_result);
        
        // Очистка памяти и вывод результатов
        if (parallel_mode) {
            free(buffer_array);
        }

        clock_gettime(CLOCK_REALTIME, &end);
        exec_time += (double)(end.tv_sec - begin.tv_sec) + (double)(end.tv_nsec - begin.tv_nsec)/1e9;
    }

    if (process_rank == 0) {
        double mean_exec_time = exec_time / RUNS;
        printf("Общее время выполнения: %f сек. \n", exec_time);
        printf("Среднее время выполнения: %f сек.", mean_exec_time);
    }

    if (parallel_mode) {
        MPI_Finalize();
    }
    return 0;
}
