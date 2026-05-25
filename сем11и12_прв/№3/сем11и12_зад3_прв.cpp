#include <boost/asio.hpp>
#include <boost/asio/awaitable.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>
#include <boost/asio/use_awaitable.hpp>
#include <boost/asio/strand.hpp>
#include <iostream>
#include <string>
#include <atomic>
#include <random>
#include <thread>
#include <vector>
#include <mutex>
#include <windows.h>

namespace asio = boost::asio;
void setup_console() {
    SetConsoleOutputCP(1251);
    SetConsoleCP(1251);
}

class BankAccount {
private:
    int64_t balance_;
    asio::strand<asio::io_context::executor_type> strand_;

public:
    BankAccount(asio::io_context& io_context, int64_t initial_balance = 0)
        : balance_(initial_balance)
        , strand_(asio::make_strand(io_context))
    {
    }
    asio::awaitable<void> async_deposit(int64_t amount) {
        if (amount < 0) {
            throw std::invalid_argument("Сумма пополнения не может быть отрицательной");
        }

        co_await asio::post(strand_, asio::use_awaitable);

        balance_ += amount;
        std::cout << "  [+] Пополнение: +" << amount
            << " | Баланс: " << balance_ << std::endl;
    }

    asio::awaitable<void> async_withdraw(int64_t amount) {
        if (amount < 0) {
            throw std::invalid_argument("Сумма снятия не может быть отрицательной");
        }

        co_await asio::post(strand_, asio::use_awaitable);

        if (balance_ < amount) {
            throw std::invalid_argument("Недостаточно средств на счету");
        }

        balance_ -= amount;
        std::cout << "  [-] Снятие: -" << amount
            << " | Баланс: " << balance_ << std::endl;
    }

    asio::awaitable<int64_t> async_get_balance() {
        co_await asio::post(strand_, asio::use_awaitable);
        co_return balance_;
    }
};

asio::awaitable<void> perform_transactions(
    BankAccount& account,
    int client_id,
    int num_deposits,
    int num_withdrawals,
    int max_amount,
    std::atomic<int64_t>& total_deposits,
    std::atomic<int64_t>& total_withdrawals,
    std::atomic<int>& completed_clients)
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(1, max_amount);

    try {
        for (int i = 0; i < num_deposits; ++i) {
            int amount = dis(gen);
            co_await account.async_deposit(amount);
            total_deposits += amount;
            co_await asio::post(asio::use_awaitable); 
        }
        for (int i = 0; i < num_withdrawals; ++i) {
            int amount = dis(gen);
            try {
                co_await account.async_withdraw(amount);
                total_withdrawals += amount;
            }
            catch (const std::invalid_argument& e) {
                std::cout << "  [!] Клиент " << client_id
                    << ": недостаточно средств для снятия " << amount << std::endl;
            }
            co_await asio::post(asio::use_awaitable);
        }

        completed_clients++;
        std::cout << "\n[Клиент " << client_id << "] завершил все операции" << std::endl;

    }
    catch (const std::exception& e) {
        std::cerr << "Ошибка у клиента " << client_id << ": " << e.what() << std::endl;
    }
}

asio::awaitable<void> run_bank_test(
    asio::io_context& io_context,
    int num_clients,
    int operations_per_client,
    int max_amount)
{
    BankAccount account(io_context, 0);

    std::atomic<int64_t> total_deposits{ 0 };
    std::atomic<int64_t> total_withdrawals{ 0 };
    std::atomic<int> completed_clients{ 0 };

    std::vector<asio::awaitable<void>> tasks;

    std::cout << "\n=== ЗАПУСК БАНКОВСКОГО ТЕСТА ===" << std::endl;
    std::cout << "Клиентов: " << num_clients << std::endl;
    std::cout << "Операций на клиента: " << operations_per_client * 2
        << " (" << operations_per_client << " депозитов + "
        << operations_per_client << " снятий)" << std::endl;
    std::cout << "Максимальная сумма операции: " << max_amount << std::endl;
    std::cout << "================================\n" << std::endl;

    for (int i = 1; i <= num_clients; ++i) {
        tasks.push_back(perform_transactions(
            account, i, operations_per_client, operations_per_client,
            max_amount, total_deposits, total_withdrawals, completed_clients));
    }

    for (auto& task : tasks) {
        co_await std::move(task);
    }
    int64_t final_balance = co_await account.async_get_balance();

    std::cout << "\n\n========== РЕЗУЛЬТАТЫ ==========" << std::endl;
    std::cout << "Всего депозитов:   " << total_deposits.load() << std::endl;
    std::cout << "Всего снятий:      " << total_withdrawals.load() << std::endl;
    std::cout << "Ожидаемый баланс:  " << (total_deposits.load() - total_withdrawals.load()) << std::endl;
    std::cout << "Фактический баланс: " << final_balance << std::endl;
    std::cout << "=================================" << std::endl;

    if (final_balance == (total_deposits.load() - total_withdrawals.load())) {
        std::cout << "\n ТЕСТ ПРОЙДЕН! Нет гонок данных!" << std::endl;
    }
    else {
        std::cout << "\n ОШИБКА! Обнаружены гонки данных!" << std::endl;
    }
}

class BankAccountNoStrand {
private:
    int64_t balance_;
    std::mutex mutex_; 

public:
    BankAccountNoStrand(int64_t initial_balance = 0) : balance_(initial_balance) {}

    void deposit(int64_t amount) {
        std::lock_guard<std::mutex> lock(mutex_);
        balance_ += amount;
    }

    void withdraw(int64_t amount) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (balance_ >= amount) balance_ -= amount;
    }

    int64_t get_balance() {
        std::lock_guard<std::mutex> lock(mutex_);
        return balance_;
    }
};


int main() {
    setup_console();

    std::cout << "===========================================" << std::endl;
    std::cout << "   БАНКОВСКИЙ СЧЁТ С АСИНХРОННЫМИ ТРАНЗАКЦИЯМИ" << std::endl;
    std::cout << "===========================================\n" << std::endl;
    const int NUM_THREADS = 4;           
    const int NUM_CLIENTS = 20;          
    const int OPERATIONS_PER_CLIENT = 10; 
    const int MAX_AMOUNT = 100;         

    std::cout << "Настройки:" << std::endl;
    std::cout << "  Потоков в пуле: " << NUM_THREADS << std::endl;
    std::cout << "  Клиентов: " << NUM_CLIENTS << std::endl;
    std::cout << "  Операций/клиент: " << OPERATIONS_PER_CLIENT * 2 << std::endl;
    std::cout << "  Макс. сумма: " << MAX_AMOUNT << std::endl;
    std::cout << std::endl;
    asio::io_context io_context(NUM_THREADS);

    asio::co_spawn(io_context,
        run_bank_test(io_context, NUM_CLIENTS, OPERATIONS_PER_CLIENT, MAX_AMOUNT),
        asio::detached);

    std::vector<std::thread> threads;
    for (int i = 0; i < NUM_THREADS; ++i) {
        threads.emplace_back([&io_context]() {
            io_context.run();
            });
    }

    for (auto& t : threads) {
        t.join();
    }

    std::cout << "\nПрограмма завершена. Нажмите Enter для выхода...";
    std::cin.get();

    return 0;
}