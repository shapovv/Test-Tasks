cmake_minimum_required(VERSION 3.21)
project(StudentsDB)

set(CMAKE_CXX_STANDARD 20)

add_executable(StudentsDB main.cpp)

find_package(cpprestsdk REQUIRED)
target_link_libraries(StudentsDB PRIVATE cpprestsdk::cpprest)

find_package(SQLite3 REQUIRED)
include_directories(${SQLite3_INCLUDE_DIRS})
target_link_libraries(StudentsDB PRIVATE ${SQLite3_LIBRARIES})

find_package(OpenSSL REQUIRED)


target_include_directories(StudentsDB PRIVATE ${OPENSSL_INCLUDE_DIR})
target_link_libraries(StudentsDB PRIVATE ${OPENSSL_LIBRARIES})


