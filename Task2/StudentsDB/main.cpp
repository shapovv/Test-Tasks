#include <cpprest/http_listener.h>
#include <cpprest/json.h>
#include <iostream>
#include <sqlite3.h>
#include <algorithm>
#include <vector>

// Использование пространств имен для упрощения кода
using namespace web;
using namespace web::http;
using namespace web::http::experimental::listener;

sqlite3 *db;  // Глобальный указатель на объект базы данных SQLite
char *errMessage = 0;  // Глобальная переменная для сообщений об ошибках SQLite

// Функция для инициализации и открытия базы данных
bool initDB(const std::string &dbPath) {
    if (sqlite3_open(dbPath.c_str(), &db) != SQLITE_OK) {
        std::cerr << "Error opening SQLite DB: " << sqlite3_errmsg(db) << std::endl;
        return false;
    }
    return true;
}

// Функция для выполнения SQL-запросов с параметрами
bool executeSQL(const std::string &sql, const std::vector<std::string> &params = {}) {
    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, NULL) != SQLITE_OK) {
        std::cerr << "SQL error: " << sqlite3_errmsg(db) << std::endl;
        return false;
    }

    for (size_t i = 0; i < params.size(); ++i) {
        sqlite3_bind_text(stmt, i + 1, params[i].c_str(), -1, SQLITE_TRANSIENT);
    }

    if (sqlite3_step(stmt) != SQLITE_DONE) {
        std::cerr << "SQL error: " << sqlite3_errmsg(db) << std::endl;
        sqlite3_finalize(stmt);
        return false;
    }

    sqlite3_finalize(stmt);
    return true;
}

// Валидация входных данных для предотвращения SQL инъекций
bool isValidInput(const std::string &input) {
    return input.find("'") == std::string::npos;
}

// Обработчик GET-запросов для получения списка студентов
void handle_get(http_request request) {
    std::cout << "GET request received" << std::endl;

    std::string sql =
        "SELECT id, first_name, last_name, middle_name, birth_date, group_name FROM students";
    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, NULL) != SQLITE_OK) {
        std::cerr << "Failed to prepare statement" << std::endl;

        http_response response(status_codes::InternalError);
        response.headers().add(U("Access-Control-Allow-Origin"), U("*"));
        response.set_body(U("Error in database operation"));
        request.reply(response);

        return;
    }

    json::value students = json::value::array();
    int index = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        json::value student = json::value::object();
        // Поочередное извлечение данных о студенте и добавление их в JSON-объект
        student[U("id")] = json::value::number(sqlite3_column_int(stmt, 0));
        student[U("first_name")] =
            json::value::string(U(reinterpret_cast<const char *>(sqlite3_column_text(stmt, 1))));
        student[U("last_name")] =
            json::value::string(U(reinterpret_cast<const char *>(sqlite3_column_text(stmt, 2))));
        student[U("middle_name")] =
            json::value::string(U(reinterpret_cast<const char *>(sqlite3_column_text(stmt, 3))));
        student[U("birth_date")] =
            json::value::string(U(reinterpret_cast<const char *>(sqlite3_column_text(stmt, 4))));
        student[U("group_name")] =
            json::value::string(U(reinterpret_cast<const char *>(sqlite3_column_text(stmt, 5))));
        students[index++] = student;
    }

    sqlite3_finalize(stmt);

    http_response response(status_codes::OK);
    response.headers().add(U("Access-Control-Allow-Origin"), U("*"));
    response.set_body(students);  // Отправка списка студентов клиенту
    request.reply(response);
}

// Обработчик POST-запросов для добавления нового студента
void handle_post(http_request request) {
    request.extract_json()
        .then([=](json::value requestJson) {
            // Извлечение данных о студенте из JSON-запроса
            std::string firstName = requestJson[U("first_name")].as_string();
            std::string lastName = requestJson[U("last_name")].as_string();
            std::string middleName = requestJson[U("middle_name")].as_string();
            std::string birthDate = requestJson[U("birth_date")].as_string();
            std::string groupName = requestJson[U("group_name")].as_string();

            // Проверка валидности входных данных
            if (!isValidInput(firstName) || !isValidInput(lastName) || !isValidInput(middleName) ||
                !isValidInput(birthDate) || !isValidInput(groupName)) {
                http_response response(status_codes::BadRequest);
                response.headers().add(U("Access-Control-Allow-Origin"), U("*"));
                response.set_body(U("Invalid input data"));
                request.reply(response);
                return;
            }

            std::string sql =
                "INSERT INTO students (first_name, last_name, middle_name, birth_date, group_name) "
                "VALUES (?, ?, ?, ?, ?)";
            std::vector<std::string> params = {firstName, lastName, middleName, birthDate,
                                               groupName};

            // Выполнение SQL-запроса на добавление студента
            if (!executeSQL(sql, params)) {
                http_response response(status_codes::InternalError);
                response.headers().add(U("Access-Control-Allow-Origin"), U("*"));
                response.set_body(U("Failed to insert student"));
                request.reply(response);
                return;
            }

            std::cout << "POST request received: " << requestJson.serialize() << std::endl;
            http_response response(status_codes::OK);
            response.headers().add(U("Access-Control-Allow-Origin"), U("*"));
            response.set_body(U("Student added successfully"));
            request.reply(response);
        })
        .wait();
}

// Обработчик DELETE-запросов для удаления студента
void handle_delete(http_request request) {
    auto path = uri::split_path(uri::decode(request.relative_uri().path()));
    if (path.size() == 2 && path[0] == "students") {
        std::string studentId = path[1];

        // Проверка, является ли ID студента числом
        if (!std::all_of(studentId.begin(), studentId.end(), ::isdigit)) {
            http_response response(status_codes::BadRequest);
            response.headers().add(U("Access-Control-Allow-Origin"), U("*"));
            response.set_body(U("Invalid student ID"));
            request.reply(response);
            return;
        }

        // Проверка существования студента в базе данных
        std::string checkSql = "SELECT COUNT(*) FROM students WHERE id = ?";
        sqlite3_stmt *stmt;
        if (sqlite3_prepare_v2(db, checkSql.c_str(), -1, &stmt, NULL) != SQLITE_OK) {
            http_response response(status_codes::InternalError);
            response.headers().add(U("Access-Control-Allow-Origin"), U("*"));
            response.set_body(U("Error in database operation"));
            request.reply(response);
            return;
        }
        sqlite3_bind_text(stmt, 1, studentId.c_str(), -1, SQLITE_TRANSIENT);

        if (sqlite3_step(stmt) != SQLITE_ROW || sqlite3_column_int(stmt, 0) == 0) {
            // Если студент не найден, отправляем соответствующий ответ
            http_response response(status_codes::NotFound);
            response.headers().add(U("Access-Control-Allow-Origin"), U("*"));
            response.set_body(U("Student not found"));
            request.reply(response);
            sqlite3_finalize(stmt);
            return;
        }
        sqlite3_finalize(stmt);

        // Выполнение SQL-запроса на удаление студента
        std::string sql = "DELETE FROM students WHERE id = ?";
        if (!executeSQL(sql, {studentId})) {
            http_response response(status_codes::InternalError);
            response.headers().add(U("Access-Control-Allow-Origin"), U("*"));
            response.set_body(U("Failed to delete student"));
            request.reply(response);
            return;
        }

        http_response response(status_codes::OK);
        response.headers().add(U("Access-Control-Allow-Origin"), U("*"));
        response.set_body(U("Student deleted"));
        request.reply(response);
    } else {
        http_response response(status_codes::BadRequest);
        response.headers().add(U("Access-Control-Allow-Origin"), U("*"));
        response.set_body(U("Invalid request"));
        request.reply(response);
    }
}

// Функция закрытия базы данных
void closeDB() {
    sqlite3_close(db);
}

// Обработчик OPTIONS запросов, необходимый для работы CORS
void handle_options(http_request request) {
    http_response response(status_codes::OK);
    response.headers().add(U("Allow"), U("GET, POST, OPTIONS, DELETE"));
    response.headers().add(U("Access-Control-Allow-Origin"), U("*"));
    response.headers().add(U("Access-Control-Allow-Methods"), U("GET, POST, OPTIONS, DELETE"));
    response.headers().add(U("Access-Control-Allow-Headers"), U("Content-Type"));
    request.reply(response);
}

// Основная функция для инициализации и запуска HTTP-сервера
int main() {
    std::string dbPath = "StudentsDB.sqlite";

    // Инициализация и открытие базы данных
    if (!initDB(dbPath)) {
        std::cerr << "Failed to open database." << std::endl;
        return 1;
    }

    // Настройка HTTP-слушателя (listener) для обработки входящих запросов
    http_listener listener(U("http://localhost:6502"));

    // Регистрация обработчиков для различных типов HTTP-запросов
    listener.support(methods::GET, handle_get);
    listener.support(methods::POST, handle_post);
    listener.support(methods::DEL, handle_delete);
    listener.support(methods::OPTIONS, handle_options);

    try {
        // Запуск слушателя и ожидание входящих запросов
        listener.open()
            .then([&listener]() { std::cout << "Server is listening on port 6502...\n"; })
            .wait();
        std::cout << "Press Enter to exit." << std::endl;
        std::string dummy;
        std::getline(std::cin, dummy);
        listener.close().wait();
    } catch (std::exception const &e) {
        std::cout << e.what() << std::endl;
    }

    // Закрытие базы данных при завершении работы сервера
    closeDB();

    return 0;
}
