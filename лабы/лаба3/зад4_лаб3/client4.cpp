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
    setlocale(LC_ALL, "Russia");

    try {
        boost::asio::io_context io_context;

        tcp::socket socket(io_context);
        tcp::endpoint endpoint(boost::asio::ip::make_address("127.0.0.1"), 12345);

        socket.connect(endpoint);

        cout << "=== КЛИЕНТ МНОГОПОТОЧНОГО СЕРВЕРА ===" << endl;
        cout << "Подключено к серверу" << endl;
        cout << endl;
        cout << "Доступные команды:" << endl;
        cout << "  factorial <n>  - вычислить факториал" << endl;
        cout << "  prime <n>      - проверить, простое ли число" << endl;
        cout << "  sort <числа>   - отсортировать числа (через пробел)" << endl;
        cout << "  help           - показать справку" << endl;
        cout << "  exit           - выход" << endl;
        cout << endl;

        thread reader([&socket]() {
            try {
                while (true) {
                    boost::asio::streambuf buffer;
                    boost::asio::read_until(socket, buffer, '\n');

                    istream input_stream(&buffer);
                    string response;
                    getline(input_stream, response);

                    if (!response.empty()) {
                        cout << "\n[ОТВЕТ] " << response << endl;
                        cout << "> " << flush;
                    }
                }
            }
            catch (exception& e) {
                cout << "\n[СОЕДИНЕНИЕ ЗАКРЫТО]" << endl;
            }
            });

        string input;
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