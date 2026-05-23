#include <iostream>
#include <thread>
#include <vector>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <random>

using namespace std;
#ifdef _WIN32
#include <windows.h>
#endif

template<typename T>
class Account {
private:
    T balance;
    mutex mtx;
    condition_variable cv;
    Account(const Account&) = delete;
    Account& operator=(const Account&) = delete;

public:
    Account(T initial_balance) : balance(initial_balance) {}
    Account(Account&& other) noexcept : balance(other.balance) {
    }

    T get_balance() {
        lock_guard<mutex> lock(mtx);
        return balance;
    }

    void deposit(T amount) {
        {
            lock_guard<mutex> lock(mtx);
            balance += amount;
        }
        cv.notify_all();
    }

    bool withdraw(T amount) {
        lock_guard<mutex> lock(mtx);
        if (balance >= amount) {
            balance -= amount;
            return true;
        }
        return false;
    }
    static bool transfer(Account<T>& from, Account<T>& to, T amount) {
        if (&from == &to) return false;

        if (&from < &to) {
            unique_lock<mutex> lock_from(from.mtx);
            from.cv.wait(lock_from, [&from, amount] { return from.balance >= amount; });

            from.balance -= amount;

            unique_lock<mutex> lock_to(to.mtx);
            to.balance += amount;

            lock_from.unlock();
            to.cv.notify_all();
            from.cv.notify_all();
        }
        else {
            unique_lock<mutex> lock_to(to.mtx);
            unique_lock<mutex> lock_from(from.mtx);

            from.cv.wait(lock_from, [&from, amount] { return from.balance >= amount; });

            from.balance -= amount;
            to.balance += amount;

            lock_from.unlock();
            lock_to.unlock();
            to.cv.notify_all();
            from.cv.notify_all();
        }
        return true;
    }
};

template<typename T>
class Bank {
private:
    vector<Account<T>> accounts;

public:
    Bank(const vector<T>& initial_balances) {
        accounts.reserve(initial_balances.size());
        for (T bal : initial_balances) {
            accounts.emplace_back(bal);  
        }
    }

    void transfer(int from, int to, T amount) {
        if (from < 0 || from >= accounts.size() || to < 0 || to >= accounts.size()) {
            cout << "Ошибка: неверный номер счета!" << endl;
            return;
        }

        if (from == to) {
            cout << "Ошибка: нельзя переводить на тот же счет!" << endl;
            return;
        }

        if (amount <= 0) {
            cout << "Ошибка: сумма перевода должна быть положительной!" << endl;
            return;
        }

        Account<int>::transfer(accounts[from], accounts[to], amount);
    }

    T get_total_balance() {
        T total = 0;
        for (auto& acc : accounts) {
            total += acc.get_balance();
        }
        return total;
    }

    void print_balances() {
        cout << "Балансы счетов: ";
        for (size_t i = 0; i < accounts.size(); ++i) {
            cout << "[" << i << ":" << accounts[i].get_balance() << "] ";
        }
        cout << " | Общая сумма: " << get_total_balance() << endl;
    }

    int get_num_accounts() {
        return accounts.size();
    }
};

int main() {
#ifdef _WIN32
    SetConsoleCP(1251);
    SetConsoleOutputCP(1251);
#endif

    setlocale(LC_ALL, "Russian");

    cout << "=== Многопоточный банк ===" << endl;

    int num_accounts;
    cout << "Введите количество счетов: ";
    cin >> num_accounts;

    vector<int> initial_balances;
    cout << "Введите начальные балансы через пробел (" << num_accounts << " чисел): ";
    for (int i = 0; i < num_accounts; ++i) {
        int bal;
        cin >> bal;
        initial_balances.push_back(bal);
    }

    Bank<int> bank(initial_balances);

    cout << "\nНачальное состояние:" << endl;
    bank.print_balances();
    int num_transactions;
    cout << "\nВведите количество переводов: ";
    cin >> num_transactions;

    vector<thread> threads;
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> account_dist(0, num_accounts - 1);
    uniform_int_distribution<> amount_dist(10, 1000);

    auto start_time = chrono::steady_clock::now();

    for (int i = 0; i < num_transactions; ++i) {
        threads.emplace_back([&bank, &gen, &account_dist, &amount_dist]() {
            int from = account_dist(gen);
            int to;
            do {
                to = account_dist(gen);
            } while (from == to);

            int amount = amount_dist(gen);

            cout << "Поток [" << this_thread::get_id() << "] пытается перевести "
                << amount << " со счета " << from << " на счет " << to << endl;

            bank.transfer(from, to, amount);

            cout << "Поток [" << this_thread::get_id() << "] завершил перевод" << endl;
            });
    }

    for (auto& t : threads) {
        t.join();
    }

    auto end_time = chrono::steady_clock::now();
    auto elapsed_ms = chrono::duration_cast<chrono::milliseconds>(end_time - start_time).count();

    cout << "\nИтоговое состояние:" << endl;
    bank.print_balances();

    int initial_total = 0;
    for (int bal : initial_balances) {
        initial_total += bal;
    }

    cout << "\nПроверка сохранения общей суммы:" << endl;
    cout << "Начальная сумма: " << initial_total << endl;
    cout << "Конечная сумма: " << bank.get_total_balance() << endl;

    if (initial_total == bank.get_total_balance()) {
        cout << "Общая сумма сохранилась!" << endl;
    }
    else {
        cout << "Ошибка: общая сумма изменилась!" << endl;
    }

    cout << "\nВремя выполнения всех переводов: " << elapsed_ms << " мс" << endl;

    return 0;
}