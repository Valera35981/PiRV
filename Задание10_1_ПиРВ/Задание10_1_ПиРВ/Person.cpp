#include "Person.hpp"

Person::Person(const string& name) : name(name) {}

Person::~Person() {}

void Person::print() const {
    cout << "Person: " << name << endl;
}

string Person::getName() const {
    return name;
}