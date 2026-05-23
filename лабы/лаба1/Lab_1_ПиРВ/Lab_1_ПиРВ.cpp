#include <iostream>
#include <vector>
#include <algorithm>
#include <chrono>
#include <atomic>
#include <mutex>
#include <queue>
#include <boost/thread.hpp>
#include <windows.h>
#include <locale.h>

using namespace std;

void sortPart(vector<int>& arr, int start, int end) {
    sort(arr.begin() + start, arr.begin() + end);
}

void singleThreadSort(vector<int>& arr) {
    sort(arr.begin(), arr.end());
}

void multiThreadSortBoost(vector<int>& arr, int numThreads) {
    int size = (int)arr.size();
    int partSize = size / numThreads;
    boost::thread_group threads;

    for (int i = 0; i < numThreads; i++) {
        int start = i * partSize;
        int end = (i == numThreads - 1) ? size : start + partSize;
        threads.create_thread(boost::bind(sortPart, boost::ref(arr), start, end));
    }

    threads.join_all();
    sort(arr.begin(), arr.end());
}

long long unsafeCounter = 0;
void incrementUnsafe(int iterations) {
    for (int i = 0; i < iterations; i++) {
        unsafeCounter++;
    }
}

class AtomicQueue {
private:
    queue<int> q;
    atomic_flag lock = ATOMIC_FLAG_INIT;
public:    
    void push(int val) {
        while (lock.test_and_set(memory_order_acquire)) {
            boost::this_thread::yield();
        }
        q.push(val);
        lock.clear(memory_order_release);
    }

    bool pop(int& val) {
        while (lock.test_and_set(memory_order_acquire)) {
            boost::this_thread::yield();
        }
        if (q.empty()) {
            lock.clear(memory_order_release);
            return false;
        }
        val = q.front();
        q.pop();
        lock.clear(memory_order_release);
        return true;
    }

    int size() {
        while (lock.test_and_set(memory_order_acquire)) {
            boost::this_thread::yield();
        }
        int s = (int)q.size();
        lock.clear(memory_order_release);
        return s;
    }
};

class MutexQueue {
private:
    queue<int> q;
    mutex mtx;
public:
    void push(int val) {
        lock_guard<mutex> lock(mtx);
        q.push(val);
    }

    bool pop(int& val) {
        lock_guard<mutex> lock(mtx);
        if (q.empty()) return false;
        val = q.front();
        q.pop();
        return true;
    }

    int size() {
        lock_guard<mutex> lock(mtx);
        return (int)q.size();
    }
};

atomic<long long> totalPushed(0);
atomic<long long> totalPopped(0);
const int TASKS_PER_PRODUCER = 5000;

template<typename Q>
void producer(Q& queue, int id) {
    for (int i = 0; i < TASKS_PER_PRODUCER; i++) {
        queue.push(id * 10000 + i);
        totalPushed++;
    }
}

template<typename Q>
void consumer(Q& queue, int expectedTasks) {
    int val;
    int processed = 0;

    while (processed < expectedTasks) {
        if (queue.pop(val)) {
            processed++;
            totalPopped++;
        }
        else {
            boost::this_thread::sleep_for(boost::chrono::milliseconds(1));
        }
    }
}

template<typename Q>
void testQueue(const char* name, int producers, int consumers) {
    cout << "\n  " << name << ":\n";

    Q queue;
    totalPushed = 0;
    totalPopped = 0;

    int totalTasks = producers * TASKS_PER_PRODUCER;
    int tasksPerConsumer = totalTasks / consumers;

    boost::thread_group threads;
    auto start = chrono::high_resolution_clock::now();

    for (int i = 0; i < producers; i++) {
        threads.create_thread(boost::bind(producer<Q>, boost::ref(queue), i));
    }

    boost::this_thread::sleep_for(boost::chrono::milliseconds(10));

    for (int i = 0; i < consumers; i++) {
        threads.create_thread(boost::bind(consumer<Q>, boost::ref(queue), tasksPerConsumer));
    }

    threads.join_all();

    auto end = chrono::high_resolution_clock::now();
    auto ms = chrono::duration_cast<chrono::milliseconds>(end - start);

    cout << "    Время: " << ms.count() << " мс\n";
    cout << "    Добавлено: " << totalPushed << "\n";
    cout << "    Обработано: " << totalPopped << "\n";
    cout << "    " << (totalPushed == totalPopped ? " OK" : "ПРОБЛЕМА!") << "\n";
}

void testCounter(int threads, int iterations) {
    unsafeCounter = 0;
    boost::thread_group tg;

    auto start = chrono::high_resolution_clock::now();

    for (int i = 0; i < threads; i++) {
        tg.create_thread(boost::bind(incrementUnsafe, iterations));
    }

    tg.join_all();

    auto end = chrono::high_resolution_clock::now();
    auto ms = chrono::duration_cast<chrono::milliseconds>(end - start);

    long long expected = (long long)threads * iterations;
    cout << "\n  Счетчик (демонстрация проблемы):\n";
    cout << "    Время: " << ms.count() << " мс\n";
    cout << "    Результат: " << unsafeCounter << " (ожидалось " << expected << ")\n";
    cout << "    " << (unsafeCounter == expected ? "OK" : " ПРОБЛЕМА! Гонка данных!") << "\n";
}

int main() {
    SetConsoleOutputCP(1251);
    setlocale(LC_ALL, "Russian");

    cout << " ЛАБОРАТОРНАЯ РАБОТА №1\n";
    cout << "ЗАДАЧА 1: Параллельная сортировка массива\n";

    const int SIZE = 1000000;
    vector<int> arr(SIZE);

    cout << "Генерация массива...\n";
    for (int i = 0; i < SIZE; i++) {
        arr[i] = rand() % 1000000;
    }

    cout << "\nРезультаты:\n";

    {
        vector<int> test = arr;
        auto start = chrono::high_resolution_clock::now();
        singleThreadSort(test);
        auto end = chrono::high_resolution_clock::now();
        cout << "1 поток: " << chrono::duration_cast<chrono::milliseconds>(end - start).count() << " мс\n";
    }

    {
        vector<int> test = arr;
        auto start = chrono::high_resolution_clock::now();
        multiThreadSortBoost(test, 2);
        auto end = chrono::high_resolution_clock::now();
        cout << "2 потока: " << chrono::duration_cast<chrono::milliseconds>(end - start).count() << " мс\n";
    }

    {
        vector<int> test = arr;
        auto start = chrono::high_resolution_clock::now();
        multiThreadSortBoost(test, 4);
        auto end = chrono::high_resolution_clock::now();
        cout << "4 потока: " << chrono::duration_cast<chrono::milliseconds>(end - start).count() << " мс\n";
    }

    {
        vector<int> test = arr;
        auto start = chrono::high_resolution_clock::now();
        multiThreadSortBoost(test, 8);
        auto end = chrono::high_resolution_clock::now();
        cout << "8 потоков: " << chrono::duration_cast<chrono::milliseconds>(end - start).count() << " мс\n";
    }

    cout << "\n";
    cout << "ЗАДАЧА 2: Очередь задач и демонстрация проблемы \n";

    const int THREADS[] = { 2, 4, 8 };
    const int ITERATIONS = 50000;
    for (int threads : THREADS) {
        cout << "\n " << threads << " потоков \n";
     
        cout << "\n ДЕМОНСТРАЦИЯ ПРОБЛЕМЫ БЕЗ СИНХРОНИЗАЦИИ:\n";
        testCounter(threads, ITERATIONS);

        cout << "\n ОЧЕРЕДЬ С ATOMIC:\n";
        testQueue<AtomicQueue>("", threads, threads);

        cout << "\n ОЧЕРЕДЬ С MUTEX:\n";
        testQueue<MutexQueue>("", threads, threads);
    }
    cout << "\nГотово! Нажмите Enter для выхода ";
    cin.get();

    return 0;
}

