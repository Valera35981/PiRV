#include <iostream>
#include <boost/thread.hpp>
#include <atomic>
#include <chrono>
#include <windows.h>

using namespace std;
using namespace std::chrono;
mutex mtx;
condition_variable cv;
atomic<int> current_stage(1);  
void process_stage(int stage_number, const string& stage_name, int processing_time) {
    unique_lock<mutex> lock(mtx);
    cv.wait(lock, [stage_number]() {
        return current_stage.load() == stage_number;
        });
    cout << "Этап " << stage_number << " (" << stage_name << ") начал работу" << endl;
    cout << "Обработка... (займет " << processing_time << " сек)" << endl;
    this_thread::sleep_for(seconds(processing_time));

    cout << "Этап " << stage_number << " (" << stage_name << ") завершен!" << endl;
    cout << endl;
    current_stage++;
    cv.notify_all();
}

int main() {
    SetConsoleCP(1251);
    SetConsoleOutputCP(1251);
    cout << "Задача 1: " << endl;
    cout << "Всего этапов: 4" << endl;
    cout << "Каждый следующий этап выполняется только после завершения предыдущего" << endl;
    cout << endl;
    int time1, time2, time3, time4;
    cout << "Введите время обработки для этапа 1 (Приемка товара) в секундах: ";
    cin >> time1;
    cout << "Введите время обработки для этапа 2 (Сортировка) в секундах: ";
    cin >> time2;
    cout << "Введите время обработки для этапа 3 (Упаковка) в секундах: ";
    cin >> time3;
    cout << "Введите время обработки для этапа 4 (Отгрузка) в секундах: ";
    cin >> time4;
    cout << endl;
    boost::thread stage1(process_stage, 1, "Приемка товара", time1);
    boost::thread stage2(process_stage, 2, "Сортировка", time2);
    boost::thread stage3(process_stage, 3, "Упаковка", time3);
    boost::thread stage4(process_stage, 4, "Отгрузка", time4);
    stage1.join();
    stage2.join();
    stage3.join();
    stage4.join();

    cout << "ВСЕ ЭТАПЫ ЗАВЕРШЕНЫ!" << endl;
    cout << "Товар успешно прошел все этапы обработки" << endl;

    return 0;
}