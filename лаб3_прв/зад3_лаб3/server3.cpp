#include <boost/asio.hpp>
#include <iostream>
#include <memory>
#include <string>
#include <sstream>
#include <windows.h>
#include <regex>

using namespace std;
using boost::asio::ip::tcp;
using boost::asio::steady_timer;
using chrono::seconds;

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
                    if (!request.empty() && request.back() == '\n') {
                        request.pop_back();
                    }
                    cout << "Получено: " << request << endl;
                    handle_request(request);
                    do_read(); 
                }
                else if (ec && ec != boost::asio::error::eof) {
                    cout << "Ошибка: " << ec.message() << endl;
                }
            });
    }

    void handle_request(const string& request) {
        regex remind_regex(R"(remind\s+(\d+)\s+(.+))", regex::icase);
        smatch match;

        if (regex_match(request, match, remind_regex)) {
            int seconds_delay = stoi(match[1].str());
            string message = match[2].str();
            string confirm = "Напоминание будет через " + to_string(seconds_delay) + " секунд\n";
            do_write(confirm);

            auto timer = make_shared<steady_timer>(io_context_, chrono::seconds(seconds_delay));
            auto self = shared_from_this();

            timer->async_wait([this, self, timer, message](boost::system::error_code ec) {
                if (!ec) {
                    string reminder = message + "\n";
                    do_write(reminder);
                    cout << "Отправлено напоминание: " << message << endl;
                }
                });
        }
        else {
            do_write("Неизвестная команда. Используйте: remind <секунды> <текст>\n");
        }
    }

    void do_write(const string& response) {
        auto self(shared_from_this());
        auto response_ptr = make_shared<string>(response);

        boost::asio::async_write(socket_, boost::asio::buffer(*response_ptr),
            [this, self, response_ptr](boost::system::error_code ec, size_t ) {
                if (ec) {
                    cout << "Ошибка отправки: " << ec.message() << endl;
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
        cout << "Команда: remind <секунды> <текст>" << endl;

        io_context.run();

    }
    catch (exception& e) {
        cerr << "Ошибка: " << e.what() << endl;
    }

    return 0;
}