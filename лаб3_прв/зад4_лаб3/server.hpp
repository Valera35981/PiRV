#pragma once
#include <boost/asio.hpp>
#include <memory>
#include <string>
#include <vector>
#include "Logger.hpp"

using namespace std;
using boost::asio::ip::tcp;
uint64_t factorial(int n);
bool is_prime(int n);
vector<int> sort_numbers(const vector<int>& numbers);
class Session : public enable_shared_from_this<Session> {
public:
    Session(tcp::socket socket, boost::asio::io_context& io_context, Logger& logger);
    void start();

private:
    void do_read();
    void handle_request(const string& request);
    void do_write(const string& response);

    tcp::socket socket_;
    boost::asio::io_context& io_context_;
    Logger& logger_;
    enum { max_length = 1024 };
    char data_[max_length];
    string client_address_;
};
class Server {
public:
    Server(boost::asio::io_context& io_context, short port, Logger& logger);
    void start();

private:
    void do_accept();

    boost::asio::io_context& io_context_;
    tcp::acceptor acceptor_;
    Logger& logger_;
};