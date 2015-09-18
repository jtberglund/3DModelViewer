#pragma once

#include <string>

using std::string;

class Utils {

public:
    static string getFileNameFromPath(string path);
    static string getPathFromFileName(string fileName);

private:
    Utils();
    ~Utils();
};

