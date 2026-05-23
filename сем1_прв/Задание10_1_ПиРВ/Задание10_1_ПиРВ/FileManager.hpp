#ifndef FILEMANAGER_HPP
#define FILEMANAGER_HPP

#include <string>
#include <fstream>
#include <iostream>

class Group;  // forward declaration

using namespace std;

class FileManager {
public:
    FileManager();
    ~FileManager();

    bool saveGroupToFile(const Group& group, const string& filename);
    bool loadGroupFromFile(Group& group, const string& filename);

    inline bool checkFileExists(const string& filename) const {
        ifstream file(filename);
        return file.good();
    }
};

#endif
