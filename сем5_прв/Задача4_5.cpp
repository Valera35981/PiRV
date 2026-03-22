#include <iostream>
#include <vector>
#include <mutex>
#include <thread>
#include <chrono>
#include <atomic>
#include <queue>
#include <condition_variable>
#include <string>

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

struct PrintJob {
    string document;
    int priority;
    int job_id;

    bool operator<(const PrintJob& other) const {
        return priority < other.priority;
    }
};

class PrinterQueue {
private:
    int n_printers;
    Semaphore semaphore;
    mutex mtx;
    priority_queue<PrintJob> job_queue;
    int next_job_id;

public:
    PrinterQueue(int printers) : n_printers(printers), semaphore(printers), next_job_id(0) {}

    void printJob(string doc, int priority, int timeout_ms) {
        thread::id thread_id = this_thread::get_id();
        int job_id;

        {
            lock_guard<mutex> lock(mtx);
            job_id = next_job_id++;
            PrintJob job{ doc, priority, job_id };
            job_queue.push(job);

            cout << "Thread: " << thread_id
                << " | Приоритет: " << priority
                << " | Задание " << job_id << " добавлено в очередь" << endl;
        }

        auto start_time = chrono::steady_clock::now();

        PrintJob current_job;
        bool got_printer = false;

        while (!got_printer) {
            auto now = chrono::steady_clock::now();
            auto elapsed = chrono::duration_cast<chrono::milliseconds>(now - start_time).count();

            if (elapsed >= timeout_ms) {
                {
                    lock_guard<mutex> lock(mtx);
                    priority_queue<PrintJob> temp;
                    while (!job_queue.empty()) {
                        if (job_queue.top().job_id != job_id) {
                            temp.push(job_queue.top());
                        }
                        job_queue.pop();
                    }
                    job_queue = temp;
                }

                cout << "Thread: " << thread_id
                    << " | Приоритет: " << priority
                    << " | Задание " << job_id << " ПРЕРВАНО (таймаут " << timeout_ms << "ms)" << endl;
                return;
            }

            if (semaphore.try_acquire_for(chrono::milliseconds(10))) {
                lock_guard<mutex> lock(mtx);

                if (!job_queue.empty() && job_queue.top().job_id == job_id) {
                    current_job = job_queue.top();
                    job_queue.pop();
                    got_printer = true;
                }
                else {
                    semaphore.release();
                }
            }

            this_thread::yield();
        }

        cout << "Thread: " << thread_id
            << " | Приоритет: " << current_job.priority
            << " | Задание " << current_job.job_id << " ПЕЧАТАЕТ: " << current_job.document << endl;

        this_thread::sleep_for(chrono::milliseconds(500 + rand() % 500));

        cout << "Thread: " << thread_id
            << " | Приоритет: " << current_job.priority
            << " | Задание " << current_job.job_id << " ЗАВЕРШЕНО" << endl;

        semaphore.release();
    }
};

void print_task(PrinterQueue& pq, string doc, int priority, int timeout, int id) {
    this_thread::yield();
    pq.printJob(doc, priority, timeout);
}

int main() {
    setlocale(LC_ALL, "ru_RU");
    srand((unsigned int)time(nullptr));

    PrinterQueue pq(2);

    cout << " Очередь печати. Принтеров: 2 " << endl << endl;

    vector<thread> threads;

    threads.emplace_back(print_task, ref(pq), "Документ_А", 5, 1000, 1);
    threads.emplace_back(print_task, ref(pq), "Документ_Б", 3, 1000, 2);
    threads.emplace_back(print_task, ref(pq), "Документ_В", 10, 1000, 3);
    threads.emplace_back(print_task, ref(pq), "Документ_Г", 1, 1000, 4);
    threads.emplace_back(print_task, ref(pq), "Документ_Д", 8, 1000, 5);
    threads.emplace_back(print_task, ref(pq), "Документ_Е", 7, 1000, 6);

    for (auto& t : threads) {
        t.detach();
    }

    this_thread::sleep_for(chrono::milliseconds(4000));

    cout << "\n Все задания обработаны " << endl;

    return 0;
}