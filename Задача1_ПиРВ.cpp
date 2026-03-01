#include <iostream>
#include <limits>
#include <cstdlib>
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

double calculateAverage(const double* arr, int size) {
    if (size <= 0) return 0.0;

    double sum = 0.0;
    for (int i = 0; i < size; ++i) {
        sum += arr[i];
    }
    return sum / size;
}

double findMax(const double* arr, int size) {
    if (size <= 0) return 0.0;

    double max = arr[0];
    for (int i = 1; i < size; ++i) {
        if (arr[i] > max) {
            max = arr[i];
        }
    }
    return max;
}


double findMin(const double* arr, int size) {
    if (size <= 0) return 0.0;

    double min = arr[0];
    for (int i = 1; i < size; ++i) {
        if (arr[i] < min) {
            min = arr[i];
        }
    }
    return min;
}


int countAboveThreshold(const double* arr, int size, double threshold) {
    if (size <= 0) return 0;

    int count = 0;
    for (int i = 0; i < size; ++i) {
        if (arr[i] > threshold) {
            count++;
        }
    }
    return count;
}

int main() {
    setlocale(LC_ALL, "Russian");
    int N;
    cout << "Введите количество студентов: ";  

    while (!(cin >> N) || N <= 0) {
        cout << "Ошибка! Введите положительное число: ";
        clearInput();
    }

    double* grades = new double[N];

    cout << "введите средние баллы студентов (от 0 до 5):\n";
    for (int i = 0; i < N; ++i) {
        double grade;
        bool validInput = false;

        do {
            cout << "Student " << i + 1 << ": ";
            if (inputGrade(grade)) {
                if (grade >= 0.0 && grade <= 5.0) {
                    grades[i] = grade;
                    validInput = true;
                }
                else {
                    cout << "Ошибка! Балл должен быть от 0 до 5.\n";
                }
            }
            else {
                cout << "Ошибка! Введите число.\n";
            }
        } while (!validInput);
    }

    cout << "\n Результат \n";
    cout << "Средний балл студентов: " << calculateAverage(grades, N) << endl;
    cout << "Максимальный балл: " << findMax(grades, N) << endl;
    cout << "Минимальный балл: " << findMin(grades, N) << endl;

    double threshold;
    cout << "\n Введите пороговое значение: ";
    cin >> threshold;
    cout << "Количевство студентов с баллом выше порога " << threshold << ": "
        << countAboveThreshold(grades, N, threshold) << endl;

    delete[] grades;
    cout << "\n Программа завершена. Память очищена.\n";

    return 0;
}