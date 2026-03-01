#ifndef STUDENT_HPP
#define STUDENT_HPP

#include "Person.hpp"
#include "RecordBook.hpp"
#include <vector>

using namespace std;

class Student : public Person {
private:
    RecordBook recordBook;
    vector<double> grades;

public:
    Student(const string& name, const string& recordNumber);
    ~Student();

    void addGrade(double grade);
    double calculateAverage() const;
    void print() const override;

    inline string getRecordNumber() const {
        return recordBook.getNumber();
    }
};

#endif
