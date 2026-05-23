#include <iostream>
#include <vector>
#include <algorithm>
#include <iomanip>
#include <clocale>
#include <fstream>    
#include <cstring>    

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

void printStudentInfo(int index, const vector<double>& grades) {
    double avg = calculateStudentAverage(grades);
    cout << "Студент " << setw(2) << index + 1 << " | Средний балл: "
        << fixed << setprecision(2) << avg << " | Оценки: ";

    cout << "[ ";
    for (double g : grades) {
        cout << g << " ";
    }
    cout << "]" << endl;
}

#pragma pack(push, 1)  
struct FileHeader {
    char signature[8];      
    int version;            
    int studentCount;       
    int subjectCount;      
};
#pragma pack(pop)  

bool saveToFile(const string& filename, const vector<vector<double>>& grades) {
    ofstream file(filename, ios::binary);
    if (!file.is_open()) {
        cout << "Ошибка! Не удалось открыть файл для записи." << endl;
        return false;
    }

    FileHeader header;

    strncpy_s(header.signature,sizeof(header.signature), "GRADES1", 7);
    header.signature[7] = '\0'; 
    header.version = 1;
    header.studentCount = static_cast<int>(grades.size());
    header.subjectCount = static_cast<int>(grades.empty() ? 0 : grades[0].size());

    cout << "\n ИНФОРМАЦИЯ О СТРУКТУРЕ \n";
    cout << "Размер структуры FileHeader: " << sizeof(FileHeader) << " байт" << endl;
    cout << "  - signature: " << sizeof(header.signature) << " байт" << endl;
    cout << "  - version: " << sizeof(header.version) << " байт" << endl;
    cout << "  - studentCount: " << sizeof(header.studentCount) << " байт" << endl;
    cout << "  - subjectCount: " << sizeof(header.subjectCount) << " байт" << endl;

    file.write(reinterpret_cast<char*>(&header), sizeof(header));

    for (const auto& student : grades) {
        for (double grade : student) {
            file.write(reinterpret_cast<char*>(&grade), sizeof(grade));
        }
    }

    file.close();
    cout << "Данные успешно сохранены в файл: " << filename << endl;
    return true;
}

bool loadFromFile(const string& filename, vector<vector<double>>& grades) {
    ifstream file(filename, ios::binary);
    if (!file.is_open()) {
        cout << "Ошибка! Не удалось открыть файл для чтения." << endl;
        return false;
    }

    FileHeader header;
    file.read(reinterpret_cast<char*>(&header), sizeof(header));

    if (!file) {
        cout << "Ошибка! Не удалось прочитать заголовок файла." << endl;
        file.close();
        return false;
    }

    cout << "\n ПРОВЕРКА ФАЙЛА \n";
    cout << "Сигнатура из файла: " << header.signature << endl;

    if (strcmp(header.signature, "GRADES1") != 0) {
        cout << "Ошибка! Неверная сигнатура файла." << endl;
        file.close();
        return false;
    }

    cout << "Версия формата: " << header.version << endl;
    cout << "Количество студентов: " << header.studentCount << endl;
    cout << "Количество предметов: " << header.subjectCount << endl;

    if (header.version != 1) {
        cout << "Ошибка! Неподдерживаемая версия формата." << endl;
        file.close();
        return false;
    }

    grades.clear();
    grades.resize(header.studentCount, vector<double>(header.subjectCount));

    for (int i = 0; i < header.studentCount; ++i) {
        for (int j = 0; j < header.subjectCount; ++j) {
            double grade;
            file.read(reinterpret_cast<char*>(&grade), sizeof(grade));

            if (!file) {
                cout << "Ошибка! Не удалось прочитать оценку студента " << i + 1
                    << ", предмет " << j + 1 << endl;
                file.close();
                return false;
            }

            grades[i][j] = grade;
        }
    }

    file.close();
    cout << "Данные успешно загружены из файла: " << filename << endl;
    return true;
}

int main() {
    setlocale(LC_ALL, "Russian");

    vector<vector<double>> grades;
    int choice;

    do {
        cout << "\n МЕНЮ \n";
        cout << "1. Ввести данные вручную\n";
        cout << "2. Загрузить данные из файла\n";
        cout << "3. Сохранить данные в файл\n";
        cout << "4. Показать данные\n";
        cout << "5. Выход\n";
        cout << "Выберите действие: ";

        cin >> choice;
        clearInput();

        switch (choice) {
        case 1: {
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

            grades.assign(N, vector<double>(M));

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
            cout << "\nДанные успешно введены!\n";
            break;
        }

        case 2: {
            string filename;
            cout << "Введите имя файла для загрузки: ";
            cin >> filename;

            if (loadFromFile(filename, grades)) {
                cout << "\nЗагруженные данные:\n";
                for (size_t i = 0; i < grades.size(); ++i) {
                    printStudentInfo(i, grades[i]);
                }
            }
            break;
        }

        case 3: {
            if (grades.empty()) {
                cout << "Ошибка! Нет данных для сохранения.\n";
                break;
            }

            string filename;
            cout << "Введите имя файла для сохранения: ";
            cin >> filename;

            saveToFile(filename, grades);
            break;
        }

        case 4: {
            if (grades.empty()) {
                cout << "Нет данных для отображения.\n";
            }
            else {
                cout << "\n ТЕКУЩИЕ ДАННЫЕ \n";
                cout << "Всего студентов: " << grades.size() << endl;
                cout << "Предметов: " << (grades.empty() ? 0 : grades[0].size()) << endl;

                for (size_t i = 0; i < grades.size(); ++i) {
                    printStudentInfo(i, grades[i]);
                }

                double totalSum = 0.0;
                int totalGrades = 0;
                for (const auto& student : grades) {
                    for (double g : student) {
                        totalSum += g;
                        totalGrades++;
                    }
                }
                cout << "\nСредний балл по группе: " << fixed << setprecision(2)
                    << totalSum / totalGrades << endl;
            }
            break;
        }

        case 5:
            cout << "Программа завершена.\n";
            break;

        default:
            cout << "Ошибка! Неверный выбор.\n";
        }

    } while (choice != 5);

    return 0;
}
