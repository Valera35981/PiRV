#include "server.hpp"
#include <sstream>
#include <algorithm>
#include <numeric>
#include <chrono>
#include <random>

using namespace std;
using namespace chrono;

uint64_t factorial(int n) {
    if (n < 0) return 0;
    if (n > 20) return 0; 
    uint64_t result = 1;
    for (int i = 2; i <= n; ++i) {
        result *= i;
        this_thread::sleep_for(milliseconds(50)); 
    }
    return result;
}


bool is_prime(int n) {
    if (n <= 1) return false;
    if (n <= 3) return true;
    if (n % 2 == 0 || n % 3 == 0) return false;

    for (int i = 5; i * i <= n; i += 6) {
        this_thread::sleep_for(milliseconds(10)); 
        if (n % i == 0 || n % (i + 2) == 0) return false;
    }
    return true;
}

vector<int> sort_numbers(const vector<int>& numbers) {
    vector<int> sorted = numbers;
    this_thread::sleep_for(milliseconds(500));
    sort(sorted.begin(), sorted.end());
    return sorted;
}
Session::Session(tcp::socket socket, boost::asio::io_context& io_context, Logger& logger)
    : socket_(move(socket)), io_context_(io_context), logger_(logger) {
    try {
        client_address_ = socket_.remote_endpoint().address().to_string();
        client_address_ += ":" + to_string(socket_.remote_endpoint().port());
    }
    catch (...) {
        client_address_ = "unknown";
    }
}

void Session::start() {
    logger_.log("Новое подключение от " + client_address_);
    do_read();
}

void Session::do_read() {
    auto self(shared_from_this());

    socket_.async_read_some(boost::asio::buffer(data_, max_length),
        [this, self](boost::system::error_code ec, size_t length) {
            if (!ec && length > 0) {
                string request(data_, length);
                while (!request.empty() && (request.back() == '\n' || request.back() == '\r')) {
                    request.pop_back();
                }

                logger_.log("Получено от " + client_address_ + ": " + request);

                boost::asio::post(io_context_, [this, self, request]() {
                    handle_request(request);
                    });

                do_read(); 
            }
            else if (ec && ec != boost::asio::error::eof) {
                logger_.log("Ошибка чтения от " + client_address_ + ": " + ec.message());
            }
        });
}

void Session::handle_request(const string& request) {
    string response;
    istringstream iss(request);
    string command;
    iss >> command;

    if (command == "factorial" || command == "факториал") {
        int n;
        iss >> n;
        logger_.log("Вычисление факториала для " + to_string(n) + " от " + client_address_);

        uint64_t result = factorial(n);
        response = "Факториал " + to_string(n) + " = " + to_string(result) + "\n";

        logger_.log_result(client_address_, "factorial " + to_string(n), to_string(result));

    }
    else if (command == "prime" || command == "простое") {
        int n;
        iss >> n;
        logger_.log("Проверка числа " + to_string(n) + " на простоту от " + client_address_);

        bool result = is_prime(n);
        response = "Число " + to_string(n) + (result ? " простое" : " не простое") + "\n";

        logger_.log_result(client_address_, "prime " + to_string(n), result ? "prime" : "not prime");

    }
    else if (command == "sort" || command == "сортировка") {
        vector<int> numbers;
        int num;
        while (iss >> num) {
            numbers.push_back(num);
        }

        logger_.log("Сортировка " + to_string(numbers.size()) + " чисел от " + client_address_);

        vector<int> sorted = sort_numbers(numbers);

        ostringstream oss;
        oss << "Отсортировано: ";
        for (size_t i = 0; i < sorted.size() && i < 20; ++i) {
            oss << sorted[i] << " ";
        }
        if (sorted.size() > 20) oss << "...";
        oss << "\n";
        response = oss.str();

        string result_str;
        for (int x : sorted) result_str += to_string(x) + " ";
        logger_.log_result(client_address_, "sort", result_str);

    }
    else if (command == "help" || command == "помощь") {
        response = "Доступные команды:\n"
            "  factorial <n> - вычислить факториал n\n"
            "  prime <n> - проверить, простое ли число n\n"
            "  sort <числа> - отсортировать числа\n"
            "  help - показать справку\n"
            "  exit - закрыть соединение\n";
    }
    else if (command == "exit") {
        response = "До свидания!\n";
        do_write(response);
        return;
    }
    else {
        response = "Неизвестная команда. Введите 'help' для справки\n";
    }

    do_write(response);
}

void Session::do_write(const string& response) {
    auto self(shared_from_this());
    auto response_ptr = make_shared<string>(response);

    boost::asio::async_write(socket_, boost::asio::buffer(*response_ptr),
        [this, self, response_ptr](boost::system::error_code ec, size_t ) {
            if (ec) {
                logger_.log("Ошибка отправки клиенту " + client_address_ + ": " + ec.message());
            }
        });
}

Server::Server(boost::asio::io_context& io_context, short port, Logger& logger)
    : io_context_(io_context), acceptor_(io_context, tcp::endpoint(tcp::v4(), port)), logger_(logger) {

    logger_.log("Сервер запущен на порту " + to_string(port));
}

void Server::start() {
    do_accept();
}

void Server::do_accept() {
    acceptor_.async_accept(
        [this](boost::system::error_code ec, tcp::socket socket) {
            if (!ec) {
                make_shared<Session>(move(socket), io_context_, logger_)->start();
            }
            else {
                logger_.log("Ошибка принятия подключения: " + ec.message());
            }
            do_accept();
        });
}