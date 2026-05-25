#include <boost/asio.hpp>
#include <iostream>
#include <memory>
#include <thread>
#include <vector>
#include <string>

using boost::asio::ip::tcp;
int global_message_count = 0;

class Session : public std::enable_shared_from_this<Session> {
public:
    Session(tcp::socket socket,
        boost::asio::strand<boost::asio::io_context::executor_type>& strand)
        : socket_(std::move(socket)), strand_(strand) {}

    void start() {
        do_read();
    }

private:
    void do_read() {
        auto self(shared_from_this());
        socket_.async_read_some(
            boost::asio::buffer(data_, max_length),
            boost::asio::bind_executor(strand_,
                [this, self](boost::system::error_code ec, std::size_t length) {
                    if (!ec) {
                        std::string received(data_, length);
                        std::cout << "[Сервер] Получено: " << received << std::endl;

                        boost::asio::post(strand_, [this, self, received]() {
                            ++global_message_count;
                            std::cout << "[Сервер] Всего сообщений: "
                                << global_message_count << std::endl;
                            });

                        do_write(length);
                    }
                    else if (ec != boost::asio::error::eof) {
                        std::cerr << "[Сервер] Ошибка чтения: " << ec.message() << std::endl;
                    }
                })
        );
    }

    void do_write(std::size_t length) {
        auto self(shared_from_this());
        boost::asio::async_write(
            socket_,
            boost::asio::buffer(data_, length),
            boost::asio::bind_executor(strand_,
                [this, self](boost::system::error_code ec, std::size_t) {
                    if (!ec) {
                        do_read();  
                    }
                    else {
                        std::cerr << "[Сервер] Ошибка отправки: " << ec.message() << std::endl;
                    }
                })
        );
    }

    tcp::socket socket_;
    boost::asio::strand<boost::asio::io_context::executor_type>& strand_;
    enum { max_length = 1024 };
    char data_[max_length];
};

class Server {
public:
    Server(boost::asio::io_context& io_context, short port)
        : acceptor_(io_context, tcp::endpoint(tcp::v4(), port)),
        strand_(io_context.get_executor()) {
        std::cout << "[Сервер] Запущен на порту " << port << std::endl;
        do_accept();
    }

private:
    void do_accept() {
        acceptor_.async_accept(
            boost::asio::bind_executor(strand_,
                [this](boost::system::error_code ec, tcp::socket socket) {
                    if (!ec) {
                        std::cout << "[Сервер] Беспилотник подключился" << std::endl;
                        std::make_shared<Session>(std::move(socket), strand_)->start();
                    }
                    else {
                        std::cerr << "[Сервер] Ошибка принятия: " << ec.message() << std::endl;
                    }
                    do_accept();
                })
        );
    }

    tcp::acceptor acceptor_;
    boost::asio::strand<boost::asio::io_context::executor_type> strand_;
};

int main() {
    setlocale(LC_ALL, "Russian");
    try {
        boost::asio::io_context io_context;
        const int num_threads = 4;
        const short port = 12345;

        Server server(io_context, port);

        std::vector<std::thread> threads;
        for (int i = 0; i < num_threads; ++i) {
            threads.emplace_back([&io_context]() {
                io_context.run();
                });
        }

        for (auto& t : threads) {
            t.join();
        }
    }
    catch (std::exception& e) {
        std::cerr << "[Сервер] Исключение: " << e.what() << std::endl;
    }
    return 0;
}