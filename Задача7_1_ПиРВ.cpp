#include <iostream>
#include <string>
#include <vector>
#include <clocale>

using namespace std;

class Person {
protected:
    string имя;

public:
    Person(const string& имя) : имя(имя) {}
    virtual ~Person() {}

    virtual void print() const {
        cout << "Person: " << имя << endl;
    }
};

class Student : public Person {
private:
    double среднийБалл;

public:
    Student(const string& имя, double балл) : Person(имя), среднийБалл(балл) {}

    void print() const override {
        cout << "Студент: " << имя << ", средний балл: " << среднийБалл << endl;
    }
};

class Teacher : public Person {
private:
    string предмет;

public:
    Teacher(const string& имя, const string& предм) : Person(имя), предмет(предм) {}

    void print() const override {
        cout << "Преподаватель: " << имя << ", предмет: " << предмет << endl;
    }
};

int main() {
    setlocale(LC_ALL, "Russian");
    vector<Person*> люди;
    люди.push_back(new Student("Анатолий", 4.5));
    люди.push_back(new Teacher("Анна", "Математика"));
    люди.push_back(new Student("Алина", 3.8));
    люди.push_back(new Teacher("Егор", "Физика"));

    for (const auto& человек : люди) {
        человек->print();
    }

    for (auto человек : люди) {
        delete человек;
    }

    return 0;
}