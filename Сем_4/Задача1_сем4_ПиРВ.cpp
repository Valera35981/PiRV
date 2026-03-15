#include <iostream>
#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <clocale>
#include <windows.h>
using namespace std;

template <typename T>
class ParallelSum {
private:
    vector<T> data;
    size_t n_threads;
    T total_sum;
    mutex mtx;
    condition_variable cv;
    atomic<int> threads_completed;
    inline T segment_sum(const vector<T>& vec, size_t start, size_t end) {
        T sum = T();
        for (size_t i = start; i < end; ++i) {
            sum += vec[i];
        }
        return sum;
    }

public:
    ParallelSum(const vector<T>& arr, size_t threads)
        : data(arr), n_threads(threads), total_sum(T()), threads_completed(0) {}

    T compute_sum() {
        size_t data_size = data.size();
        size_t block_size = data_size / n_threads;
        size_t remainder = data_size % n_threads;

        size_t start = 0;

        for (size_t i = 0; i < n_threads; ++i) {
            size_t end = start + block_size + (i < remainder ? 1 : 0);
            thread t([this, start, end, i]() {
                if (i % 2 == 0) {
                    this_thread::yield();
                }
                T local_sum = this->segment_sum(this->data, start, end);
                {
                    lock_guard<std::mutex> lock(mtx);
                    cout << "Поток " << this_thread::get_id()
                        << " обработал элементы [" << start << ", " << end << ")"
                        << ", сумма сегмента = " << local_sum << endl;
                }
                {
                    lock_guard<std::mutex> lock(mtx);
                    total_sum += local_sum;
                }
                threads_completed++;
                if (threads_completed == n_threads) {
                    cv.notify_one(); 
                }
                });
            t.detach();

            start = end;
        }
        unique_lock<std::mutex> lock(mtx);
        cv.wait(lock, [this]() { return threads_completed == n_threads; });
        cout << "\nИтоговая сумма всех элементов: " << total_sum << endl;

        return total_sum;
    }
};

int main() {
    setlocale(LC_ALL, "Russian");
    SetConsoleOutputCP(1251);
    SetConsoleCP(1251);
    cout << "\n Тест с int " << endl;
    vector<int> int_data = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 };
    ParallelSum<int> int_sum(int_data, 4);
    int int_result = int_sum.compute_sum();

    cout << "\n Тест с double " << endl;
    vector<double> double_data = { 1.5, 2.5, 3.5, 4.5, 5.5, 6.5, 7.5, 8.5 };
    ParallelSum<double> double_sum(double_data, 3);
    double double_result = double_sum.compute_sum();

    return 0;
}