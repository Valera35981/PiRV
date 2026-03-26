#include <iostream>
#include <boost/thread.hpp>
#include <boost/thread/thread.hpp>
#include <queue>
#include <vector>
#include <windows.h>
using namespace std;

mutex console_mutex;
void safe_print(const string& message) {
    lock_guard<mutex> lock(console_mutex);
    cout << message << endl;
}
class Semaphore {
private:
    int count;
    boost::mutex mtx;
    boost::condition_variable cv;

public:
    Semaphore(int initial_count = 0) : count(initial_count) {}

    void acquire() {
        boost::unique_lock<boost::mutex> lock(mtx);
        while (count == 0) {
            cv.wait(lock);
        }
        count--;
    }

    void release() {
        boost::unique_lock<boost::mutex> lock(mtx);
        count++;
        cv.notify_one();
    }
};
struct Task {
    int id;
    string description;

    Task(int _id, const string& _desc) : id(_id), description(_desc) {}
};
class TaskQueue {
private:
    queue<Task> tasks;
    Semaphore semaphore;
    boost::mutex mtx;
    int max_size;
    bool finished;
public:
    TaskQueue(int _max_size) : max_size(_max_size), semaphore(0), finished(false) {} 
    void finish() {  
        finished = true;
        semaphore.release();  
    }
    void add_task(const Task& task) {
        boost::lock_guard<boost::mutex> lock(mtx);
        tasks.push(task);
        semaphore.release();  
        cout << "Задача " << task.id << " добавлена в очередь" << endl;
    }
    Task get_task() {
        semaphore.acquire();
        boost::lock_guard<boost::mutex> lock(mtx);
        if (tasks.empty() && finished) {
            return Task(-1, ""); 
        }

        Task task = tasks.front();
        tasks.pop();
        return task;
    }
    bool empty() {
        boost::lock_guard<boost::mutex> lock(mtx);
        return tasks.empty();
    }
};
void worker(int worker_id, TaskQueue& queue1, TaskQueue& queue2) {
    while (true) {
        Task task(0, "");
        bool got_task = false;

        if (!queue1.empty()) {
            task = queue1.get_task();
            if (task.id != -1) {
                got_task = true;
                lock_guard<mutex> lock(console_mutex);
                cout << "Рабочий " << worker_id << " взял задачу " << task.id << " из очереди 1" << endl;
            }
        }
        if (!got_task && !queue2.empty()) {
            task = queue2.get_task();
            if (task.id != -1) {
                got_task = true;
                lock_guard<mutex> lock(console_mutex);
                cout << "Рабочий " << worker_id << " взял задачу " << task.id << " из очереди 2" << endl;
            }
        }
        if (!got_task) {
            lock_guard<mutex> lock(console_mutex);
            cout << "Рабочий " << worker_id << " завершает работу" << endl;
            break;
        }
        {
            lock_guard<mutex> lock(console_mutex);
            cout << "Рабочий " << worker_id << " начал выполнение задачи " << task.id << endl;
        }
        boost::this_thread::sleep_for(boost::chrono::seconds(2));
        {
            lock_guard<mutex> lock(console_mutex);
            cout << "Рабочий " << worker_id << " завершил выполнение задачи " << task.id << endl;
        }
    }
}

int main() {
    SetConsoleCP(1251);
    SetConsoleOutputCP(1251);
    cout << "Задача 2: " << endl;
    cout << "Всего рабочих: 5" << endl;
    cout << "Всего очередей: 2" << endl;
    cout << endl;
    TaskQueue queue1(10);
    TaskQueue queue2(10);
    int tasks_count1, tasks_count2;
    cout << "Введите количество задач для очереди 1: ";
    cin >> tasks_count1;
    cout << "Введите количество задач для очереди 2: ";
    cin >> tasks_count2;
    cout << endl;
    cout << "Добавление задач в очередь 1:" << endl;
    for (int i = 1; i <= tasks_count1; i++) {
        queue1.add_task(Task(i, "Задача из очереди 1 №" + to_string(i)));
    }
    cout << endl;
    cout << "Добавление задач в очередь 2:" << endl;
    for (int i = tasks_count1 + 1; i <= tasks_count1 + tasks_count2; i++) {
        queue2.add_task(Task(i, "Задача из очереди 2 №" + to_string(i - tasks_count1)));
    }
    cout << endl;
    queue1.finish();  
    queue2.finish();
    vector<boost::thread> workers;
    for (int i = 1; i <= 5; i++) {
        workers.push_back(boost::thread(worker, i, ref(queue1), ref(queue2)));
    }
    for (auto& w : workers) {
        w.join();
    }

    cout << "ВСЕ ЗАДАЧИ ВЫПОЛНЕНЫ!" << endl;
    cout << "Все 5 рабочих завершили работу" << endl;

    return 0;
}