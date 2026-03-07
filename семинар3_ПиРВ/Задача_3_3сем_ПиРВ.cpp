#include <iostream>
#include <chrono>
#include <clocale>
#include <windows.h>

using namespace std;

int main() {
    setlocale(LC_ALL, "Russian");
    SetConsoleCP(1251);
    SetConsoleOutputCP(1251);

    long long total_seconds;
    cout << "Введите количество секунд: ";
    cin >> total_seconds;
    chrono::seconds total_duration(total_seconds);
    auto hours = chrono::duration_cast<chrono::hours>(total_duration);
    auto minutes = chrono::duration_cast<chrono::minutes>(total_duration - hours);
    auto seconds = chrono::duration_cast<chrono::seconds>(total_duration - hours - minutes);
    cout << hours.count() << " ";
    if (hours.count() % 10 == 1 && hours.count() % 100 != 11) {
        cout << "час";
    }
    else if ((hours.count() % 10 >= 2 && hours.count() % 10 <= 4) &&
        (hours.count() % 100 < 10 || hours.count() % 100 >= 20)) {
        cout << "часа";
    }
    else {
        cout << "часов";
    }

    cout << " " << minutes.count() << " ";
    if (minutes.count() % 10 == 1 && minutes.count() % 100 != 11) {
        cout << "минута";
    }
    else if ((minutes.count() % 10 >= 2 && minutes.count() % 10 <= 4) &&
        (minutes.count() % 100 < 10 || minutes.count() % 100 >= 20)) {
        cout << "минуты";
    }
    else {
        cout << "минут";
    }

    cout << " " << seconds.count() << " ";
    if (seconds.count() % 10 == 1 && seconds.count() % 100 != 11) {
        cout << "секунда";
    }
    else if ((seconds.count() % 10 >= 2 && seconds.count() % 10 <= 4) &&
        (seconds.count() % 100 < 10 || seconds.count() % 100 >= 20)) {
        cout << "секунды";
    }
    else {
        cout << "секунд";
    }

    cout << endl;

    return 0;
}