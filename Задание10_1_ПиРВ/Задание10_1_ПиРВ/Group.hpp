#ifndef GROUP_HPP
#define GROUP_HPP

#include <string>
#include <vector>
#include <iostream>
#include <algorithm>

class Student; 

using namespace std;

class Group {
private:
    string name;
    vector<Student*> students;

public:
    Group(const string& name);
    ~Group();

    void addStudent(Student* student);
    void removeStudent(const string& studentName);
    double calculateGroupAverage() const;
    void printAllStudents() const;

    inline string getName() const {
        return name;
    }
};

#endif
