#include <iostream>
#include <vector>
#include <algorithm>
#include <iomanip>
#include <clocale>
#include <utility>

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
    for (double g : grades) {
        sum += g;
    }
    return sum / grades.size();
}

void printStudentInfo(int index, double average) {
    cout << "Студент " << setw(2) << index + 1
        << " | Средний балл: " << fixed << setprecision(2) << average << endl;
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
                        cout << "    Ошибка! Оценка должна быть от 0 до 5.\n";
                    }
                }
                else {
                    cout << "    Ошибка! Введите число.\n";
                }
            } while (!validInput);
        }
    }
            vector<pair<int, double>> students;
            for (int i = 0; i < N; ++i) {
                double avg = calculateStudentAverage(grades[i]);
                students.push_back(make_pair(i, avg));
            }
            cout << "\n ИСХОДНЫЕ ДАННЫЕ \n";
            for (const auto& student : students) {
                printStudentInfo(student.first, student.second);
            }

            double threshold;
            cout << "\n Введите пороговое значение для удаления студентов: ";
            while (!(cin >> threshold) || threshold< 0 || threshold > 5) {
                cout << "Ошибка! Введите число от 0 до 5: ";
                clearInput();
            }

            cout << "\n ПРИМЕНЕНИЕ ERASE-REMOVE \n";
            cout << "Порог удаления: " << fixed << setprecision(2) << threshold << endl;
            cout << "Удаляем студентов со средним баллом < " << threshold << endl;

            auto newEnd = remove_if(students.begin(), students.end(),
                [threshold](const pair<int, double>& student) {
                return student.second < threshold;  
                });

            students.erase(newEnd, students.end());

            int removedCount = N - students.size();
            cout << "Удалено студентов: " << removedCount << endl;
            cout << "Осталось студентов: " << students.size() << endl;

            cout << "\n СТУДЕНТЫ ПОСЛЕ УДАЛЕНИЯ \n";
            for (const auto& student : students) {
                printStudentInfo(student.first, student.second);
            }
            cout << "\n СТАТИСТИКА \n";

            vector<double> allAverages(N);
            for (int i = 0; i < N; ++i) {
                allAverages[i] = calculateStudentAverage(grades[i]);
            }

            int excellentCount = 0;
            for (double avg : allAverages) {
                if (avg >= 4.5) excellentCount++;
            }

            int atRiskCount = 0;
            for (double avg : allAverages) {
                if (avg < 3.0) atRiskCount++;
            }

            cout << "Всего студентов в группе: " << N << endl;
            cout << "Количество отличников (>= 4.5): " << excellentCount << endl;
            cout << "Количество студентов с риском отчисления (< 3.0): " << atRiskCount << endl;

            cout << "\n СТАТИСТИКА ПО ОСТАВШИМСЯ СТУДЕНТАМ \n";

            int excellentRemaining = 0;
            int atRiskRemaining = 0;

            for (const auto& student : students) {
                if (student.second >= 4.5) excellentRemaining++;
                if (student.second < 3.0) atRiskRemaining++;
            }

            cout << "Отличников среди оставшихся: " << excellentRemaining << endl;
            cout << "Студентов с риском отчисления среди оставшихся: " << atRiskRemaining << endl;

            return 0;
        }

       