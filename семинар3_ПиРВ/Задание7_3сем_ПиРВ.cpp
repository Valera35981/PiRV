#include <iostream>
#include <vector>
#include <string>
#include <clocale>
#include <windows.h>

using namespace std;

class VirtualThread {
private:
    vector<string> tasks;       
    int currentIndex;           
    bool taskStarted;           
    string currentTask;        

public:
    VirtualThread(const vector<string>& taskList)
        : tasks(taskList), currentIndex(0), taskStarted(false), currentTask("") {}

    void runNextTask(int threadId) {
        if (!taskStarted && currentIndex < tasks.size()) {
            currentTask = tasks[currentIndex];
            cout << "Виртуальный поток " << threadId << " начал " << currentTask << endl;
            taskStarted = true;
        }
        else if (taskStarted) {
            cout << "Виртуальный поток " << threadId << " закончил " << currentTask << endl;
            taskStarted = false;
            currentIndex++;
        }
    }
    bool hasTasks() const {
        return currentIndex < tasks.size() || taskStarted;
    }
};

class HyperThreadingSimulator {
private:
    VirtualThread thread1;
    VirtualThread thread2;

public:
    HyperThreadingSimulator(const vector<string>& tasks1, const vector<string>& tasks2)
        : thread1(tasks1), thread2(tasks2) {}

    void execute() {
        while (thread1.hasTasks() || thread2.hasTasks()) {
            if (thread1.hasTasks()) {
                thread1.runNextTask(1);
            }
            if (thread2.hasTasks()) {
                thread2.runNextTask(2);
            }
        }
    }
};

int main() {
    setlocale(LC_ALL, "Russian");
    SetConsoleCP(1251);
    SetConsoleOutputCP(1251);
    vector<string> thread1_tasks = { "Задачу A", "Задачу C" };
    vector<string> thread2_tasks = { "Задачу B", "Задачу D" };

    HyperThreadingSimulator simulator(thread1_tasks, thread2_tasks);
    simulator.execute();

    return 0;
}