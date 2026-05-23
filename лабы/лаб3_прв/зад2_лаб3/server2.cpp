#include <boost/asio.hpp>
#include <iostream>
#include <memory>
#include <string>
#include <sstream>
#include <vector>
#include <numeric>
#include <iomanip>
#include <windows.h>

using namespace std;
using boost::asio::ip::tcp;

class Session : public enable_shared_from_this<Session> {
public:
    Session(tcp::socket socket, boost::asio::io_context& io_context)
        : socket_(move(socket)), io_context_(io_context) {}

    void start() {
        do_read();
    }

private:
    void do_read() {
        auto self(shared_from_this());

        socket_.async_read_some(boost::asio::buffer(data_, max_length),
            [this, self](boost::system::error_code ec, size_t length) {
                if (!ec && length > 0) {
                    string request(data_, length);
                    cout << "Получено: " << request << endl;
                    auto request_ptr = make_shared<string>(request);

                    boost::asio::post(io_context_, [this, self, request_ptr]() {
                        string response = calculate_average(*request_ptr);
                        do_write(response);
                        });
                }
                else if (ec) {
                    cout << "Клиент отключился" << endl;
                }
            });
    }

    string calculate_average(const string& input) {
        istringstream iss(input);
        vector<double> numbers;
        double num;

        while (iss >> num) {
            numbers.push_back(num);
        }

        if (numbers.empty()) {
            return "Ошибка: не введено чисел\n";
        }

        double sum = accumulate(numbers.begin(), numbers.end(), 0.0);
        double average = sum / numbers.size();

        ostringstream oss;
        oss << "Среднее: " << fixed << setprecision(2) << average << "\n";

        return oss.str();
    }

    void do_write(const string& response) {
        auto self(shared_from_this());
        auto response_ptr = make_shared<string>(response);

        boost::asio::async_write(socket_, boost::asio::buffer(*response_ptr),
            [this, self, response_ptr](boost::system::error_code ec, size_t ) {
                if (!ec) {
                    do_read();
                }
            });
    }

    tcp::socket socket_;
    boost::asio::io_context& io_context_;
    enum { max_length = 1024 };
    char data_[max_length];
};

class Server {
public:
    Server(boost::asio::io_context& io_context, short port)
        : io_context_(io_context), acceptor_(io_context, tcp::endpoint(tcp::v4(), port)) {
        do_accept();
    }

private:
    void do_accept() {
        acceptor_.async_accept(
            [this](boost::system::error_code ec, tcp::socket socket) {
                if (!ec) {
                    cout << "Новый клиент подключился" << endl;
                    make_shared<Session>(move(socket), io_context_)->start();
                }
                do_accept();
            });
    }

    boost::asio::io_context& io_context_;
    tcp::acceptor acceptor_;
};

int main() {
    SetConsoleOutputCP(1251);
    SetConsoleCP(1251);
    setlocale(LC_ALL, "Russian");

    try {
        boost::asio::io_context io_context;

        Server server(io_context, 12345);

        cout << "Сервер запущен на порту 12345" << endl;
        cout << "Ожидание подключений..." << endl;

        io_context.run();

    }
    catch (exception& e) {
        cerr << "Ошибка: " << e.what() << endl;
    }

    return 0;
}