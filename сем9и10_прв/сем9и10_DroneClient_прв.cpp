#include <boost/asio.hpp>
#include <iostream>
#include <string>

using boost::asio::ip::tcp;

class Client {
public:
    Client(boost::asio::io_context& io_context, const std::string& host, short port)
        : socket_(io_context), resolver_(io_context) {
        auto endpoints = resolver_.resolve(host, std::to_string(port));
        boost::asio::connect(socket_, endpoints);
        std::cout << "[Клиент] Подключён к серверу " << host << ":" << port << std::endl;
    }

    void send_message(const std::string& message) {
        boost::asio::write(socket_, boost::asio::buffer(message));

        char reply[1024];
        size_t len = socket_.read_some(boost::asio::buffer(reply));
        std::cout << "[Клиент] Эхо-ответ: " << std::string(reply, len) << std::endl;
    }

private:
    tcp::socket socket_;
    tcp::resolver resolver_;
};

int main() {
    setlocale(LC_ALL, "Russian");
    try {
        boost::asio::io_context io_context;
        std::string host = "127.0.0.1";
        short port = 12345;

        Client client(io_context, host, port);

        std::string input;
        while (true) {
            std::cout << "Введите координаты (или 'quit' для выхода): ";
            std::getline(std::cin, input);
            if (input == "quit") break;
            client.send_message(input);
        }
    }
    catch (std::exception& e) {
        std::cerr << "[Клиент] Ошибка: " << e.what() << std::endl;
    }
    return 0;
}