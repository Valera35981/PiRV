#include <iostream>
#include <chrono>
#include <thread>
#include <clocale>
#include <windows.h>

using namespace std;

int main() {
    setlocale(LC_ALL, "Russian");
    SetConsoleCP(1251);
    SetConsoleOutputCP(1251);

    int N;
    cout << "Введите количество секунд: ";
    cin >> N;

    cout << "Таймер запущен!" << endl;
    for (int i = N; i > 0; --i) {
        cout << "Осталось: " << i << " ";
        if (i % 10 == 1 && i % 100 != 11) {
            cout << "секунда";
        }
        else if ((i % 10 >= 2 && i % 10 <= 4) && (i % 100 < 10 || i % 100 >= 20)) {
            cout << "секунды";
        }
        else {
            cout << "секунд";
        }

        cout << endl;
        this_thread::sleep_for(chrono::seconds(1));
    }

    cout << "Время вышло!" << endl;

    return 0;
}