#include <iostream>
#include <mutex>
#include <thread>
#include <chrono>
#include <atomic>
#include <condition_variable>
#include <random>

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

    int get_count() {
        lock_guard<mutex> lock(mtx);
        return count;
    }
};

class ParkingLot {
private:
    int capacity;
    Semaphore semaphore;
    mutex mtx;
    atomic<int> occupied{ 0 };

public:
    ParkingLot(int cap) : capacity(cap), semaphore(cap) {}

    void park(bool isVIP, int timeout_ms) {
        thread::id thread_id = this_thread::get_id();
        auto start_time = chrono::steady_clock::now();

        if (isVIP) {
            {
                lock_guard<mutex> lock(mtx);
                if (occupied.load() < capacity) {
                    semaphore.acquire();
                    occupied++;

                    cout << "Thread: " << thread_id
                        << " | VIP | ЗАНЯЛ место"
                        << " | Занято: " << occupied.load()
                        << " | Свободно: " << (capacity - occupied.load()) << endl;
                    return;
                }
            }

            bool got_place = false;
            while (!got_place &&
                chrono::duration_cast<chrono::milliseconds>(
                    chrono::steady_clock::now() - start_time).count() < timeout_ms) {

                        {
                            lock_guard<mutex> lock(mtx);
                            if (occupied.load() < capacity && semaphore.try_acquire_for(chrono::milliseconds(10))) {
                                occupied++;
                                got_place = true;

                                cout << "Thread: " << thread_id
                                    << " | VIP | ЗАНЯЛ место (после ожидания)"
                                    << " | Занято: " << occupied.load()
                                    << " | Свободно: " << (capacity - occupied.load()) << endl;
                                break;
                            }
                        }
                        this_thread::yield();
            }

            if (!got_place) {
                cout << "Thread: " << thread_id
                    << " | VIP | ТАЙМАУТ " << timeout_ms << "ms"
                    << " | Не удалось припарковаться" << endl;
            }
        }
        else {
            bool got_place = semaphore.try_acquire_for(chrono::milliseconds(timeout_ms));

            if (got_place) {
                lock_guard<mutex> lock(mtx);
                occupied++;

                cout << "Thread: " << thread_id
                    << " | Обычный | ЗАНЯЛ место"
                    << " | Занято: " << occupied.load()
                    << " | Свободно: " << (capacity - occupied.load()) << endl;
            }
            else {
                lock_guard<mutex> lock(mtx);
                cout << "Thread: " << thread_id
                    << " | Обычный | ТАЙМАУТ " << timeout_ms << "ms"
                    << " | Занято: " << occupied.load()
                    << " | Свободно: " << (capacity - occupied.load()) << endl;
            }
        }
    }

    void leave() {
        thread::id thread_id = this_thread::get_id();

        semaphore.release();
        occupied--;

        cout << "Thread: " << thread_id
            << " | ОСВОБОДИЛ место"
            << " | Занято: " << occupied.load()
            << " | Свободно: " << (capacity - occupied.load()) << endl;
    }

    void set_capacity(int new_capacity) {
        lock_guard<mutex> lock(mtx);

        if (new_capacity > capacity) {
            int diff = new_capacity - capacity;
            for (int i = 0; i < diff; i++) {
                semaphore.release();
            }
        }

        capacity = new_capacity;
        cout << "Парковка: вместимость изменена на " << capacity
            << " | Занято: " << occupied.load()
            << " | Свободно: " << (capacity - occupied.load()) << endl;
    }

    int get_capacity() const {
        return capacity;
    }

    int get_occupied() const {
        return occupied.load();
    }
};

void car(ParkingLot& lot, bool isVIP, int timeout, int id) {
    cout << "Thread: " << this_thread::get_id()
        << " | " << (isVIP ? "VIP" : "Обычный")
        << " | Пытается припарковаться" << endl;

    this_thread::yield();

    lot.park(isVIP, timeout);

    if (isVIP || (lot.get_occupied() > 0)) {
        this_thread::sleep_for(chrono::milliseconds(200 + rand() % 300));
        lot.leave();
    }

    cout << "Thread: " << this_thread::get_id()
        << " | " << (isVIP ? "VIP" : "Обычный")
        << " | Завершает поездку" << endl;
}

int main() {
    setlocale(LC_ALL, "ru_RU");
    srand((unsigned int)time(nullptr));

    ParkingLot lot(3);

    cout << "=== Парковка открыта. Вместимость: " << lot.get_capacity() << " ===" << endl << endl;

    vector<thread> threads;

    threads.emplace_back(car, ref(lot), true, 1000, 1);
    threads.emplace_back(car, ref(lot), false, 800, 2);
    threads.emplace_back(car, ref(lot), false, 800, 3);
    threads.emplace_back(car, ref(lot), true, 1000, 4);
    threads.emplace_back(car, ref(lot), false, 800, 5);
    threads.emplace_back(car, ref(lot), false, 800, 6);
    threads.emplace_back(car, ref(lot), true, 1000, 7);

    for (auto& t : threads) {
        t.detach();
    }

    this_thread::sleep_for(chrono::milliseconds(1500));

    cout << "\n Динамическое увеличение вместимости до 5 " << endl;
    lot.set_capacity(5);

    this_thread::sleep_for(chrono::milliseconds(2000));

    cout << "\n Динамическое уменьшение вместимости до 2 " << endl;
    lot.set_capacity(2);

    this_thread::sleep_for(chrono::milliseconds(2000));

    cout << "\n Парковка закрывается " << endl;
    cout << "Финиш. Занято мест: " << lot.get_occupied() << endl;

    return 0;
}