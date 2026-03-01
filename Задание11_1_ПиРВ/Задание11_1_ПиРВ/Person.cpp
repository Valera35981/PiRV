#include "Person.hpp"

Person::Person(const string& name) : name(name) {}

Person::~Person() {}

void Person::print() const {
    cout << "Человек: " << name << endl;
}

string Person::getName() const {
    return name;
}

void Person::setName(const string& newName) {
    name = newName;
}