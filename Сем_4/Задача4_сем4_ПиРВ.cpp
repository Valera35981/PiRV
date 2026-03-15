#include <iostream>
#include <fstream>
#include <thread>
#include <vector>
#include <mutex>
#include <string>
#include <chrono>
#include <sstream>

using namespace std;

// Настройка русского языка для консоли
#ifdef _WIN32
#include <windows.h>
#endif

template<typename T>
class Logger {
private:
    ofstream log_file;
    mutex mtx;

    // Вспомогательная функция для преобразования ID потока в строку
    string thread_id_to_string(const thread::id& id) {
        ostringstream oss;
        oss << id;
        return oss.str();
    }

    // Inline функция для форматирования
    inline string format_message(const T& msg) {
        return "[Поток " + thread_id_to_string(this_thread::get_id()) + "] " + to_string(msg);
    }

public:
    Logger(const string& filename) {
        log_file.open(filename, ios::app);
    }

    void log(const T& message) {
        string formatted = format_message(message);

        lock_guard<mutex> lock(mtx);

        // Запись в файл
        log_file << formatted << endl;
        log_file.flush();

        // Запись в консоль
        cout << formatted << endl;
    }
};

// Специализация для string
template<>
string Logger<string>::format_message(const string& msg) {
    return "[Поток " + thread_id_to_string(this_thread::get_id()) + "] " + msg;
}

// Специализация для const char*
template<>
string Logger<const char*>::format_message(const char* const& msg) {
    return "[Поток " + thread_id_to_string(this_thread::get_id()) + "] " + string(msg);
}

// Функция worker (не статическая, как просит предупреждение VCR003, но можно игнорировать)
void worker(int id, Logger<string>& logger, int count) {
    for (int i = 0; i < count; i++) {
        logger.log("Сообщение " + to_string(i) + " от потока " + to_string(id));
        this_thread::sleep_for(chrono::milliseconds(100));
    }
}

int main() {
#ifdef _WIN32
    SetConsoleCP(1251);
    SetConsoleOutputCP(1251);
#endif

    setlocale(LC_ALL, "Russian");

    cout << "=== Потоковое логирование ===" << endl;

    string filename;
    cout << "Введите имя файла лога: ";
    cin >> filename;

    int num_threads;
    cout << "Введите количество потоков: ";
    cin >> num_threads;

    int messages_per_thread;
    cout << "Введите количество сообщений на поток: ";
    cin >> messages_per_thread;

    // Создаем логгер
    Logger<string> logger(filename);

    // Создаем потоки через лямбды
    vector<thread> threads;

    for (int i = 0; i < num_threads; i++) {
        threads.push_back(thread([&logger, i, messages_per_thread]() {
            worker(i, logger, messages_per_thread);
            }));
    }

    // Ожидаем завершения
    for (auto& t : threads) {
        t.join();
    }

    cout << "\nВсе потоки завершены. Лог сохранен в файл " << filename << endl;

    return 0;
}