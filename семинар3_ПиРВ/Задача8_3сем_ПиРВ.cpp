#include <iostream>
#include <vector>
#include <algorithm>
#include <chrono>
#include <thread>
#include <random>
#include <clocale>
#include <windows.h>

using namespace std;
using namespace chrono;

class Task {
public:
    int value;          
    int priority;       
    int duration_ms;     
    int steps;           
    int currentStep;     

    Task(int v, int p, int d, int s)
        : value(v), priority(p), duration_ms(d), steps(s), currentStep(0) {}

    int computeResult() const {
        return value * value;
    }

    bool isCompleted() const {
        return currentStep >= steps;
    }

    double getProgress() const {
        return static_cast<double>(currentStep) / steps;
    }
};

class VirtualThread {
private:
    vector<Task> tasks;  
    int findHighestPriorityTask() {
        int maxPriorityIndex = -1;
        int maxPriority = -1;

        for (int i = 0; i < tasks.size(); ++i) {
            if (!tasks[i].isCompleted() && tasks[i].priority > maxPriority) {
                maxPriority = tasks[i].priority;
                maxPriorityIndex = i;
            }
        }
        return maxPriorityIndex;
    }

public:
    void addTask(const Task& task) {
        tasks.push_back(task);
    }
    bool runStep(int threadId) {
        int taskIndex = findHighestPriorityTask();

        if (taskIndex == -1) {
            return false; 
        }

        Task& task = tasks[taskIndex];
        task.currentStep++;

        int stepDuration = task.duration_ms / task.steps;
        this_thread::sleep_for(milliseconds(stepDuration));

        cout << "Виртуальный поток " << threadId
            << " выполняет шаг " << task.currentStep << "/" << task.steps
            << " задачи " << task.value
            << " с приоритетом " << task.priority << endl;

        if (task.isCompleted()) {
            int result = task.computeResult();
            cout << "Виртуальный поток " << threadId
                << " завершил задачу " << task.value
                << ": результат = " << result << endl;
        }

        return true;
    }

    bool hasTasks() const {
        for (const auto& task : tasks) {
            if (!task.isCompleted()) {
                return true;
            }
        }
        return false;
    }

    void removeCompletedTasks() {
        tasks.erase(
            remove_if(tasks.begin(), tasks.end(),
                [](const Task& t) { return t.isCompleted(); }),
            tasks.end()
        );
    }
};

class HyperThreadingSimulator {
private:
    VirtualThread thread1;
    VirtualThread thread2;

public:
    HyperThreadingSimulator(const VirtualThread& t1, const VirtualThread& t2)
        : thread1(t1), thread2(t2) {}

    void execute() {
        cout << "=== Имитация Hyper-Threading с приоритетами и шагами ===" << endl;
        cout << endl;
        while (thread1.hasTasks() || thread2.hasTasks()) {
            if (thread1.hasTasks()) {
                thread1.runStep(1);
            }

            if (thread2.hasTasks()) {
                thread2.runStep(2);
            }

            thread1.removeCompletedTasks();
            thread2.removeCompletedTasks();
        }

        cout << endl << "Все задачи выполнены!" << endl;
    }
};

int randomInt(int min, int max) {
    static random_device rd;
    static mt19937 gen(rd());
    uniform_int_distribution<int> dist(min, max);
    return dist(gen);
}

int main() {
    setlocale(LC_ALL, "Russian");
    SetConsoleCP(1251);
    SetConsoleOutputCP(1251);

    VirtualThread thread1;
    VirtualThread thread2;

    int numTasks1 = randomInt(2, 4);
    cout << "Генерация задач для потока 1:" << endl;
    for (int i = 0; i < numTasks1; ++i) {
        int value = randomInt(1, 50);
        int priority = randomInt(1, 10);
        int duration = randomInt(200, 1000);
        int steps = randomInt(2, 5);

        thread1.addTask(Task(value, priority, duration, steps));
        cout << "  Задача: value=" << value
            << ", priority=" << priority
            << ", duration=" << duration
            << "ms, steps=" << steps << endl;
    }

    cout << endl;
    int numTasks2 = randomInt(2, 4); 
    cout << "Генерация задач для потока 2:" << endl;
    for (int i = 0; i < numTasks2; ++i) {
        int value = randomInt(1, 50);
        int priority = randomInt(1, 10);
        int duration = randomInt(200, 1000);
        int steps = randomInt(2, 5);

        thread2.addTask(Task(value, priority, duration, steps));
        cout << "  Задача: value=" << value
            << ", priority=" << priority
            << ", duration=" << duration
            << "ms, steps=" << steps << endl;
    }

    cout << endl;

    HyperThreadingSimulator simulator(thread1, thread2);

    simulator.execute();

    return 0;
}
