cmake_minimum_required(VERSION 3.21)
project(FileManager)

set(CMAKE_CXX_STANDARD 20)

add_executable(FileManager main.cpp FileContent.cpp DependencyGraph.cpp FileUtils.cpp)
