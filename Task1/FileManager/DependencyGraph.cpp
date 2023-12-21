#include "DependencyGraph.h"
#include <iostream>

// Добавляет зависимость от одного файла к другому в граф.
// @param from: Путь к файлу, от которого идет зависимость.
// @param to: Путь к файлу, который является зависимостью.
void DependencyGraph::addDependency(const std::string &from, const std::string &to) {
    adjList[from].push_back(to);  // Добавление зависимости в список смежности
}

// Возвращает список зависимостей для заданного файла.
// @param file: Путь к файлу, зависимости которого запрашиваются.
// @return Вектор строк, содержащий пути к зависимым файлам.
const std::vector<std::string> &DependencyGraph::getDependencies(const std::string &file) const {
    return adjList.at(file);
}

// Проверяет наличие файла в графе зависимостей.
// @param file: Путь к файлу для проверки.
// @return true, если файл присутствует в графе, иначе false.
bool DependencyGraph::contains(const std::string &file) const {
    return adjList.find(file) != adjList.end();
}

// Глубинный поиск (DFS) для обнаружения циклов и подготовки к топологической сортировке.
// @param file: Текущий файл для проверки.
// @param visited: Множество посещенных файлов.
// @param recStack: Стек рекурсивного обхода (для обнаружения циклов).
// @param sortStack: Стек для хранения порядка топологической сортировки.
// @param path: Текущий путь обхода (для отображения циклов).
// @param rootPathStr: Корневой путь для преобразования абсолютных путей в относительные.
// @return true, если найден цикл, иначе false.
bool DependencyGraph::dfs(const std::string &file, std::unordered_set<std::string> &visited,
                          std::unordered_set<std::string> &recStack,
                          std::stack<std::string> &sortStack, std::vector<std::string> path,
                          const std::string &rootPathStr) {
    // Если файл уже находится в стеке рекурсии, обнаружен цикл
    if (recStack.find(file) != recStack.end()) {
        path.push_back(file);
        std::cout << "Обнаружена циклическая зависимость:" << std::endl;
        // Вывод пути, составляющего цикл
        for (size_t i = 0; i < path.size(); ++i) {
            if (i > 0) {
                std::cout << " -> ";
            }
            std::size_t found = path[i].find(rootPathStr);
            std::string relativePath = (found != std::string::npos)
                                           ? path[i].substr(found + rootPathStr.length() + 1)
                                           : path[i];
            std::size_t dotPosition = relativePath.rfind(".txt");
            if (dotPosition != std::string::npos) {
                relativePath.erase(dotPosition, 4);
            }
            std::cout << relativePath;
        }
        std::cout << std::endl;
        return true;
    }

    // Добавление файла в список посещенных и стек рекурсии
    if (visited.find(file) == visited.end()) {
        visited.insert(file);
        recStack.insert(file);
        path.push_back(file);

        // Рекурсивный обход всех зависимостей файла
        if (contains(file)) {
            for (const auto &dep : getDependencies(file)) {
                if (dfs(dep, visited, recStack, sortStack, path, rootPathStr)) {
                    return true;
                }
            }
        }

        // Удаление файла из стека рекурсии и добавление в стек сортировки
        recStack.erase(file);
        sortStack.push(file);
    }
    return false;
}

// Выполнение топологической сортировки графа зависимостей.
// @param sortedFiles: Вектор для хранения отсортированных файлов.
// @param rootPathStr: Корневой путь для форматирования вывода.
// @return true, если сортировка выполнена успешно, иначе false (в случае цикла).
bool DependencyGraph::topologicalSort(std::vector<std::string> &sortedFiles,
                                      const std::string &rootPathStr) {
    std::unordered_set<std::string> visited;  // Множество посещенных файлов
    std::unordered_set<std::string> recStack;  // Стек рекурсивного обхода
    std::stack<std::string> sortStack;  // Стек для топологической сортировки
    std::vector<std::string> path;  // Вектор для хранения текущего пути

    // Обход всех узлов графа
    for (const auto &pair : adjList) {
        if (dfs(pair.first, visited, recStack, sortStack, path, rootPathStr)) {
            // Если обнаружен цикл, завершение с ошибкой
            return false;
        }
    }

    // Вывод отсортированных файлов
    while (!sortStack.empty()) {
        sortedFiles.push_back(sortStack.top());
        sortStack.pop();
    }

    return true;  // Успешная топологическая сортировка
}
