#include "Utils.h"

Utils::Utils() {}

Utils::~Utils() {}

string Utils::getFileNameFromPath(string path) {
    return path.substr(path.find_last_of("/\\") + 1);
}

string Utils::getPathFromFileName(string fileName) {
    return fileName.substr(0, fileName.find_last_of("/\\") + 1);
}
