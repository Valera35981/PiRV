#include "server.hpp"
#include <iostream>
#include <thread>
#include <vector>
#include <windows.h>

using namespace std;

int main(int argc, char* argv[]) {
    SetConsoleOutputCP(1251);
    SetConsoleCP(1251);
    setlocale(LC_ALL, "Russian");
    short port = 12345;
    int num_threads = 4;
    if (argc > 1) {
        port = static_cast<short>(stoi(argv[1]));
    }
    if (argc > 2) {
        num_threads = stoi(argv[2]);
    }

    cout << "=== МНОГОПОТОЧНЫЙ TCP-СЕРВЕР ===" << endl;
    cout << "Порт: " << port << endl;
    cout << "Потоков в пуле: " << num_threads << endl;
    cout << "================================" << endl;

    try {
        boost::asio::io_context io_context;
        Logger logger(io_context);

        Server server(io_context, port, logger);
        server.start();

        logger.log("Запуск " + to_string(num_threads) + " потоков");
        vector<thread> threads;
        for (int i = 0; i < num_threads; ++i) {
            threads.emplace_back([&io_context]() {
                try {
                    io_context.run();
                }
                catch (exception& e) {
                    cerr << "Ошибка в потоке: " << e.what() << endl;
                }
                });
        }
        cout << "Сервер запущен. Нажмите Enter для остановки..." << endl;
        cin.get();
        io_context.stop();
        for (auto& t : threads) {
            if (t.joinable()) {
                t.join();
            }
        }

        logger.print_all_logs();

    }
    catch (exception& e) {
        cerr << "Ошибка: " << e.what() << endl;
        return 1;
    }

    return 0;
}