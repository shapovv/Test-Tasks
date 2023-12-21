#pragma once
#include <filesystem>
#include <string>
#include "FileContent.h"

namespace fs = std::filesystem;
/*Функция для чтения содержимого текстового файла и извлечения его зависимостей.
* Файлы могут содержать директивы `require`, указывающие на зависимости от других файлов.
* @param filePath: путь к файлу, который необходимо прочитать.
* @param fileContent: объект FileContent, в который будет записано содержимое файла и извлеченные зависимости.*/
void readFile(const fs::path &filePath, FileContent &fileContent);
