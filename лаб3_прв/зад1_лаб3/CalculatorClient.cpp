#include <boost/asio.hpp>
#include <iostream>
#include <string>

using namespace std;
using boost::asio::ip::tcp;

int main() {
    try {
        setlocale(LC_ALL, "Russian");

        boost::asio::io_context io_context;
        tcp::socket socket(io_context);
        tcp::endpoint endpoint(boost::asio::ip::make_address("127.0.0.1"), 12345);
        socket.connect(endpoint);
        cout << "Подключено к серверу калькулятора" << endl;
        cout << "Поддерживаемые операции: +, -, *, /" << endl;
        cout << "Пример: 5 * 7" << endl;
        cout << "Введите выражение (или 'exit' для выхода): " << endl;

        string expression;

        while (true) {
            cout << "> ";
            getline(cin, expression);

            if (expression == "exit" || expression == "quit") {
                break;
            }

            if (expression.empty()) {
                continue;
            }
            expression += "\n";
            boost::asio::write(socket, boost::asio::buffer(expression));
            boost::asio::streambuf buffer;
            boost::asio::read_until(socket, buffer, '\n');
            istream input_stream(&buffer);
            string result;
            getline(input_stream, result);

            cout << "Результат: " << result << endl;
        }

        socket.close();
        cout << "Соединение закрыто" << endl;

    }
    catch (exception& e) {
        cerr << "Ошибка клиента: " << e.what() << endl;
    }

    return 0;
}