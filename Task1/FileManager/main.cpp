#include <iostream>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>
#include <unordered_map>
#include <stack>
#include <unordered_set>

namespace fs = std::filesystem;

class FileContent {
 public:
  std::string content;
  std::vector<std::string> dependencies;

  void addDependency(const std::string &dep) {
      dependencies.push_back(dep);
  }

  void addContent(const std::string &line) {
      content += line + "\n";
  }
};

void readFile(const fs::path &filePath, FileContent &fileContent) {
    std::ifstream file(filePath);
    std::string line;
    while (std::getline(file, line)) {
        if (line.find("require") != std::string::npos) {
            size_t start = line.find("'") + 1; // поиграться с кавычками
            size_t end = line.find("'", start);
            if (start < end && start != std::string::npos && end != std::string::npos) {
                std::string dependencyPath = line.substr(start, end - start);
                fileContent.addDependency(dependencyPath);
            }
        } else {
            fileContent.addContent(line);
        }
    }
}

class DependencyGraph {
 private:
  std::unordered_map<std::string, std::vector<std::string>> adjList;

 public:
  void addDependency(const std::string &from, const std::string &to) {
      adjList[from].push_back(to);
  }

  const std::vector<std::string> &getDependencies(const std::string &file) const {
      return adjList.at(file);
  }

  bool contains(const std::string &file) const {
      return adjList.find(file) != adjList.end();
  }

  bool dfs(const std::string &file,
           std::unordered_set<std::string> &visited,
           std::unordered_set<std::string> &recStack,
           std::stack<std::string> &sortStack,
           std::vector<std::string> path,
           const std::string &rootPathStr) {
      if (recStack.find(file) != recStack.end()) {
          path.push_back(file);
          std::cout << "Обнаружена циклическая зависимость:" << std::endl;
          for (size_t i = 0; i < path.size(); ++i) {
              if (i > 0) {
                  std::cout << " -> ";
              }
              std::size_t found = path[i].find(rootPathStr);
              std::string relativePath =
                  (found != std::string::npos) ? path[i].substr(found + rootPathStr.length() + 1) : path[i];
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

  bool topologicalSort(std::vector<std::string> &sortedFiles, const std::string &rootPathStr) {
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

};

int main() {
    fs::path rootPath = "root";
//    fs::path rootPath = "root1";
//    fs::path rootPath = "root2";

    DependencyGraph graph;
    std::unordered_map<std::string, FileContent> fileContents;
    std::string rootPathStr = fs::absolute(rootPath).string();

    for (const auto &entry : fs::recursive_directory_iterator(rootPath)) {
        if (entry.is_regular_file() && entry.path().extension() == ".txt") {
            FileContent fileContent;
            readFile(entry.path(), fileContent);
            std::string absoluteFilePath = fs::absolute(entry.path()).string();
            fileContents[absoluteFilePath] = fileContent;
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
    if (!graph.topologicalSort(sortedFiles, rootPathStr)) {
        std::cout << "Исправьте эту ошибку для успешной конкатенации." << std::endl;
        return 1;
    }

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

    std::ofstream outputFile("result.txt");
    for (const auto &file : sortedFiles) {
        outputFile << fileContents[file].content;
    }

    std::cout << "Файлы успешно склеены." << std::endl;
    return 0;
}


