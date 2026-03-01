#include <iostream>
#include <clocale>
#include "Student.hpp"
#include "Teacher.hpp"
#include "Group.hpp"
#include "FileManager.hpp"

using namespace std;

int main() {
    setlocale(LC_ALL, "Russian");

    cout << " МНОГОМОДУЛЬНЫЙ ПРОЕКТ \n";

    Student s1("Иван Петров", "12345");
    Student s2("Анна Сидорова", "67890");

    s1.addGrade(4.5);
    s1.addGrade(5.0);
    s2.addGrade(4.0);
    s2.addGrade(4.5);

    Group group("ИУ1-43Б");
    group.addStudent(&s1);
    group.addStudent(&s2);

    group.printAllStudents();
    cout << "Средний балл группы: " << group.calculateGroupAverage() << endl;

    FileManager fm;
    fm.saveGroupToFile(group, "group.bin");

    return 0;
}