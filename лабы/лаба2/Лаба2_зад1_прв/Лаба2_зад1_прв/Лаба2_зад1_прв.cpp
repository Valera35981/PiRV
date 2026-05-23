#include <iostream>
#include <thread>
#include <vector>
#include <queue>
#include <mutex>
#include <semaphore>
#include <chrono>
#include <random>
#include <atomic>

using namespace std;
using namespace chrono;

struct Task {
    int id;
    int priority;
    int duration;
    bool isCritical;
};

struct TaskComparator {
    bool operator()(const Task& a, const Task& b) {
        if (a.isCritical != b.isCritical) {
            return b.isCritical;
        }
        return a.priority > b.priority;
    }
};

const int TOTAL_PROCESSORS = 4;
atomic<int> activeProcessors(TOTAL_PROCESSORS);
atomic<int> totalActiveTasks(0);
atomic<int> taskCounter(0);
atomic<bool> systemRunning(true);

counting_semaphore<TOTAL_PROCESSORS> processorSem(TOTAL_PROCESSORS);
mutex consoleMutex;
mutex queueMutex;
priority_queue<Task, vector<Task>, TaskComparator> taskQueue;

vector<thread> workerThreads;
mutex workersMutex;

random_device rd;
mt19937 gen(rd());
uniform_int_distribution<> durationDist(1, 5);
uniform_int_distribution<> priorityDist(1, 10);

void processTask(Task task) {
    processorSem.acquire();
    totalActiveTasks++;
    {
        lock_guard<mutex> lock(consoleMutex);
        cout << "[ПРОЦЕССОР] обрабатывает задачу #" << task.id
            << " | приоритет: " << task.priority
            << (task.isCritical ? " | КРИТИЧЕСКАЯ" : "")
            << " | время: " << task.duration << " сек" << endl;
    }
    this_thread::sleep_for(seconds(task.duration));
    if (totalActiveTasks.load() > activeProcessors.load() * 2) {
        lock_guard<mutex> lock(queueMutex);
        if (task.duration > 1) {
            Task subtask1 = { taskCounter++, task.priority, task.duration / 2, task.isCritical };
            Task subtask2 = { taskCounter++, task.priority, task.duration - task.duration / 2, task.isCritical };
            taskQueue.push(subtask1);
            taskQueue.push(subtask2);
            {
                lock_guard<mutex> lock(consoleMutex);
                cout << "[РАЗДЕЛЕНИЕ] Задача #" << task.id
                    << " разделена на #" << subtask1.id
                    << " и #" << subtask2.id << endl;
            }
        }
    }
    {
        lock_guard<mutex> lock(consoleMutex);
        cout << "[ГОТОВО] Задача #" << task.id << " завершена" << endl;
    }
    totalActiveTasks--;
    processorSem.release();
}

void distributeTasks() {
    while (systemRunning) {
        Task task;
        bool hasTask = false;
        {
            lock_guard<mutex> lock(queueMutex);
            if (!taskQueue.empty()) {
                task = taskQueue.top();
                taskQueue.pop();
                hasTask = true;
            }
        }
        if (hasTask) {
            lock_guard<mutex> lock(workersMutex);
            workerThreads.emplace_back(processTask, task);
        }
        else {
            this_thread::sleep_for(milliseconds(100));
        }
    }
}

void monitorSystem() {
    this_thread::sleep_for(seconds(5));
    if (systemRunning) {
        {
            lock_guard<mutex> lock(consoleMutex);
            cout << "\n!!! [АВАРИЯ] Квантовый процессор #2 вышел из строя !!!" << endl;
            cout << "[ПЕРЕРАСПРЕДЕЛЕНИЕ] Задачи переносятся на процессоры #1, #3, #4" << endl;
        }
        if (!processorSem.try_acquire()) {
            processorSem.acquire();
        }
        activeProcessors--;
        {
            lock_guard<mutex> lock(consoleMutex);
            cout << "Активных процессоров: " << activeProcessors.load() << endl;
            cout << "Задачи в очереди будут обработаны оставшимися процессорами\n" << endl;
        }
    }
    while (systemRunning) {
        this_thread::sleep_for(seconds(3));
        {
            lock_guard<mutex> lock(consoleMutex);
            cout << "\n[СТАТУС] Активных задач: " << totalActiveTasks.load()
                << " | В очереди: " << taskQueue.size()
                << " | Процессоров: " << activeProcessors.load() << endl;
        }
    }
}

void taskGenerator() {
    int taskId = 1;
    for (int i = 0; i < 10; i++) {
        bool isCritical = (i % 3 == 0);
        int priority = isCritical ? 1 : priorityDist(gen);
        int duration = durationDist(gen);
        Task task = { taskId++, priority, duration, isCritical };
        {
            lock_guard<mutex> lock(queueMutex);
            taskQueue.push(task);
        }
        {
            lock_guard<mutex> lock(consoleMutex);
            cout << "[ДОБАВЛЕНО] Задача #" << task.id
                << (isCritical ? " [КРИТИЧЕСКАЯ]" : "")
                << " | приоритет: " << priority
                << " | время: " << duration << " сек" << endl;
        }
        this_thread::sleep_for(milliseconds(500));
    }
    {
        lock_guard<mutex> lock(consoleMutex);
        cout << "\nВсе 10 задач добавлены в очередь\n" << endl;
    }
}

int main() {
    setlocale(LC_ALL, "Russian");
    cout << "КВАНТОВЫЙ СИМУЛЯТОР" << endl;
    cout << "Доступно процессоров: " << TOTAL_PROCESSORS << endl;
    cout << "Критические задачи имеют наивысший приоритет" << endl;
    cout << "При перегрузке задачи автоматически разделяются" << endl;
    cout << "Через 5 секунд произойдет отказ одного процессора\n" << endl;
    thread generator(taskGenerator);
    thread monitor(monitorSystem);
    vector<thread> distributors;
    for (int i = 0; i < 3; i++) {
        distributors.emplace_back(distributeTasks);
    }
    generator.join();
    while (totalActiveTasks.load() > 0 || !taskQueue.empty()) {
        this_thread::sleep_for(seconds(1));
        cout << "Ожидание завершения... Активных задач: " << totalActiveTasks.load()
            << ", в очереди: " << taskQueue.size() << endl;
    }
    systemRunning = false;
    for (auto& t : distributors) {
        if (t.joinable()) t.join();
    }
    if (monitor.joinable()) monitor.join();
    {
        lock_guard<mutex> lock(workersMutex);
        for (auto& t : workerThreads) {
            if (t.joinable()) t.join();
        }
    }
    cout << "\nВСЕ ЗАДАЧИ ВЫПОЛНЕНЫ!" << endl;
    cout << "Всего создано задач: " << taskCounter.load() << endl;
    return 0;
}