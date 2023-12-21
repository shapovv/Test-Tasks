#pragma once
#include <string>
#include <vector>

/* Класс FileContent предназначен для хранения содержимого текстового файла
 * и списка его зависимостей. Это может включать как текст файла, так и пути к другим файлам,
 * на которые он ссылается.*/

class FileContent {
public:
    std::string content;                    // Содержимое файла
    std::vector<std::string> dependencies;  // Список зависимостей файла

    // Добавляет зависимость в список зависимостей файла.
    void addDependency(const std::string &dep);

    // Добавляет строку текста к содержимому файла.
    void addContent(const std::string &line);
};
