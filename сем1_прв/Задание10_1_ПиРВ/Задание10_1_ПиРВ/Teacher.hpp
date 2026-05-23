#ifndef TEACHER_HPP
#define TEACHER_HPP

#include "Person.hpp"
#include <string>

using namespace std;

class Teacher : public Person {
private:
    string subject;

public:
    Teacher(const string& name, const string& subject);
    ~Teacher();

    void print() const override;

    inline string getSubject() const {
        return subject;
    }
};

#endif
