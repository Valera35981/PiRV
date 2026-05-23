#include <boost/asio.hpp>
#include <iostream>
#include <string>
#include <windows.h>
#include <thread>

using namespace std;
using boost::asio::ip::tcp;

int main() {
    SetConsoleOutputCP(1251);
    SetConsoleCP(1251);
    setlocale(LC_ALL, "Russian");

    try {
        boost::asio::io_context io_context;

        tcp::socket socket(io_context);
        tcp::endpoint endpoint(boost::asio::ip::make_address("127.0.0.1"), 12345);

        socket.connect(endpoint);

        cout << "=== СЕРВЕР НАПОМИНАНИЙ ===" << endl;
        cout << "Подключено к серверу" << endl;
        cout << "Формат команды: remind <секунды> <текст>" << endl;
        cout << "Пример: remind 3 Сделать домашнее задание" << endl;
        cout << "Для выхода введите 'exit'" << endl;
        cout << endl;

        string input;
        thread reader([&socket]() {
            try {
                while (true) {
                    boost::asio::streambuf buffer;
                    boost::asio::read_until(socket, buffer, '\n');

                    istream input_stream(&buffer);
                    string response;
                    getline(input_stream, response);

                    if (!response.empty()) {
                        cout << "\n[СЕРВЕР] " << response << endl;
                        cout << "> " << flush;
                    }
                }
            }
            catch (exception& e) {
            }
            });

        while (true) {
            cout << "> ";
            getline(cin, input);

            if (input == "exit" || input == "quit") {
                break;
            }

            if (input.empty()) {
                continue;
            }
            input += "\n";
            boost::asio::write(socket, boost::asio::buffer(input));
        }

        socket.close();
        reader.detach(); 
        cout << "Соединение закрыто" << endl;

    }
    catch (exception& e) {
        cerr << "Ошибка клиента: " << e.what() << endl;
    }

    return 0;
}