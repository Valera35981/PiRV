#include "FileManager.hpp"
#include "Group.hpp"
#include <cstring>

#pragma pack(push, 1)
struct FileHeader {
    char signature[8];
    int version;
    int studentCount;
};
#pragma pack(pop)

FileManager::FileManager() {}

FileManager::~FileManager() {}

bool FileManager::saveGroupToFile(const Group& group, const string& filename) {
    ofstream file(filename, ios::binary);
    if (!file.is_open()) return false;

    FileHeader header;
    memcpy(header.signature, "GROUP10", 7);
    header.signature[7] = '\0';
    header.version = 1;
    header.studentCount = 0;

    file.write(reinterpret_cast<char*>(&header), sizeof(header));
    file.close();

    return true;
}

bool FileManager::loadGroupFromFile(Group& group, const string& filename) {
    ifstream file(filename, ios::binary);
    if (!file.is_open()) return false;

    FileHeader header;
    file.read(reinterpret_cast<char*>(&header), sizeof(header));
    file.close();

    return strcmp(header.signature, "GROUP10") == 0;
}