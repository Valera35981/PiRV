#ifndef RECORDBOOK_HPP
#define RECORDBOOK_HPP

#include <string>
#include <vector>
#include <iostream>

using namespace std;

class RecordBook {
private:
    string number;
    vector<double> grades;

public:
    RecordBook(const string& number);
    ~RecordBook();

    void addGrade(double grade);
    double calculateAverage() const;
    void printGrades() const;
    size_t getGradeCount() const;

    inline string getNumber() const {
        return number;
    }
};

#endif
