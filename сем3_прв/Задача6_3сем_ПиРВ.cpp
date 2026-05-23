#include <iostream>
#include <vector>
#include <clocale>
#include <windows.h>

using namespace std;

class VirtualThread {
private:
    vector<int> numbers;  
    int currentIndex;     
    long long factorial(int n) {
        long long result = 1;
        for (int i = 2; i <= n; ++i) {
            result *= i;
        }
        return result;
    }

public:
    VirtualThread(const vector<int>& nums) : numbers(nums), currentIndex(0) {}
    bool run(int threadId) {
        if (currentIndex < numbers.size()) {
            int num = numbers[currentIndex];
            long long fact = factorial(num);
            cout << "Виртуальный поток " << threadId << " вычисляет "
                << num << "! = " << fact << endl;
            currentIndex++;
            return true;
        }
        return false;
    }

    bool hasTasks() const {
        return currentIndex < numbers.size();
    }
};

class HyperThreadingSimulator {
private:
    VirtualThread thread1;
    VirtualThread thread2;

public:
    HyperThreadingSimulator(const vector<int>& nums1, const vector<int>& nums2)
        : thread1(nums1), thread2(nums2) {}

    void execute() {
        while (thread1.hasTasks() || thread2.hasTasks()) {
            if (thread1.hasTasks()) {
                thread1.run(1);
            }
            if (thread2.hasTasks()) {
                thread2.run(2);
            }
        }
    }
};

int main() {
    setlocale(LC_ALL, "Russian");
    SetConsoleCP(1251);
    SetConsoleOutputCP(1251);
    vector<int> thread1_numbers = { 5, 10 };      
    vector<int> thread2_numbers = { 7, 12 };      
    HyperThreadingSimulator simulator(thread1_numbers, thread2_numbers);
    simulator.execute();

    return 0;
}