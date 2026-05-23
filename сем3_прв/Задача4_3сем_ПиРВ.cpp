#include <iostream>
#include <vector>
#include <algorithm>
#include <chrono>
#include <random>
#include <clocale>
#include <windows.h>

using namespace std;
using namespace chrono;

class TaskTimer {
private:
    high_resolution_clock::time_point start_time;
    high_resolution_clock::time_point end_time;

public:
    void start() {
        start_time = high_resolution_clock::now();
    }

    void stop() {
        end_time = high_resolution_clock::now();
    }

    long long getDuration() {
        return duration_cast<milliseconds>(end_time - start_time).count();
    }

    void measureSorting(vector<int>& arr) {
        start();
        sort(arr.begin(), arr.end());
        stop();
        cout << "Время выполнения сортировки: " << getDuration() << " миллисекунд" << endl;
    }
};

int main() {
    setlocale(LC_ALL, "Russian");
    SetConsoleCP(1251);
    SetConsoleOutputCP(1251);

    const int SIZE_1 = 100000;
    const int SIZE_2 = 200000;
    TaskTimer timer;

    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<int> dist(1, 10000);

    vector<int> arr1(SIZE_1);
    for (int i = 0; i < SIZE_1; ++i) {
        arr1[i] = dist(gen);
    }
    timer.measureSorting(arr1);

    vector<int> arr2(SIZE_2);
    for (int i = 0; i < SIZE_2; ++i) {
        arr2[i] = dist(gen);
    }
    timer.measureSorting(arr2);

    return 0;
}