#include "FileContent.h"

void FileContent::addDependency(const std::string &dep) {
    dependencies.push_back(dep);
}

void FileContent::addContent(const std::string &line) {
    content += line + "\n";
}
