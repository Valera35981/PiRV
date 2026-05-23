#include <iostream>
#include <vector>
#include <thread>
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
class MatrixProcessor {
private:
    vector<vector<T>> matrix;
    int rows;
    int cols;
    int num_threads;

    mutable mutex mtx;
    condition_variable cv;
    atomic<int> completed_threads{ 0 };

    int total_elements_processed = 0;
    atomic<int> active_threads{ 0 };

public:
    MatrixProcessor(int n, int m, int threads) : rows(n), cols(m), num_threads(threads) {
        matrix.resize(rows, vector<T>(cols));

        random_device rd;
        mt19937 gen(rd());
        uniform_int_distribution<T> dist(1, 100);

        for (int i = 0; i < rows; i++) {
            for (int j = 0; j < cols; j++) {
                matrix[i][j] = dist(gen);
            }
        }
    }

    void print_matrix(const string& title) {
        lock_guard<mutex> lock(mtx);
        cout << "\n" << title << ":\n";
        for (int i = 0; i < rows; i++) {
            for (int j = 0; j < cols; j++) {
                cout << matrix[i][j] << "\t";
            }
            cout << "\n";
        }
        cout << endl;
    }

    void apply(T(*func)(T)) {
        completed_threads = 0;
        total_elements_processed = 0;

        cout << "\n Запуск параллельной обработки " << endl;
        cout << "Матрица " << rows << "x" << cols << ", потоков: " << num_threads << endl;
        int total_elements = rows * cols;
        int elements_per_thread = total_elements / num_threads;
        int remainder = total_elements % num_threads;

        vector<thread> threads;

        int start_idx = 0;
        for (int t = 0; t < num_threads; t++) {
            int end_idx = start_idx + elements_per_thread + (t < remainder ? 1 : 0);

            threads.push_back(thread([this, func, start_idx, end_idx, t]() {
                {
                    lock_guard<mutex> lock(mtx);
                    cout << "[Поток " << thread_id_to_string(this_thread::get_id())
                        << "] начал обработку элементов с " << start_idx
                        << " по " << end_idx - 1 << endl;
                }

                active_threads++;

                int processed = 0;
                for (int idx = start_idx; idx < end_idx; idx++) {
                    int i = idx / cols;
                    int j = idx % cols;

                    matrix[i][j] = func(matrix[i][j]);

                    processed++;
                    if (idx % 3 == 0) {
                        this_thread::yield();
                    }
                }

                {
                    lock_guard<mutex> lock(mtx);
                    total_elements_processed += processed;
                    cout << "[Поток " << thread_id_to_string(this_thread::get_id())
                        << "] завершил работу. Обработано: " << processed << " элементов" << endl;
                }

                active_threads--;
                completed_threads++;

                if (completed_threads == num_threads) {
                    cv.notify_all();
                }
                }));

            start_idx = end_idx;
        }

        {
            unique_lock<mutex> lock(mtx);
            cv.wait(lock, [this]() { return completed_threads == num_threads; });
        }

        for (auto& t : threads) {
            if (t.joinable()) {
                t.join();
            }
        }

        cout << "\nОбработка завершена. Всего обработано: " << total_elements_processed << " элементов" << endl;
    }
    T get(int i, int j) const {
        lock_guard<mutex> lock(mtx);
        return matrix[i][j];
    }
};

int square(int x) {
    return x * x;
}

int double_value(int x) {
    return x * 2;
}

int add_random(int x) {
    static random_device rd;
    static mt19937 gen(rd());
    static uniform_int_distribution<> dist(1, 10);
    return x + dist(gen);
}

int main() {
#ifdef _WIN32
    SetConsoleCP(1251);
    SetConsoleOutputCP(1251);
#endif

    setlocale(LC_ALL, "Russian");

    cout << " Многопоточная обработка матриц " << endl;

    int rows, cols, threads;

    cout << "Введите количество строк: ";
    cin >> rows;

    cout << "Введите количество столбцов: ";
    cin >> cols;

    cout << "Введите количество потоков: ";
    cin >> threads;
    MatrixProcessor<int> processor(rows, cols, threads);
    processor.print_matrix("Исходная матрица");

    cout << "\nВыберите функцию для применения:\n";
    cout << "1 - возведение в квадрат\n";
    cout << "2 - удвоение\n";
    cout << "3 - добавление случайного числа\n";

    int choice;
    cin >> choice;

    switch (choice) {
    case 1:
        processor.apply(square);
        break;
    case 2:
        processor.apply(double_value);
        break;
    case 3:
        processor.apply(add_random);
        break;
    default:
        cout << "Неверный выбор!" << endl;
        return 1;
    }

    processor.print_matrix("Обработанная матрица");

    cout << "\nГотово!" << endl;

    return 0;
}