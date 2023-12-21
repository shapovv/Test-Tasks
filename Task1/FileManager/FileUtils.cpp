#include "FileUtils.h"
#include <fstream>
#include <iostream>

void readFile(const fs::path &filePath, FileContent &fileContent) {
    std::ifstream file(filePath);

    // Проверка успешности открытия файла
    if (!file.is_open()) {
        std::cerr << "Не удалось открыть файл: " << filePath << std::endl;
        return;
    }

    std::string line;
    // Чтение файла построчно
    while (std::getline(file, line)) {
        // Проверка на наличие директивы 'require' в строке
        if (line.find("require") != std::string::npos) {
            // Извлечение пути к зависимому файлу из директивы
            size_t start = line.find("'") + 1;   // Поиск начала пути к файлу
            size_t end = line.find("'", start);  // Поиск конца пути к файлу
            if (start < end && start != std::string::npos && end != std::string::npos) {
                std::string dependencyPath = line.substr(start, end - start);
                fileContent.addDependency(dependencyPath);
            }
        } else {
            // Добавление строки к содержимому файла
            fileContent.addContent(line);
        }
    }
}
