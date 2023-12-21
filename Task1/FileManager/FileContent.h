#pragma once
#include <string>
#include <vector>

class FileContent {
public:
    std::string content;
    std::vector<std::string> dependencies;

    void addDependency(const std::string &dep);
    void addContent(const std::string &line);
};
