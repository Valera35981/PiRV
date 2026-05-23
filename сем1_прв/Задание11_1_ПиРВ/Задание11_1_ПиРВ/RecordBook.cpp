#include "RecordBook.hpp"

RecordBook::RecordBook(const string& number) : number(number) {}

RecordBook::~RecordBook() {}

void RecordBook::addGrade(double grade) {
    if (grade >= 0.0 && grade <= 5.0) {
        grades.push_back(grade);
    }
}

double RecordBook::calculateAverage() const {
    if (grades.empty()) return 0.0;

    double sum = 0.0;
    for (double g : grades) {
        sum += g;
    }
    return sum / grades.size();
}

void RecordBook::printGrades() const {
    cout << "Зачётка #" << number << ": ";
    if (grades.empty()) {
        cout << "нет оценок";
    }
    else {
        for (double g : grades) {
            cout << g << " ";
        }
    }
    cout << endl;
}

size_t RecordBook::getGradeCount() const {
    return grades.size();
}