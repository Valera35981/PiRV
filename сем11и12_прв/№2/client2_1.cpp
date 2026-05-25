#include <boost/asio.hpp>
#include <iostream>
#include <string>
#include <thread>
#include <windows.h>

namespace asio = boost::asio;
using asio::ip::tcp;


int main() {
    setlocale(LC_ALL, "Russian");

    std::cout << "========== КЛИЕНТ 1 ==========\n";
    std::cout << "Подключение к серверу...\n";

    try {
        asio::io_context io_context;
        tcp::socket socket(io_context);

        socket.connect(tcp::endpoint(asio::ip::make_address("127.0.0.1"), 12345));
        std::cout << "✓ Подключен к серверу на порту 12345!\n";
        std::cout << "================================================\n";
        std::cout << "Введите сообщения:\n\n";

        std::thread reader([&socket]() {
            char data[1024];
            try {
                while (true) {
                    size_t n = socket.read_some(asio::buffer(data));
                    if (n > 0) {
                        std::cout << "\n[Ответ от сервера]: " << std::string(data, n) << std::endl;
                        std::cout << "Ваше сообщение: " << std::flush;
                    }
                }
            }
            catch (...) {
            }
            });

        std::string message;
        while (std::getline(std::cin, message)) {
            if (message.empty()) {
                break;
            }
            asio::write(socket, asio::buffer(message + "\n"));
            std::cout << " Отправлено: " << message << std::endl;
        }

        socket.close();
        reader.detach();
        std::cout << "\nСоединение закрыто.\n";

    }
    catch (const std::exception& e) {
        std::cerr << "Ошибка: " << e.what() << std::endl;
        std::cout << "\nУбедитесь, что сервер запущен!\n";
    }

    std::cout << "Нажмите Enter для выхода...";
    std::cin.get();
    return 0;
}