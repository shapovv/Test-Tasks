#include "FileContent.h"

// Добавляет новую зависимость к списку зависимостей файла.
// Это используется при анализе директив `require` в файлах.
void FileContent::addDependency(const std::string &dep) {
    dependencies.push_back(dep);
}

// Добавляет строку к текущему содержимому файла.
// Используется при последовательном чтении файла построчно.
void FileContent::addContent(const std::string &line) {
    content += line + "\n";
}
