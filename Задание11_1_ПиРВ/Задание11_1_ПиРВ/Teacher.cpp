#include "Teacher.hpp"

Teacher::Teacher(const string& name, const string& subject)
    : Person(name), subject(subject) {}

Teacher::~Teacher() {}

void Teacher::print() const {
    cout << "Преподаватель: " << name << ", предмет: " << subject << endl;
}

void Teacher::setSubject(const string& newSubject) {
    subject = newSubject;
}