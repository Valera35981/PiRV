#ifndef STUDENT_HPP
#define STUDENT_HPP

#include "Person.hpp"
#include "RecordBook.hpp"
#include <string>
#include <vector>

using namespace std;

class Student : public Person {
private:
    RecordBook recordBook;

public:
    Student(const string& name, const string& recordNumber);
    ~Student();

    void addGrade(double grade);
    double calculateAverage() const;
    void print() const override;
    string getRecordNumber() const;
    size_t getGradeCount() const;
};

#endif
