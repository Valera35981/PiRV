#include <iostream>
#include <thread>
#include <vector>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <random>
#include <queue>
#include <atomic>

using namespace std;

#ifdef _WIN32
#include <windows.h>
#endif

template<typename T>
class CircularBuffer {
private:
    vector<T> buffer;
    size_t capacity;
    size_t head = 0;  // индекс для чтения
    size_t tail = 0;  // индекс для записи
    size_t count = 0; // количество элементов

    mutable mutex mtx;
    condition_variable not_full;
    condition_variable not_empty;

    atomic<size_t> total_produced{ 0 };
    atomic<size_t> total_consumed{ 0 };

public:
    CircularBuffer(size_t cap) : capacity(cap) {
        buffer.resize(capacity);
    }

    void produce(T value) {
        {
            unique_lock<mutex> lock(mtx);

            // Ждем свободного места
            while (count >= capacity) {
                cout << "Производитель [" << this_thread::get_id()
                    << "] ждет: буфер полон" << endl;

                lock.unlock();
                this_thread::yield();
                lock.lock();

                not_full.wait(lock, [this] { return count < capacity; });
            }

            // Добавляем элемент в кольцевой буфер
            buffer[tail] = value;
            tail = (tail + 1) % capacity;
            count++;
            total_produced++;

            cout << "Производитель [" << this_thread::get_id()
                << "] + " << value << " [" << count << "/" << capacity << "]" << endl;
        }

        not_empty.notify_one();
    }

    T consume() {
        T value;

        {
            unique_lock<mutex> lock(mtx);

            // Ждем появления элементов
            while (count == 0) {
                cout << "Потребитель [" << this_thread::get_id()
                    << "] ждет: буфер пуст" << endl;

                lock.unlock();
                this_thread::yield();
                lock.lock();

                not_empty.wait(lock, [this] { return count > 0; });
            }

            // Извлекаем элемент из кольцевого буфера
            value = buffer[head];
            head = (head + 1) % capacity;
            count--;
            total_consumed++;

            cout << "Потребитель [" << this_thread::get_id()
                << "] - " << value << " [" << count << "/" << capacity << "]" << endl;
        }

        not_full.notify_one();
        return value;
    }

    size_t size() const {
        lock_guard<mutex> lock(mtx);
        return count;
    }

    void print_stats() const {
        lock_guard<mutex> lock(mtx);
        cout << "Статистика: произведено=" << total_produced
            << ", потреблено=" << total_consumed
            << ", в буфере=" << count << endl;
    }
};

int main() {
#ifdef _WIN32
    SetConsoleCP(1251);
    SetConsoleOutputCP(1251);
#endif

    setlocale(LC_ALL, "Russian");

    cout << "=== Производитель-Потребитель (кольцевой буфер) ===" << endl;

    int buffer_size, num_producers, num_consumers, items_per_producer;

    cout << "Размер буфера: ";
    cin >> buffer_size;

    cout << "Количество производителей: ";
    cin >> num_producers;

    cout << "Количество потребителей: ";
    cin >> num_consumers;

    cout << "Элементов на производителя: ";
    cin >> items_per_producer;

    CircularBuffer<int> buffer(buffer_size);

    vector<thread> producers;
    vector<thread> consumers;

    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> dist(1, 100);

    // Запуск производителей
    for (int i = 0; i < num_producers; ++i) {
        producers.emplace_back([&buffer, &gen, &dist, items_per_producer]() {
            for (int j = 0; j < items_per_producer; ++j) {
                buffer.produce(dist(gen));
                this_thread::sleep_for(chrono::milliseconds(rand() % 50 + 10));
            }
            });
    }

    // Запуск потребителей
    int total_items = num_producers * items_per_producer;
    int items_per_consumer = total_items / num_consumers;

    for (int i = 0; i < num_consumers; ++i) {
        consumers.emplace_back([&buffer, items_per_consumer]() {
            for (int j = 0; j < items_per_consumer; ++j) {
                buffer.consume();
                this_thread::sleep_for(chrono::milliseconds(rand() % 70 + 20));
            }
            });
    }

    // Ожидание завершения
    for (auto& p : producers) p.join();
    for (auto& c : consumers) c.join();

    cout << "\n=== Готово ===" << endl;
    buffer.print_stats();

    return 0;
}