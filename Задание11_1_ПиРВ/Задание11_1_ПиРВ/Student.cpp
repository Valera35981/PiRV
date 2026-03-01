#include "Student.hpp"

Student::Student(const string& name, const string& recordNumber)
    : Person(name), recordBook(recordNumber) {}

Student::~Student() {}

void Student::addGrade(double grade) {
    recordBook.addGrade(grade);
}

double Student::calculateAverage() const {
    return recordBook.calculateAverage();
}

void Student::print() const {
    cout << "Студент: " << name << ", средний балл: " << calculateAverage()
        << ", оценок: " << getGradeCount() << endl;
}

string Student::getRecordNumber() const {
    return recordBook.getNumber();
}

size_t Student::getGradeCount() const {
    return recordBook.getGradeCount();
}