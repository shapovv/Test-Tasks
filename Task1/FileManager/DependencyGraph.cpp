#include "DependencyGraph.h"
#include <iostream>

void DependencyGraph::addDependency(const std::string &from, const std::string &to) {
    adjList[from].push_back(to);
}

const std::vector<std::string> &DependencyGraph::getDependencies(const std::string &file) const {
    return adjList.at(file);
}

bool DependencyGraph::contains(const std::string &file) const {
    return adjList.find(file) != adjList.end();
}

bool DependencyGraph::dfs(const std::string &file, std::unordered_set<std::string> &visited,
                          std::unordered_set<std::string> &recStack, std::stack<std::string> &sortStack,
                          std::vector<std::string> path, const std::string &rootPathStr) {
    if (recStack.find(file) != recStack.end()) {
        path.push_back(file);
        std::cout << "Обнаружена циклическая зависимость:" << std::endl;
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

    if (visited.find(file) == visited.end()) {
        visited.insert(file);
        recStack.insert(file);
        path.push_back(file);

        if (contains(file)) {
            for (const auto &dep : getDependencies(file)) {
                if (dfs(dep, visited, recStack, sortStack, path, rootPathStr)) {
                    return true;
                }
            }
        }

        recStack.erase(file);
        sortStack.push(file);
    }
    return false;
}

bool DependencyGraph::topologicalSort(std::vector<std::string> &sortedFiles, const std::string &rootPathStr) {
    std::unordered_set<std::string> visited;
    std::unordered_set<std::string> recStack;
    std::stack<std::string> sortStack;
    std::vector<std::string> path;

    for (const auto &pair : adjList) {
        if (dfs(pair.first, visited, recStack, sortStack, path, rootPathStr)) {
            return false;
        }
    }

    while (!sortStack.empty()) {
        sortedFiles.push_back(sortStack.top());
        sortStack.pop();
    }

    return true;
}
