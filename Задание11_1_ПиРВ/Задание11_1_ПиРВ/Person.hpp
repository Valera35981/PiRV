#ifndef PERSON_HPP
#define PERSON_HPP

#include <string>
#include <iostream>

using namespace std;

class Person {
protected:
    string name;

public:
    Person(const string& name);
    virtual ~Person();

    virtual void print() const;

    string getName() const;
    void setName(const string& newName);
};

#endif
