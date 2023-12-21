#pragma once
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <stack>

class DependencyGraph {
private:
    std::unordered_map<std::string, std::vector<std::string>> adjList;

public:
    void addDependency(const std::string &from, const std::string &to);
    const std::vector<std::string> &getDependencies(const std::string &file) const;
    bool contains(const std::string &file) const;
    bool dfs(const std::string &file, std::unordered_set<std::string> &visited,
             std::unordered_set<std::string> &recStack, std::stack<std::string> &sortStack,
             std::vector<std::string> path, const std::string &rootPathStr);
    bool topologicalSort(std::vector<std::string> &sortedFiles, const std::string &rootPathStr);
};
