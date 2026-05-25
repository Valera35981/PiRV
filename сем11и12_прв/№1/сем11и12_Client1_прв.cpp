#include <boost/asio.hpp>
#include <iostream>
#include <string>
#include <locale>

using boost::asio::ip::tcp;

int main() {
    setlocale(LC_ALL, "Russian");

    try {
        boost::asio::io_context io_context;
        tcp::resolver resolver(io_context);
        auto endpoints = resolver.resolve("127.0.0.1", "12345");

        tcp::socket socket(io_context);
        boost::asio::connect(socket, endpoints);

        std::cout << "[Клиент] Подключён к серверу" << std::endl;

        std::string input;
        while (true) {
            std::cout << "Введите координаты (или 'quit' для выхода): ";
            std::getline(std::cin, input);

            if (input == "quit") break;
            boost::asio::write(socket, boost::asio::buffer(input));

            char reply[1024];
            size_t len = socket.read_some(boost::asio::buffer(reply));
            std::cout << "[Клиент] Эхо-ответ: " << std::string(reply, len) << std::endl;
        }

    }
    catch (const std::exception& e) {
        std::cerr << "[Клиент] Ошибка: " << e.what() << std::endl;
    }

    return 0;
}