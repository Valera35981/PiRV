#ifndef FILEMANAGER_HPP
#define FILEMANAGER_HPP

#include <string>
#include <fstream>
#include <iostream>
#include <vector>
#include <cstring>

class Group;
class Student;

using namespace std;

#pragma pack(push, 1)
struct FileHeader {
    char signature[8];
    int version;
    char groupName[50];
    int studentCount;
};
#pragma pack(pop)

class FileManager {
public:
    FileManager();
    ~FileManager();

    bool saveGroupToFile(const Group& group, const string& filename);
    bool loadGroupFromFile(Group& group, const string& filename, vector<Student*>& studentStorage);

    inline bool checkFileExists(const string& filename) const {
        ifstream file(filename);
        return file.good();
    }

    inline size_t getFileSize(const string& filename) const {
        ifstream file(filename, ios::binary | ios::ate);
        return file.tellg();
    }
};

#endif
