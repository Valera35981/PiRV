#include <iostream>
#include <vector>
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

template<typename T>
class SemaphoreBuffer {
private:
    vector<vector<T>> buffers;
    vector<Semaphore*> empty;
    vector<Semaphore*> full;
    vector<mutex*> mtx;

public:
    SemaphoreBuffer(int num_buffers, int buffer_size) : buffers(num_buffers) {
        for (int i = 0; i < num_buffers; i++) {
            buffers[i].reserve(buffer_size);
            empty.push_back(new Semaphore(buffer_size));
            full.push_back(new Semaphore(0));
            mtx.push_back(new mutex());
        }
    }

    ~SemaphoreBuffer() {
        for (size_t i = 0; i < empty.size(); i++) {
            delete empty[i];
            delete full[i];
            delete mtx[i];
        }
    }

    void produce(T value, int buffer_index, int timeout_ms) {
        thread::id thread_id = this_thread::get_id();
        auto start_time = chrono::steady_clock::now();

        if (empty[buffer_index]->try_acquire_for(chrono::milliseconds(timeout_ms))) {
            {
                lock_guard<mutex> lock(*mtx[buffer_index]);
                buffers[buffer_index].push_back(value);
            }
            full[buffer_index]->release();

            auto elapsed = chrono::duration_cast<chrono::milliseconds>(chrono::steady_clock::now() - start_time).count();
            cout << "Thread: " << thread_id
                << " | Буфер " << buffer_index
                << " | ПРОИЗВЕЛ: " << value
                << " | Время ожидания: " << elapsed << "ms" << endl;
        }
        else {
            auto elapsed = chrono::duration_cast<chrono::milliseconds>(chrono::steady_clock::now() - start_time).count();
            cout << "Thread: " << thread_id
                << " | Буфер " << buffer_index
                << " | ТАЙМАУТ " << timeout_ms << "ms"
                << " | Не удалось произвести" << endl;
        }
    }

    T consume(int buffer_index, int timeout_ms) {
        thread::id thread_id = this_thread::get_id();
        auto start_time = chrono::steady_clock::now();

        if (full[buffer_index]->try_acquire_for(chrono::milliseconds(timeout_ms))) {
            T value;
            {
                lock_guard<mutex> lock(*mtx[buffer_index]);
                value = buffers[buffer_index].back();
                buffers[buffer_index].pop_back();
            }
            empty[buffer_index]->release();

            auto elapsed = chrono::duration_cast<chrono::milliseconds>(chrono::steady_clock::now() - start_time).count();
            cout << "Thread: " << thread_id
                << " | Буфер " << buffer_index
                << " | ПОТРЕБИЛ: " << value
                << " | Время ожидания: " << elapsed << "ms" << endl;
            return value;
        }
        else {
            auto elapsed = chrono::duration_cast<chrono::milliseconds>(chrono::steady_clock::now() - start_time).count();
            cout << "Thread: " << thread_id
                << " | Буфер " << buffer_index
                << " | ТАЙМАУТ " << timeout_ms << "ms"
                << " | Не удалось потребить" << endl;
            return T();
        }
    }

    int get_buffer_size(int buffer_index) {
        lock_guard<mutex> lock(*mtx[buffer_index]);
        return buffers[buffer_index].size();
    }
};

void producer(SemaphoreBuffer<int>& buffer, int id, int num_productions) {
    for (int i = 0; i < num_productions; i++) {
        int buffer_index = rand() % 3;
        int value = id * 100 + i;
        int timeout = 500 + rand() % 500;

        buffer.produce(value, buffer_index, timeout);
        this_thread::yield();
        this_thread::sleep_for(chrono::milliseconds(100 + rand() % 200));
    }
}

void consumer(SemaphoreBuffer<int>& buffer, int id, int num_consumptions) {
    for (int i = 0; i < num_consumptions; i++) {
        int buffer_index = rand() % 3;
        int timeout = 500 + rand() % 500;

        buffer.consume(buffer_index, timeout);
        this_thread::yield();
        this_thread::sleep_for(chrono::milliseconds(150 + rand() % 250));
    }
}

int main() {
    setlocale(LC_ALL, "ru_RU");
    srand((unsigned int)time(nullptr));

    SemaphoreBuffer<int> buffer(3, 5);

    cout << " Система с 3 буферами (емкость каждого: 5) " << endl;
    cout << " 3 производителя и 3 потребителя " << endl << endl;

    vector<thread> threads;

    for (int i = 0; i < 3; i++) {
        threads.emplace_back(producer, ref(buffer), i + 1, 4);
    }

    for (int i = 0; i < 3; i++) {
        threads.emplace_back(consumer, ref(buffer), i + 1, 4);
    }

    for (auto& t : threads) {
        t.detach();
    }

    this_thread::sleep_for(chrono::milliseconds(5000));

    cout << "\n Статистика по буферам " << endl;
    for (int i = 0; i < 3; i++) {
        cout << "Буфер " << i << " содержит " << buffer.get_buffer_size(i) << " элементов" << endl;
    }

    cout << "\n Завершение работы " << endl;

    return 0;
}