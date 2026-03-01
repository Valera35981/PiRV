#include "FileManager.hpp"
#include "Group.hpp"
#include "Student.hpp"

FileManager::FileManager() {}

FileManager::~FileManager() {}

bool FileManager::saveGroupToFile(const Group& group, const string& filename) {
    ofstream file(filename, ios::binary);
    if (!file.is_open()) {
        cout << "Ошибка: Не удалось открыть файл для записи: " << filename << endl;
        return false;
    }

    FileHeader header;
    memset(header.signature, 0, 8);
    strncpy_s(header.signature, 8, "GROUP11", 7);
    header.version = 1;

    string groupName = group.getName();
    strncpy_s(header.groupName, 50, groupName.c_str(), groupName.length());
    header.groupName[49] = '\0';

    header.studentCount = static_cast<int>(group.getStudentCount());

    file.write(reinterpret_cast<char*>(&header), sizeof(header));

    cout << "Группа сохранена в файл: " << filename << endl;
    cout << "  Размер заголовка: " << sizeof(FileHeader) << " байт" << endl;
    file.close();
    return true;
}

bool FileManager::loadGroupFromFile(Group& group, const string& filename, vector<Student*>& studentStorage) {
    ifstream file(filename, ios::binary);
    if (!file.is_open()) {
        cout << "Ошибка: Не удалось открыть файл для чтения: " << filename << endl;
        return false;
    }

    FileHeader header;
    file.read(reinterpret_cast<char*>(&header), sizeof(header));

    if (strncmp(header.signature, "GROUP11", 7) != 0) {
        cout << "Ошибка: Неверная сигнатура файла" << endl;
        file.close();
        return false;
    }

    if (header.version != 1) {
        cout << "Ошибка: Неподдерживаемая версия файла" << endl;
        file.close();
        return false;
    }

    cout << "Файл успешно загружен:" << endl;
    cout << "  Сигнатура: " << header.signature << endl;
    cout << "  Версия: " << header.version << endl;
    cout << "  Группа: " << header.groupName << endl;
    cout << "  Студентов: " << header.studentCount << endl;

    file.close();
    return true;
}