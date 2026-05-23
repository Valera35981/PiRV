#include <iostream>
#include <vector>
#include <string>
#include <iomanip>
#include <windows.h>

using namespace std;

class Student {
private:
    string name;
    vector<double> grades;

public:
    Student() : name("Неизвестно") {}

    explicit Student(const string& studentName) : name(studentName) {}

    void addGrade(double grade) {
        if (grade >= 0.0 && grade <= 5.0) {
            grades.push_back(grade);
        }
    }

    double calculateAverage() const {
        if (grades.empty()) return 0.0;

        double sum = 0.0;
        for (double g : grades) {
            sum += g;
        }
        return sum / grades.size();
    }

    void printInfo() const {
        cout << "Имя: " << name << endl;
        cout << "Оценки: ";
        for (double g : grades) {
            cout << g << " ";
        }
        cout << "\nСредний балл: " << fixed << setprecision(2)
            << calculateAverage() << endl;
    }
};

int main() {
    SetConsoleCP(1251);
    SetConsoleOutputCP(1251);

    Student s1;
    Student s2("Иван");

    s2.addGrade(4.5);
    s2.addGrade(5.0);
    s2.addGrade(3.5);

    cout << "\n СТУДЕНТЫ \n";
    cout << "\nСтудент 1:\n";
    s1.printInfo();
    cout << "\nСтудент 2:\n";
    s2.printInfo();

    return 0;
}
