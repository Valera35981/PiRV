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

    vector<pair<int, double>> studentAverages;

    for (int i = 0; i < N; ++i) {
        double avg = calculateStudentAverage(grades[i]);
        studentAverages.push_back(make_pair(i, avg));
    }

    cout << "\n ДО СОРТИРОВКИ \n";
    for (const auto& student : studentAverages) {
        printStudentInfo(student.first, student.second);
    }

    sort(studentAverages.begin(), studentAverages.end(),
        [](const pair<int, double>& a, const pair<int, double>& b) {
            if (a.second != b.second) {
                return a.second > b.second; 
            }
            return a.first < b.first; 
        });

    cout << "\n ПОСЛЕ СОРТИРОВКИ \n";
    for (const auto& student : studentAverages) {
        printStudentInfo(student.first, student.second);
    }

    return 0;
}
