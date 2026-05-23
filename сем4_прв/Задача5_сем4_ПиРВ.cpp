#include <iostream>
#include <map>
#include <thread>
#include <vector>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <random>
#include <string>
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

template<typename Key, typename Value>
class Cache {
private:
    map<Key, Value> data;
    mutable mutex mtx;
    condition_variable cv;

public:
    inline void set(const Key& key, const Value& value) {
        {
            lock_guard<mutex> lock(mtx);
            data[key] = value;
            cout << "[Поток " << thread_id_to_string(this_thread::get_id()) << "] Установлено: " << key << " = " << value << endl;
        }
        cv.notify_all();
    }

    inline Value get(const Key& key) {
        unique_lock<mutex> lock(mtx);

        while (data.find(key) == data.end()) {
            cout << "[Поток " << thread_id_to_string(this_thread::get_id()) << "] Ожидание ключа: " << key << endl;
            cv.wait(lock);
        }

        Value val = data[key];
        cout << "[Поток " << thread_id_to_string(this_thread::get_id()) << "] Получено: " << key << " = " << val << endl;
        return val;
    }
    void print_all() {
        lock_guard<mutex> lock(mtx);
        cout << "\n Содержимое кэша " << endl;
        for (auto it = data.begin(); it != data.end(); ++it) {
            cout << "  " << it->first << " : " << it->second << endl;
        }
        cout << "\n" << endl;
    }
};

static void writer_thread(Cache<string, int>& cache, int id, int count) {
    for (int i = 0; i < count; i++) {
        string key = "key" + to_string(i % 3);
        int value = rand() % 100;
        cache.set(key, value);
        this_thread::sleep_for(chrono::milliseconds(100));
    }
}

static void reader_thread(Cache<string, int>& cache, int id, int count) {
    for (int i = 0; i < count; i++) {
        string key = "key" + to_string(rand() % 3);
        cache.get(key);
        this_thread::sleep_for(chrono::milliseconds(150));
    }
}

int main() {
#ifdef _WIN32
    SetConsoleCP(1251);
    SetConsoleOutputCP(1251);
#endif

    setlocale(LC_ALL, "Russian");

    cout << "Потокобезопасный кэш" << endl;

    Cache<string, int> cache;

    int num_writers, num_readers, ops_per_thread;

    cout << "Количество потоков-писателей: ";
    cin >> num_writers;

    cout << "Количество потоков-читателей: ";
    cin >> num_readers;

    cout << "Операций на поток: ";
    cin >> ops_per_thread;

    vector<thread> threads;

    for (int i = 0; i < num_writers; i++) {
        threads.push_back(thread([&cache, i, ops_per_thread]() {
            writer_thread(cache, i, ops_per_thread);
            }));
    }

    for (int i = 0; i < num_readers; i++) {
        threads.push_back(thread([&cache, i, ops_per_thread]() {
            reader_thread(cache, i, ops_per_thread);
            }));
    }

    for (auto& t : threads) {
        t.join();
    }

    cache.print_all();

    cout << "Все потоки завершены" << endl;

    return 0;
}