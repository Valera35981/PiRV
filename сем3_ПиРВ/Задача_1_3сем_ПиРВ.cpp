#include <iostream>
#include <chrono>
#include <clocale>
#include <windows.h>

using namespace std;
long long sumFrom1ToN(int N) {
    long long sum = 0;
    for (int i = 1; i <= N; ++i) {
        sum += i;
    }
    return sum;
}

int main() {
    setlocale(LC_ALL, "Russian");
    SetConsoleCP(1251);
    SetConsoleOutputCP(1251);

    int N;
    cout << "Введите число N: ";
    cin >> N;
    auto start = chrono::high_resolution_clock::now();
    long long result = sumFrom1ToN(N);
    auto end = chrono::high_resolution_clock::now();
    auto duration = chrono::duration_cast<chrono::milliseconds>(end - start);

    cout << "Сумма чисел от 1 до " << N << " = " << result << endl;
    cout << "Время выполнения: " << duration.count() << " миллисекунды" << endl;

    return 0;
}