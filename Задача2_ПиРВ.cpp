#include <iostream>
#include <vector>
#include <limits>
#include <iomanip>
#include <clocale>

using namespace std;

void clearInput() {
    cin.clear();
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
}


bool inputGrade(double& grade) {
    cin >> grade;
    if (cin.fail()) {
        clearInput();
        return false;
    }
    return true;
}

double calculateStudentAverage(const vector<double>& grades) {
    if (grades.empty()) return 0.0;

    double sum = 0.0;
    for (const double& grade : grades) {
        sum += grade;
    }
    return sum / grades.size();
}

double calculateSubjectAverage(const vector<vector<double>>& allGrades, int subjectIndex) {
    if (allGrades.empty()) return 0.0;

    double sum = 0.0;
    int count = 0;

    for (const auto& student : allGrades) {
        if (subjectIndex < student.size()) {
            sum += student[subjectIndex];
            count++;
        }
    }

    return (count > 0) ? sum / count : 0.0;
}

int findStudentWithMaxAverage(const vector<vector<double>>& allGrades) {
    if (allGrades.empty()) return -1;

    int bestStudentIndex = 0;
    double bestAverage = calculateStudentAverage(allGrades[0]);

    for (size_t i = 1; i < allGrades.size(); ++i) {
        double currentAverage = calculateStudentAverage(allGrades[i]);
        if (currentAverage > bestAverage) {
            bestAverage = currentAverage;
            bestStudentIndex = i;
        }
    }

    return bestStudentIndex;
}

int main() {
    setlocale(LC_ALL, "Russian");
    int N, M;

    cout << "Введите количество студентов: ";
    while (!(cin >> N) || N <= 0) {
        cout << "Ошибка! Введите положительное число: ";
        clearInput();
    }

    cout << "Введите количество предметов: ";
    while (!(cin >> M) || M <= 0) {
        cout << "Ошибка! Введите положительное число: ";
        clearInput();
    }

    vector<vector<double>> grades(N, vector<double>(M));

    cout << "\nВведите оценки студентов (от 0 до 5):\n";
    for (int i = 0; i < N; ++i) {
        cout << "\nСтудент " << i + 1 << ":\n";
        for (int j = 0; j < M; ++j) {
            double grade;
            bool validInput = false;

            do {
                cout << "  Предмет " << j + 1 << ": ";
                if (inputGrade(grade)) {
                    if (grade >= 0.0 && grade <= 5.0) {
                        grades[i][j] = grade;
                        validInput = true;
                    }
                    else {
                        cout << "    Ошибка! Оценка должна быть от 0 до 5. Повторите ввод.\n";
                    }
                }
                else {
                    cout << "    Ошибка! Введите число. Повторите ввод.\n";
                }
            } while (!validInput);
        }
    }

    cout << "\n СРЕДНИЕ БАЛЛЫ СТУДЕНТОВ \n";
    vector<double> studentAverages(N);
    for (int i = 0; i < N; ++i) {
        studentAverages[i] = calculateStudentAverage(grades[i]);
        cout << "Студент " << i + 1 << ": " << fixed << setprecision(2)
            << studentAverages[i] << endl;
    }

    cout << "\n СРЕДНИЕ БАЛЛЫ ПО ПРЕДМЕТАМ \n";
    for (int j = 0; j < M; ++j) {
        double subjectAvg = calculateSubjectAverage(grades, j);
        cout << "Предмет " << j + 1 << ": " << fixed << setprecision(2)
            << subjectAvg << endl;
    }

    int bestStudent = findStudentWithMaxAverage(grades);
    if (bestStudent != -1) {
        cout << "\n ЛУЧШИЙ СТУДЕНТ \n";
        cout << "Студент " << bestStudent + 1
            << " имеет максимальный средний балл: "
            << fixed << setprecision(2) << studentAverages[bestStudent] << endl;
    }

    return 0;
}