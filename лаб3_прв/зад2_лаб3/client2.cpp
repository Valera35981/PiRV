#include <boost/asio.hpp>
#include <iostream>
#include <string>
#include <windows.h>

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

        cout << "Подключено к серверу" << endl;
        cout << "Введите числа через пробел (например: 10 20 30 40)" << endl;
        cout << "Для выхода введите 'exit'" << endl;

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
            boost::asio::streambuf buffer;
            boost::asio::read_until(socket, buffer, '\n');

            istream input_stream(&buffer);
            string response;
            getline(input_stream, response);

            cout << response << endl;
        }

        socket.close();
        cout << "Соединение закрыто" << endl;

    }
    catch (exception& e) {
        cerr << "Ошибка клиента: " << e.what() << endl;
    }

    return 0;
}