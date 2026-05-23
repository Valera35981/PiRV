#include <boost/asio.hpp>
#include <iostream>
#include <string>
#include <sstream>
#include <iomanip>


using namespace std;
using boost::asio::ip::tcp;

// Функция для вычисления выражения
string calculate(const string& expression) {
    istringstream iss(expression);
    double a, b;
    char op;

    iss >> a >> op >> b;

    if (iss.fail()) {
        return "Ошибка: неверный формат. Используйте: число операция число";
    }

    double result;

    switch (op) {
    case '+':
        result = a + b;
        break;
    case '-':
        result = a - b;
        break;
    case '*':
        result = a * b;
        break;
    case '/':
        if (b == 0) {
            return "Ошибка: деление на ноль";
        }
        result = a / b;
        break;
    default:
        return "Ошибка: неподдерживаемая операция. Используйте +, -, *, /";
    }

    // Для деления выводим 2 знака после запятой
    if (op == '/') {
        ostringstream oss;
        oss << fixed << setprecision(2) << result;
        return oss.str();
    }

    // Для целочисленных результатов выводим без десятичной части
    if (result == static_cast<int>(result)) {
        return to_string(static_cast<int>(result));
    }

    ostringstream oss;
    oss << fixed << setprecision(2) << result;
    return oss.str();
}

int main() {
    try {
        setlocale(LC_ALL, "Russian");

        boost::asio::io_context io_context;
        tcp::acceptor acceptor(io_context, tcp::endpoint(tcp::v4(), 12345));

        cout << "Калькуляторный сервер запущен на порту 12345" << endl;
        cout << "Ожидание подключения..." << endl;

        for (;;) {
            tcp::socket socket(io_context);
            acceptor.accept(socket);

            cout << "Клиент подключен" << endl;

            // Буфер для приема данных
            boost::asio::streambuf buffer;

            // Чтение до символа новой строки
            boost::asio::read_until(socket, buffer, '\n');

            // Преобразуем буфер в строку
            istream input_stream(&buffer);
            string expression;
            getline(input_stream, expression);

            cout << "Получено выражение: " << expression << endl;

            // Вычисляем результат
            string result = calculate(expression);

            // Добавляем символ новой строки для клиента
            result += "\n";

            // Отправляем результат
            boost::asio::write(socket, boost::asio::buffer(result));

            cout << "Отправлен результат: " << result;
            cout << "Соединение закрыто" << endl;
        }
    }
    catch (exception& e) {
        cerr << "Ошибка сервера: " << e.what() << endl;
    }

    return 0;
}