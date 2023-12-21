#include "FileUtils.h"
#include <fstream>
#include <iostream>

void readFile(const fs::path &filePath, FileContent &fileContent) {
    std::ifstream file(filePath);
    if (!file.is_open()) {
        std::cerr << "Не удалось открыть файл: " << filePath << std::endl;
        return;
    }

    std::string line;
    while (std::getline(file, line)) {
        if (line.find("require") != std::string::npos) {
            size_t start = line.find("'") + 1; // Поиск начала пути к файлу
            size_t end = line.find("'", start); // Поиск конца пути к файлу
            if (start < end && start != std::string::npos && end != std::string::npos) {
                std::string dependencyPath = line.substr(start, end - start);
                fileContent.addDependency(dependencyPath);
            }
        } else {
            fileContent.addContent(line);
        }
    }
}
