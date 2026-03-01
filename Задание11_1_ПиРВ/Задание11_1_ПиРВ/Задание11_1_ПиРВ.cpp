#include <iostream>
#include <clocale>
#include <vector>
#include <memory>
#include "Student.hpp"
#include "Teacher.hpp"
#include "Group.hpp"
#include "FileManager.hpp"

using namespace std;

int main() {
    setlocale(LC_ALL, "Russian");

    cout << "ПОЛНЫЙ МНОГОМОДУЛЬНЫЙ ПРОЕКТ \n";

    cout << "\n 1. Создание студентов \n";
    vector<unique_ptr<Student>> students;

    students.push_back(make_unique<Student>("Иван Петров", "1001"));
    students.push_back(make_unique<Student>("Анна Сидорова", "1002"));
    students.push_back(make_unique<Student>("Пётр Иванов", "1003"));
    students.push_back(make_unique<Student>("Мария Смирнова", "1004"));
    students.push_back(make_unique<Student>("Алексей Козлов", "1005"));

    cout << "\n 2. Добавление оценок \n";
    students[0]->addGrade(4.5);
    students[0]->addGrade(5.0);
    students[0]->addGrade(4.0);

    students[1]->addGrade(3.5);
    students[1]->addGrade(4.0);
    students[1]->addGrade(4.5);

    students[2]->addGrade(5.0);
    students[2]->addGrade(5.0);
    students[2]->addGrade(4.5);

    students[3]->addGrade(3.0);
    students[3]->addGrade(3.5);
    students[3]->addGrade(4.0);

    students[4]->addGrade(2.5);
    students[4]->addGrade(3.0);
    students[4]->addGrade(3.5);

    cout << "\n 3. Создание преподавателя \n";
    Teacher teacher("Мария Ивановна", "Математика");
    teacher.print();

    cout << "\n 4. Создание группы \n";
    Group group("ИУ1-43Б");

    cout << "\n 5. Добавление студентов в группу \n";
    for (const auto& student : students) {
        group.addStudent(student.get());
    }

    cout << "\n 6. Вывод группы \n";
    group.printAllStudents();
    cout << "\nСредний балл группы: " << group.calculateGroupAverage() << endl;

    cout << "\n 7. Сортировка студентов по среднему баллу\n";
    group.sortStudentsByAverage(true);
    group.printAllStudents();

    cout << "\n 8. Фильтрация студентов (средний балл >= 4.0) \n";
    auto filtered = group.filterByAverage(4.0, true);
    cout << "Найдено " << filtered.size() << " студентов с баллом >= 4.0:\n";
    for (const auto* student : filtered) {
        cout << "  - " << student->getName() << ": " << student->calculateAverage() << endl;
    }

    cout << "\n 9. Статистика \n";
    cout << "Отличников (>= 4.5): " << group.countExcellent() << endl;
    cout << "Студентов с риском отчисления (< 3.0): " << group.countAtRisk() << endl;

    cout << "\n 10. Поиск студента \n";
    Student* found = group.findStudent("Анна Сидорова");
    if (found) {
        cout << "Найден: ";
        found->print();
    }

    cout << "\n 11. Удаление студента \n";
    group.removeStudent("Пётр Иванов");
    group.printAllStudents();

    cout << "\n 12. Сохранение в файл \n";
    FileManager fileManager;
    fileManager.saveGroupToFile(group, "group.dat");

    cout << "\n 13. Проверка файла \n";
    if (fileManager.checkFileExists("group.dat")) {
        cout << "Файл group.dat существует, размер: "
            << fileManager.getFileSize("group.dat") << " байт" << endl;
    }

    cout << "\n 14. Загрузка из файла \n";
    Group loadedGroup("Временная");
    vector<Student*> emptyStorage;
    fileManager.loadGroupFromFile(loadedGroup, "group.dat", emptyStorage);

    cout << "\n ПРОГРАММА ЗАВЕРШЕНА \n";

    return 0;
}