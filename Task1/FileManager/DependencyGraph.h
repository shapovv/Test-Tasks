#pragma once
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <stack>

// Класс DependencyGraph представляет собой граф зависимостей файлов.
// Он используется для хранения и обработки зависимостей между файлами.
class DependencyGraph {
private:
    std::unordered_map<std::string, std::vector<std::string>>
        adjList;  // Список смежности для представления графа

public:
    // Добавляет зависимость между двумя файлами
    void addDependency(const std::string &from, const std::string &to);

    // Возвращает список зависимостей для данного файла
    const std::vector<std::string> &getDependencies(const std::string &file) const;

    // Проверяет, содержится ли файл в графе
    bool contains(const std::string &file) const;

    // Вспомогательная функция для обнаружения циклов и топологической сортировки (DFS)
    bool dfs(const std::string &file, std::unordered_set<std::string> &visited,
             std::unordered_set<std::string> &recStack, std::stack<std::string> &sortStack,
             std::vector<std::string> path, const std::string &rootPathStr);

    // Выполняет топологическую сортировку графа
    bool topologicalSort(std::vector<std::string> &sortedFiles, const std::string &rootPathStr);
};
