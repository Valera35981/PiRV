%% writefile barrier.cpp
#include <iostream>
#include <iomanip>
#include <cstring>
#include <chrono>
#include <thread>
#include <random>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <semaphore.h>
#include <errno.h>

using namespace std;

const char* SHARED_MEM_NAME = "/my_barrier_shm";
const char* BARRIER_SEM_NAME = "/my_barrier_sem";
const size_t SHM_SIZE = 4096;
const int NUM_CHILDREN = 5;

struct SharedData {
    int results[NUM_CHILDREN];
    int ready_count;
    bool all_done;
};

int do_work(int id) {
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> dist(100, 500);

    int work_time = dist(gen);
    cout << "  [Дочерний " << id << "] начал работу (займёт " << work_time << " мс)" << endl;
    this_thread::sleep_for(chrono::milliseconds(work_time));

    int result = id * 100 + (work_time % 100);
    cout << "  [Дочерний " << id << "] завершил работу, результат = " << result << endl;
    return result;
}

int main() {
    cout << "=== Программа с барьером на семафорах (fork + разделяемая память) ===" << endl;
    cout << "Будет создано " << NUM_CHILDREN << " дочерних процессов." << endl;

    int shm_fd = shm_open(SHARED_MEM_NAME, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
    if (shm_fd == -1) {
        cerr << "Ошибка shm_open: " << strerror(errno) << endl;
        return 1;
    }

    if (ftruncate(shm_fd, SHM_SIZE) == -1) {
        cerr << "Ошибка ftruncate: " << strerror(errno) << endl;
        shm_unlink(SHARED_MEM_NAME);
        return 1;
    }

    SharedData* shared = (SharedData*)mmap(nullptr, SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (shared == MAP_FAILED) {
        cerr << "Ошибка mmap: " << strerror(errno) << endl;
        shm_unlink(SHARED_MEM_NAME);
        return 1;
    }

    memset(shared->results, 0, sizeof(shared->results));
    shared->ready_count = 0;
    shared->all_done = false;

    sem_t* barrier_sem = sem_open(BARRIER_SEM_NAME, O_CREAT, S_IRUSR | S_IWUSR, 0);
    if (barrier_sem == SEM_FAILED) {
        cerr << "Ошибка sem_open: " << strerror(errno) << endl;
        munmap(shared, SHM_SIZE);
        shm_unlink(SHARED_MEM_NAME);
        return 1;
    }

    cout << "\nСоздание дочерних процессов..." << endl;

    for (int i = 0; i < NUM_CHILDREN; ++i) {
        pid_t pid = fork();

        if (pid == -1) {
            cerr << "Ошибка fork для процесса " << i << ": " << strerror(errno) << endl;
            continue;
        }

        if (pid == 0) {
            cout << "[Дочерний " << i << "] PID = " << getpid() << " создан" << endl;
            int result = do_work(i);
            shared->results[i] = result;
            cout << "[Дочерний " << i << "] достиг барьера, отправляю сигнал" << endl;
            sem_post(barrier_sem);
            exit(0);
        }
    }

    cout << "\n[Родитель] Ожидаю, пока все дочерние процессы достигнут барьера..." << endl;

    for (int i = 0; i < NUM_CHILDREN; ++i) {
        sem_wait(barrier_sem);
        cout << "[Родитель] Получен сигнал от процесса " << i + 1 << "/" << NUM_CHILDREN << endl;
    }

    cout << "\n[Родитель] БАРЬЕР ПРОЙДЕН! Все " << NUM_CHILDREN << " процессов завершили работу." << endl;

    cout << "\n=== РЕЗУЛЬТАТЫ РАБОТЫ ДОЧЕРНИХ ПРОЦЕССОВ ===" << endl;
    int total = 0;
    for (int i = 0; i < NUM_CHILDREN; ++i) {
        cout << "  Процесс " << i << ": результат = " << shared->results[i] << endl;
        total += shared->results[i];
    }
    cout << "  Сумма всех результатов: " << total << endl;
    cout << "  Среднее арифметическое: " << fixed << setprecision(2) << (double)total / NUM_CHILDREN << endl;

    cout << "\n[Родитель] Ожидаю завершения всех дочерних процессов..." << endl;
    int status;
    for (int i = 0; i < NUM_CHILDREN; ++i) {
        pid_t finished_pid = wait(&status);
        cout << "  Процесс PID " << finished_pid << " завершён (код: " << WEXITSTATUS(status) << ")" << endl;
    }

    cout << "\nОсвобождение ресурсов..." << endl;
    munmap(shared, SHM_SIZE);
    close(shm_fd);
    shm_unlink(SHARED_MEM_NAME);
    sem_close(barrier_sem);
    sem_unlink(BARRIER_SEM_NAME);

    cout << "\nПрограмма успешно завершена!" << endl;
    return 0;
}