#include "Group.hpp"
#include "Student.hpp"

Group::Group(const string& name) : name(name) {}

Group::~Group() {}

void Group::addStudent(Student* student) {
    if (student != nullptr) {
        students.push_back(student);
        cout << "  Студент " << student->getName() << " добавлен в группу " << name << endl;
    }
}

bool Group::removeStudent(const string& studentName) {
    auto it = find_if(students.begin(), students.end(),
        [&studentName](Student* s) {
            return s->getName() == studentName;
        });

    if (it != students.end()) {
        cout << "  Студент " << studentName << " удалён из группы " << name << endl;
        students.erase(it);
        return true;
    }

    cout << "  Студент " << studentName << " не найден в группе " << name << endl;
    return false;
}

void Group::printAllStudents() const {
    cout << "\n=== Группа: " << name << " ===" << endl;
    if (students.empty()) {
        cout << "  В группе нет студентов" << endl;
        return;
    }

    cout << "  Всего студентов: " << students.size() << endl;
    for (size_t i = 0; i < students.size(); ++i) {
        cout << "  " << i + 1 << ". ";
        students[i]->print();
    }
}

double Group::calculateGroupAverage() const {
    if (students.empty()) return 0.0;

    double sum = 0.0;
    for (const auto* student : students) {
        sum += student->calculateAverage();
    }
    return sum / students.size();
}

void Group::sortStudentsByAverage(bool descending) {
    if (descending) {
        sort(students.begin(), students.end(),
            [](const Student* a, const Student* b) {
                return a->calculateAverage() > b->calculateAverage();
            });
    }
    else {
        sort(students.begin(), students.end(),
            [](const Student* a, const Student* b) {
                return a->calculateAverage() < b->calculateAverage();
            });
    }
    cout << "  Студенты отсортированы по среднему баллу" << (descending ? " (по убыванию)" : " (по возрастанию)") << endl;
}

vector<Student*> Group::filterByAverage(double threshold, bool above) const {
    vector<Student*> result;

    for (auto* student : students) {
        if (above && student->calculateAverage() >= threshold) {
            result.push_back(student);
        }
        else if (!above && student->calculateAverage() < threshold) {
            result.push_back(student);
        }
    }

    return result;
}

Student* Group::findStudent(const string& studentName) const {
    auto it = find_if(students.begin(), students.end(),
        [&studentName](Student* s) {
            return s->getName() == studentName;
        });

    return (it != students.end()) ? *it : nullptr;
}

int Group::countExcellent() const {
    int count = 0;
    for (const auto* student : students) {
        if (student->calculateAverage() >= 4.5) {
            count++;
        }
    }
    return count;
}

int Group::countAtRisk() const {
    int count = 0;
    for (const auto* student : students) {
        if (student->calculateAverage() < 3.0) {
            count++;
        }
    }
    return count;
}