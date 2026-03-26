#include <iostream>
#include <vector>
#include <boost/thread.hpp>
#include <chrono>
#include <random>
#include <mutex>
#include <windows.h>
using namespace std;
using namespace std::chrono;

const int ROWS = 1000;
const int COLS = 1000;
const int NUM_THREADS = 4;
long long total_sum = 0;
mutex sum_mutex;
void compute_partial_sum(const vector<vector<int>>& matrix, int start_row, int end_row) {
    long long partial_sum = 0;
    for (int i = start_row; i < end_row; ++i) {
        for (int j = 0; j < COLS; ++j) {
            partial_sum += matrix[i][j];
        }
    }
    lock_guard<mutex> lock(sum_mutex);
    total_sum += partial_sum;
}

int main() {
    SetConsoleCP(1251);
    SetConsoleOutputCP(1251);
    cout << "Программа для вычисления суммы матрицы:" << endl;
    cout << "Размер матрицы: " << ROWS << "x" << COLS << endl;
    cout << "Количество потоков: " << NUM_THREADS << endl;
    cout << endl;
    vector<vector<int>> matrix(ROWS, vector<int>(COLS));
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<int> dist(1, 100);
    cout << "Заполнение матрицы случайными числами..." << endl;
    for (int i = 0; i < ROWS; ++i) {
        for (int j = 0; j < COLS; ++j) {
            matrix[i][j] = dist(gen);
        }
    }
    cout << "Матрица заполнена." << endl;
    cout << endl;
    auto start_time = high_resolution_clock::now();
    int rows_per_thread = ROWS / NUM_THREADS;
    int remaining_rows = ROWS % NUM_THREADS;
    vector<boost::thread> threads;
    int current_row = 0;
    for (int i = 0; i < NUM_THREADS; ++i) {
        int start_row = current_row;
        int end_row = current_row + rows_per_thread;
        if (i < remaining_rows) {
            end_row++;
        }
        cout << "Создан поток " << i << ": обрабатывает строки с " << start_row << " по " << (end_row - 1) << endl;
        threads.push_back(boost::thread(compute_partial_sum, ref(matrix), start_row, end_row));
        current_row = end_row;
    }
    cout << endl;
    for (auto& t : threads) {
        t.join();
    }
    auto end_time = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(end_time - start_time);
    cout << endl;
    cout << "РЕЗУЛЬТАТЫ: " << endl;
    cout << "Итоговая сумма всех элементов матрицы: " << total_sum << endl;
    cout << "Общее время выполнения: " << duration.count() << " мс" << endl;

    return 0;
}