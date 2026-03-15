#include <iostream>
#include <queue>
#include <thread>
#include <vector>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <random>
#include <sstream>

using namespace std;
#ifdef _WIN32
#include <windows.h>
#endif
string thread_id_to_string(const thread::id& id) {
    ostringstream oss;
    oss << id;
    return oss.str();
}

template<typename T>
struct PriorityItem {
    T value;
    int priority;

    bool operator<(const PriorityItem& other) const {
        return priority < other.priority;
    }
};

template<typename T>
class PriorityQueue {
private:
    priority_queue<PriorityItem<T>> queue;
    mutable mutex mtx;
    condition_variable cv;
    int total_pushed = 0;
    int total_popped = 0;

public:
    void push(T value, int priority) {
        {
            lock_guard<mutex> lock(mtx);

            PriorityItem<T> item{ value, priority };
            queue.push(item);
            total_pushed++;

            cout << "[Поток " << thread_id_to_string(this_thread::get_id())
                << "] Добавлен: " << value << " (приоритет: " << priority << ")"
                << " | В очереди: " << queue.size() << endl;
        }
        cv.notify_one();
    }

    T pop() {
        unique_lock<mutex> lock(mtx);
        while (queue.empty()) {
            cout << "[Поток " << thread_id_to_string(this_thread::get_id())
                << "] Ожидание элементов..." << endl;
            cv.wait(lock);
        }
        PriorityItem<T> item = queue.top();
        queue.pop();
        total_popped++;

        cout << "[Поток " << thread_id_to_string(this_thread::get_id())
            << "] Извлечен: " << item.value << " (приоритет: " << item.priority << ")"
            << " | В очереди: " << queue.size() << endl;

        return item.value;
    }
    bool try_pop(T& value) {
        lock_guard<mutex> lock(mtx);

        if (queue.empty()) {
            return false;
        }

        PriorityItem<T> item = queue.top();
        queue.pop();
        value = item.value;
        total_popped++;

        cout << "[Поток " << thread_id_to_string(this_thread::get_id())
            << "] Извлечен (без ожидания): " << item.value
            << " (приоритет: " << item.priority << ")" << endl;

        return true;
    }
    int size() {
        lock_guard<mutex> lock(mtx);
        return queue.size();
    }

    void print_stats() {
        lock_guard<mutex> lock(mtx);
        cout << "\n Статистика очереди " << endl;
        cout << "Добавлено: " << total_pushed << endl;
        cout << "Извлечено: " << total_popped << endl;
        cout << "Текущий размер: " << queue.size() << endl;
        cout << "\n" << endl;
    }
};

static void producer(PriorityQueue<string>& pq, int id, int count) {
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> priority_dist(1, 10);
    uniform_int_distribution<> value_dist(1, 100);

    for (int i = 0; i < count; i++) {
        string value = "Данные_" + to_string(value_dist(gen));
        int priority = priority_dist(gen);

        pq.push(value, priority);

        this_thread::sleep_for(chrono::milliseconds(rand() % 100 + 50));

        if (i % 3 == 0) {
            this_thread::yield();
        }
    }
}

static void consumer(PriorityQueue<string>& pq, int id, int count) {
    for (int i = 0; i < count; i++) {
        string value = pq.pop();

        this_thread::sleep_for(chrono::milliseconds(rand() % 150 + 100));

        if (i % 2 == 0) {
            this_thread::yield();
        }
    }
}

int main() {
#ifdef _WIN32
    SetConsoleCP(1251);
    SetConsoleOutputCP(1251);
#endif

    setlocale(LC_ALL, "Russian");

    cout << " Многопоточная очередь с приоритетом " << endl;

    PriorityQueue<string> pq;

    int num_producers, num_consumers, items_per_producer;

    cout << "Количество производителей: ";
    cin >> num_producers;

    cout << "Количество потребителей: ";
    cin >> num_consumers;

    cout << "Элементов на производителя: ";
    cin >> items_per_producer;
    int total_items = num_producers * items_per_producer;
    int items_per_consumer = total_items / num_consumers;

    cout << "\nВсего будет добавлено: " << total_items << " элементов" << endl;
    cout << "Каждый потребитель извлечет примерно по " << items_per_consumer << " элементов\n" << endl;

    vector<thread> threads;
    for (int i = 0; i < num_producers; i++) {
        threads.push_back(thread([&pq, i, items_per_producer]() {
            producer(pq, i, items_per_producer);
            }));
    }

    for (int i = 0; i < num_consumers; i++) {
        threads.push_back(thread([&pq, i, items_per_consumer]() {
            consumer(pq, i, items_per_consumer);
            }));
    }

    for (auto& t : threads) {
        t.join();
    }
    pq.print_stats();

    cout << "\nВсе потоки завершены!" << endl;

    return 0;
}