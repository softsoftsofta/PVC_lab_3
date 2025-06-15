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
const int DEFAULT_RUNS = 100;
const int DEFAULT_HW_THREADS = 1;

int* CreateArray(const int SIZE) {
    int* int_array = (int*) malloc(sizeof(int) * SIZE);
    for (int i = 0; i < SIZE; i++) {
        int_array[i] = rand()%100;
    }
    return int_array;
}

void PrintArray(const int* array, const int SIZE) {
    for (int i = 0; i < SIZE; i++) {
        printf("%d ",array[i]);
    }
    printf("\n");
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

void swap(int* arr, const int i, const int j) {
    int temp = arr[i];
    arr[i] = arr[j];
    arr[j] = temp;
}

void BubbleSort(int* arr, const int size) {
    for (int i = 0; i < size - 1; i++) {
        for (int j = 0; j < size - i - 1; j++) {
            if (arr[j] > arr[j + 1])
                swap(arr, j, j + 1);
        }
    }
}

void CheckSort(int* arr, const int size) {
    for (int i = 0; i < size-1; i++) {
        if (arr[i] > arr[i+1]) {
            printf("Массив отсортирован неправильно!\n");
            return;
        }
    }
    printf("Сортировка успешна \n");
}

void MergeSubArrays(int* arr, int l, int m, int r) {
    // Функция для слияния двух массивов
    // l - индекс начала первого массива
    // m - индекс конца первого массива
    // m+1 - индекс начала второго массива
    // r - индекс конца второго массива

    int i = l; 
    int j = m+1;
    int k = 0;

    int size = r-l+1;
    int* temp_arr = malloc(sizeof(int) * size);
    while (i <= m && j <= r && k < size) {
        if (arr[i] <= arr[j]) {
            temp_arr[k] = arr[i];
            i++;
        } else {
            temp_arr[k] = arr[j];
            j++;
        }
        k++;
    }

    while (i <= m && k < size) { // Дозаполнение
        temp_arr[k] = arr[i];
        i++;
        k++;
    }

    while (j <= r && k < size) { // Дозаполнение
        temp_arr[k] = arr[j];
        j++;
        k++;
    }

    k = 0;
    for (int v = l; v < l+size; v++) {
        arr[v] = temp_arr[k];
        k++;
    }

    free(temp_arr);
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

    // Цикл выполнения задачи и подсчёта времени её выполнения
    for (int i = 0; i < RUNS; i++) {

        int* int_array = NULL; // Заполнится и используется только главным процессом
        if (process_rank == 0) {
            int_array = CreateArray(ARRAY_SIZE);
            //PrintArray(int_array, ARRAY_SIZE);
        }
        
        clock_gettime(CLOCK_REALTIME, &begin); // Начало таймера

        int* buffer_array = NULL; // Заполнится в параллельном режиме
        // Распределение массива
        if (parallel_mode) {
            buffer_array = malloc(sizeof(int) * ARRAY_SIZE/HW_THREADS);
            //MPI_Barrier(MPI_COMM_WORLD); // Для синхронизации
            // Scatter обеспечивает, что все процессы получают данные
            // в порядке по возрастанию их ранга
            MPI_Scatter(int_array, 
                        ARRAY_SIZE/HW_THREADS, 
                        MPI_INT,
                        buffer_array,
                        ARRAY_SIZE/HW_THREADS, 
                        MPI_INT,
                        0, 
                        MPI_COMM_WORLD
            );
            // Сортировка подмассива пузырьком
            BubbleSort(buffer_array, ARRAY_SIZE/HW_THREADS);
            //printf("Сумма процесса %d: %ld \n", process_rank, sum_result);

            //MPI_Barrier(MPI_COMM_WORLD); // Для синхронизации
            // Gather обеспечивает, что все процессы отправляют данные
            // в порядке по возрастанию их ранга
            MPI_Gather(buffer_array,
                ARRAY_SIZE/HW_THREADS,
                        MPI_INT,
                        int_array,
                        ARRAY_SIZE/HW_THREADS,
                        MPI_INT,
                        0,
                        MPI_COMM_WORLD
            );
            //PrintArray(int_array, ARRAY_SIZE);
            // Ленивый и неэффективный алгоритм соединения всех подмассивов
            // Работает только если массив можно поровну разделить на HW_THREADS частей 
            if (process_rank == 0) {
                for (int i = 1; i < HW_THREADS; i++) {
                    MergeSubArrays(int_array, 0, 
                                (ARRAY_SIZE/HW_THREADS)*i-1, (ARRAY_SIZE/HW_THREADS)*(i+1)-1);
                }
            }
            
            free(buffer_array);
        } else {
            // Сортировка в последовательном режиме
            BubbleSort(int_array, ARRAY_SIZE);
        }

        clock_gettime(CLOCK_REALTIME, &end);
        exec_time += (double)(end.tv_sec - begin.tv_sec) + (double)(end.tv_nsec - begin.tv_nsec)/1e9;

        // Проверка на корректность сортировки 
        if (process_rank == 0) {
            CheckSort(int_array, ARRAY_SIZE);
            //PrintArray(int_array, ARRAY_SIZE);
            free(int_array);
        }
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
