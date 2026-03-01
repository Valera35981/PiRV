#include "Teacher.hpp"

Teacher::Teacher(const string& name, const string& subject)
    : Person(name), subject(subject) {}

Teacher::~Teacher() {}

void Teacher::print() const {
    cout << "Teacher: " << name << ", subject: " << subject << endl;
}