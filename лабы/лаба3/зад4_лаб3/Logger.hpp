#pragma once
#include <boost/asio.hpp>
#include <iostream>
#include <vector>
#include <string>
#include <mutex>

using namespace std;

class Logger {
public:
    Logger(boost::asio::io_context& io_context)
        : strand_(boost::asio::make_strand(io_context)) {}
    void log(const string& message) {
        boost::asio::post(strand_, [this, message]() {
            logs_.push_back(message);
            cout << "[LOG] " << message << endl;
            });
    }
    void log_result(const string& client_info, const string& request, const string& result) {
        boost::asio::post(strand_, [this, client_info, request, result]() {
            string entry = "Client: " + client_info + " | Request: " + request + " | Result: " + result;
            logs_.push_back(entry);
            cout << "[LOG] " << entry << endl;
            });
    }
    vector<string> get_logs() {
        lock_guard<mutex> lock(logs_mutex_);
        return logs_;
    }

    void print_all_logs() {
        boost::asio::post(strand_, [this]() {
            cout << "\n=== бяе кнцх ===" << endl;
            for (const auto& entry : logs_) {
                cout << entry << endl;
            }
            cout << "=================" << endl;
            });
    }

private:
    boost::asio::strand<boost::asio::io_context::executor_type> strand_;
    vector<string> logs_;
    mutable mutex logs_mutex_;  
};