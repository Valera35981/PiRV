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

    void acquire() {
        unique_lock<mutex> lock(mtx);
        cv.wait(lock, [this]() { return count > 0; });
        count--;
    }

    bool try_acquire_for(chrono::milliseconds timeout) {
        unique_lock<mutex> lock(mtx);
        if (cv.wait_for(lock, timeout, [this]() { return count > 0; })) {
            count--;
            return true;
        }
        return false;
    }

    void release() {
        unique_lock<mutex> lock(mtx);
        count++;
        cv.notify_one();
    }
};

template<typename T>
class ResourcePool {
private:
    vector<T> resources;
    Semaphore semaphore;
    mutex mtx;
    atomic<int> failed_attempts{ 0 };

    condition_variable cv;
    priority_queue<pair<int, int>> waiting_queue;
    vector<bool> resource_in_use;

public:
    ResourcePool(size_t count) : semaphore(count) {
        for (size_t i = 0; i < count; ++i) {
            resources.push_back(i);
            resource_in_use.push_back(false);
        }
    }

    T acquire(int priority, int timeout_ms) {
        thread::id thread_id = this_thread::get_id();
        int thread_hash = hash<thread::id>{}(thread_id);

        auto start_time = chrono::steady_clock::now();

        {
            lock_guard<mutex> lock(mtx);
            waiting_queue.push({ priority, thread_hash });
        }

        bool got_resource = false;
        T acquired_resource = -1;

        while (!got_resource) {
            auto now = chrono::steady_clock::now();
            auto elapsed = chrono::duration_cast<chrono::milliseconds>(now - start_time).count();

            if (elapsed >= timeout_ms) {
                {
                    lock_guard<mutex> lock(mtx);
                    failed_attempts++;

                    priority_queue<pair<int, int>> temp;
                    while (!waiting_queue.empty()) {
                        if (!(waiting_queue.top().second == thread_hash &&
                            waiting_queue.top().first == priority)) {
                            temp.push(waiting_queue.top());
                        }
                        waiting_queue.pop();
                    }
                    waiting_queue = temp;
                }

                cout << "Thread: " << thread_id
                    << " | Приоритет: " << priority
                    << " | Таймаут " << timeout_ms << "ms"
                    << " | НЕУДАЧА" << endl;
                return -1;
            }

            if (semaphore.try_acquire_for(chrono::milliseconds(10))) {
                lock_guard<mutex> lock(mtx);

                if (!waiting_queue.empty() &&
                    waiting_queue.top().first == priority &&
                    waiting_queue.top().second == thread_hash) {

                    for (size_t i = 0; i < resources.size(); ++i) {
                        if (!resource_in_use[i]) {
                            acquired_resource = resources[i];
                            resource_in_use[i] = true;
                            got_resource = true;
                            waiting_queue.pop();
                            break;
                        }
                    }

                    if (!got_resource) {
                        semaphore.release();
                    }
                }
                else {
                    semaphore.release();
                }
            }

            this_thread::yield();
        }

        cout << "Thread: " << thread_id
            << " | Приоритет: " << priority
            << " | ЗАХВАТИЛ ресурс " << acquired_resource << endl;

        return acquired_resource;
    }

    void release(T res) {
        thread::id thread_id = this_thread::get_id();

        {
            lock_guard<mutex> lock(mtx);
            for (size_t i = 0; i < resources.size(); ++i) {
                if (resources[i] == res && resource_in_use[i]) {
                    resource_in_use[i] = false;
                    break;
                }
            }
        }

        semaphore.release();

        cout << "Thread: " << thread_id
            << " | ОСВОБОДИЛ ресурс " << res << endl;

        cv.notify_all();
    }

    int get_failed_attempts() const {
        return failed_attempts.load();
    }

    void add_resource(T res) {
        lock_guard<mutex> lock(mtx);
        resources.push_back(res);
        resource_in_use.push_back(false);
        semaphore.release();
        cout << "Добавлен ресурс: " << res << endl;
    }

    bool remove_resource(T res) {
        lock_guard<mutex> lock(mtx);
        for (size_t i = 0; i < resources.size(); ++i) {
            if (resources[i] == res && !resource_in_use[i]) {
                resources.erase(resources.begin() + i);
                resource_in_use.erase(resource_in_use.begin() + i);
                cout << "Удален ресурс: " << res << endl;
                return true;
            }
        }
        return false;
    }
};

void worker(ResourcePool<int>& pool, int priority, int timeout, int id) {
    cout << "Thread: " << this_thread::get_id()
        << " | Приоритет: " << priority
        << " | СТАРТ" << endl;

    this_thread::yield();

    int res = pool.acquire(priority, timeout);

    if (res != -1) {
        this_thread::sleep_for(chrono::milliseconds(150));
        pool.release(res);
    }

    cout << "Thread: " << this_thread::get_id()
        << " | Приоритет: " << priority
        << " | ФИНИШ" << endl;
}

int main() {
    setlocale(LC_ALL, "ru_RU");

    ResourcePool<int> pool(2);

    vector<thread> threads;

    threads.emplace_back(worker, ref(pool), 10, 800, 1);
    threads.emplace_back(worker, ref(pool), 5, 800, 2);
    threads.emplace_back(worker, ref(pool), 8, 800, 3);
    threads.emplace_back(worker, ref(pool), 10, 800, 4);
    threads.emplace_back(worker, ref(pool), 3, 800, 5);

    for (auto& t : threads) {
        t.detach();
    }

    this_thread::sleep_for(chrono::milliseconds(2000));

    cout << "\n Динамическое добавление " << endl;
    pool.add_resource(10);

    this_thread::sleep_for(chrono::milliseconds(1500));

    cout << "\n Динамическое удаление " << endl;
    pool.remove_resource(0);

    this_thread::sleep_for(chrono::milliseconds(1500));

    cout << "\n Неудачных попыток: " << pool.get_failed_attempts() << endl;

    return 0;
}