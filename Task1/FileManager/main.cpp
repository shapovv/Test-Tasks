#include <iostream>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>
#include <unordered_map>
#include <stack>
#include "DependencyGraph.h"
#include "FileContent.h"
#include "FileUtils.h"

namespace fs = std::filesystem;

int main() {
    fs::path rootPath =
        "root";  // Определение корневого пути для поиска файлов (Из условия задания)
    //    fs::path rootPath = "root1";  //Аналогичный пример без циклических зависимостей
    //    fs::path rootPath = "root2";  //Пример с циклической зависимостью

    DependencyGraph graph;  // Создание графа зависимостей
    std::unordered_map<std::string, FileContent>
        fileContents;  // Словарь для хранения содержимого и зависимостей файлов
    std::string rootPathStr =
        fs::absolute(rootPath).string();  // Получение абсолютного пути корневой директории

    // Обход всех файлов в корневой директории и ее поддиректориях
    for (const auto &entry : fs::recursive_directory_iterator(rootPath)) {
        if (entry.is_regular_file() && entry.path().extension() == ".txt") {
            FileContent fileContent;
            readFile(entry.path(),
                     fileContent);  // Чтение файла и извлечение его содержимого и зависимостей
            std::string absoluteFilePath = fs::absolute(entry.path()).string();
            fileContents[absoluteFilePath] = fileContent;  // Сохранение содержимого файла

            // Добавление зависимостей файла в граф
            for (const auto &dep : fileContent.dependencies) {
                fs::path relativePath(dep);
                fs::path fullPath = rootPath / relativePath;
                std::string absoluteDependencyPath = fs::absolute(fullPath).string();

                if (fullPath.extension() != ".txt") {
                    absoluteDependencyPath += ".txt";
                }

                graph.addDependency(absoluteDependencyPath, absoluteFilePath);
            }
        }
    }

    std::vector<std::string> sortedFiles;
    // Попытка выполнить топологическую сортировку графа
    if (!graph.topologicalSort(sortedFiles, rootPathStr)) {
        std::cout << "Исправьте эту ошибку для успешной конкатенации." << std::endl;
        return 1;
    }

    // Вывод отсортированных файлов
    std::cout << "Отсортированные файлы:" << std::endl;
    for (const auto &file : sortedFiles) {
        std::size_t found = file.find(rootPathStr);
        if (found != std::string::npos) {
            std::string relativePath = file.substr(found + rootPathStr.length() + 1);
            std::size_t dotPosition = relativePath.rfind(".txt");
            if (dotPosition != std::string::npos) {
                relativePath.erase(dotPosition, 4);
            }
            std::cout << relativePath << std::endl;
        } else {
            std::cout << file << std::endl;
        }
    }

    // Создание итогового файла с конкатенацией
    std::ofstream outputFile("result.txt");
    for (const auto &file : sortedFiles) {
        outputFile << fileContents[file].content;
    }

    std::cout << "Файлы успешно склеены." << std::endl;
    return 0;
}
