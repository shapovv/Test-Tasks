#pragma once
#include <filesystem>
#include <string>
#include "FileContent.h"

namespace fs = std::filesystem;

void readFile(const fs::path &filePath, FileContent &fileContent);
