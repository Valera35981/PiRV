#ifndef GROUP_HPP
#define GROUP_HPP

#include <string>
#include <vector>
#include <iostream>
#include <algorithm>
#include <functional>

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
    bool removeStudent(const string& studentName);
    void printAllStudents() const;
    double calculateGroupAverage() const;
    void sortStudentsByAverage(bool descending = true);
    vector<Student*> filterByAverage(double threshold, bool above = true) const;
    Student* findStudent(const string& studentName) const;
    int countExcellent() const;
    int countAtRisk() const;

    inline string getName() const {
        return name;
    }

    size_t getStudentCount() const {
        return students.size();
    }
};

#endif
