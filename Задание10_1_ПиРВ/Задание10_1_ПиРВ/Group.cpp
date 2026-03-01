#include "Group.hpp"
#include "Student.hpp"

Group::Group(const string& name) : name(name) {}

Group::~Group() {}

void Group::addStudent(Student* student) {
    if (student != nullptr) {
        students.push_back(student);
    }
}

void Group::removeStudent(const string& studentName) {
    auto it = find_if(students.begin(), students.end(),
        [&studentName](Student* s) {
            return s->getName() == studentName;
        });

    if (it != students.end()) {
        students.erase(it);
    }
}

double Group::calculateGroupAverage() const {
    if (students.empty()) return 0.0;

    double sum = 0.0;
    for (const auto* student : students) {
        sum += student->calculateAverage();
    }
    return sum / students.size();
}

void Group::printAllStudents() const {
    cout << "Group: " << name << endl;
    for (const auto* student : students) {
        student->print();
    }
}