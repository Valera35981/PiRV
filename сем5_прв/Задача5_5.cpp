#include <iostream>
#include <vector>
#include <mutex>
#include <thread>
#include <chrono>
#include <atomic>
#include <queue>
#include <condition_variable>

using namespace std;

class Semaphore {
private:
    mutex mtx;
    condition_variable cv;
    int count;

public:
    Semaphore(int initial) : count(initial) {}

    Semaphore(const Semaphore&) = delete;
    Semaphore& operator=(const Semaphore&) = delete;

    void acquire(int slots) {
        unique_lock<mutex> lock(mtx);
        cv.wait(lock, [this, slots]() { return count >= slots; });
        count -= slots;
    }

    bool try_acquire_for(int slots, chrono::milliseconds timeout) {
        unique_lock<mutex> lock(mtx);
        if (cv.wait_for(lock, timeout, [this, slots]() { return count >= slots; })) {
            count -= slots;
            return true;
        }
        return false;
    }

    void release(int slots) {
        unique_lock<mutex> lock(mtx);
        count += slots;
        cv.notify_all();
    }

    int get_count() {
        lock_guard<mutex> lock(mtx);
        return count;
    }
};

struct Task {
    int id;
    int required_slots;
    int duration_ms;
    int priority;

    bool operator<(const Task& other) const {
        return priority < other.priority;
    }
};

class TaskScheduler {
private:
    priority_queue<Task> task_queue;
    Semaphore resource_semaphore;
    mutex queue_mutex;
    atomic<int> completed_tasks{ 0 };
    int total_resources;

public:
    TaskScheduler(int total_res) : resource_semaphore(total_res), total_resources(total_res) {}

    void submit(Task task) {
        lock_guard<mutex> lock(queue_mutex);
        task_queue.push(task);

        cout << "Задача " << task.id
            << " добавлена в очередь (ресурсов: " << task.required_slots
            << ", приоритет: " << task.priority << ")" << endl;
    }

    void worker() {
        thread::id thread_id = this_thread::get_id();

        while (true) {
            Task current_task;
            bool has_task = false;

            {
                lock_guard<mutex> lock(queue_mutex);
                if (!task_queue.empty()) {
                    current_task = task_queue.top();
                    task_queue.pop();
                    has_task = true;
                }
            }

            if (!has_task) {
                this_thread::sleep_for(chrono::milliseconds(100));
                continue;
            }

            auto start_wait = chrono::steady_clock::now();

            if (resource_semaphore.try_acquire_for(current_task.required_slots, chrono::milliseconds(500))) {
                auto wait_time = chrono::duration_cast<chrono::milliseconds>(chrono::steady_clock::now() - start_wait).count();

                cout << "Thread: " << thread_id
                    << " | Задача " << current_task.id
                    << " | Захвачено ресурсов: " << current_task.required_slots
                    << " | Ожидание: " << wait_time << "ms" << endl;

                execute_task(current_task);

                resource_semaphore.release(current_task.required_slots);
                completed_tasks++;

                cout << "Thread: " << thread_id
                    << " | Задача " << current_task.id
                    << " | ВЫПОЛНЕНА | Освобождено ресурсов: " << current_task.required_slots << endl;
            }
            else {
                cout << "Thread: " << thread_id
                    << " | Задача " << current_task.id
                    << " | НЕ ВЫПОЛНЕНА (не хватило ресурсов)" << endl;

                lock_guard<mutex> lock(queue_mutex);
                task_queue.push(current_task);
            }

            this_thread::yield();
        }
    }

    inline void execute_task(Task& task) {
        this_thread::sleep_for(chrono::milliseconds(task.duration_ms));
    }

    int get_completed_tasks() const {
        return completed_tasks.load();
    }
};

void worker_thread(TaskScheduler& scheduler, int id) {
    scheduler.worker();
}

int main() {
    setlocale(LC_ALL, "ru_RU");

    TaskScheduler scheduler(5);

    cout << " Планировщик задач. Всего ресурсов: 5 " << endl << endl;

    vector<Task> tasks = {
        {1, 2, 300, 10},
        {2, 3, 400, 8},
        {3, 1, 200, 5},
        {4, 4, 500, 9},
        {5, 2, 350, 7},
        {6, 3, 450, 6},
        {7, 1, 250, 4},
        {8, 2, 300, 3}
    };

    for (auto& task : tasks) {
        scheduler.submit(task);
        this_thread::sleep_for(chrono::milliseconds(100));
    }

    cout << endl << " Запуск потоков-исполнителей " << endl;

    vector<thread> workers;
    for (int i = 0; i < 3; i++) {
        workers.emplace_back(worker_thread, ref(scheduler), i);
    }

    for (auto& w : workers) {
        w.detach();
    }

    this_thread::sleep_for(chrono::milliseconds(5000));

    cout << endl << " Статистика " << endl;
    cout << "Завершено задач: " << scheduler.get_completed_tasks() << endl;
    cout << "Всего задач: " << tasks.size() << endl;

    return 0;
}