#include <cpprest/http_listener.h>
#include <cpprest/json.h>
#include <iostream>
#include <sqlite3.h>
#include <algorithm>
#include <vector>

using namespace web;
using namespace web::http;
using namespace web::http::experimental::listener;

sqlite3 *db;
char *errMessage = 0;

bool initDB(const std::string &dbPath) {
    if (sqlite3_open(dbPath.c_str(), &db) != SQLITE_OK) {
        std::cerr << "Error opening SQLite DB: " << sqlite3_errmsg(db) << std::endl;
        return false;
    }
    return true;
}

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

bool isValidInput(const std::string &input) {
    return input.find("'") == std::string::npos;
}

void handle_get(http_request request) {
    std::cout << "GET request received" << std::endl;

    std::string sql = "SELECT id, first_name, last_name, middle_name, birth_date, group_name FROM students";
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
        student[U("id")] = json::value::number(sqlite3_column_int(stmt, 0));
        student[U("first_name")] = json::value::string(U(reinterpret_cast<const char *>(sqlite3_column_text(stmt, 1))));
        student[U("last_name")] = json::value::string(U(reinterpret_cast<const char *>(sqlite3_column_text(stmt, 2))));
        student[U("middle_name")] = json::value::string(U(reinterpret_cast<const char *>(sqlite3_column_text(stmt, 3))));
        student[U("birth_date")] = json::value::string(U(reinterpret_cast<const char *>(sqlite3_column_text(stmt, 4))));
        student[U("group_name")] = json::value::string(U(reinterpret_cast<const char *>(sqlite3_column_text(stmt, 5))));
        students[index++] = student;
    }

    sqlite3_finalize(stmt);

    http_response response(status_codes::OK);
    response.headers().add(U("Access-Control-Allow-Origin"), U("*"));
    response.set_body(students);
    request.reply(response);
}


void handle_post(http_request request) {
    request.extract_json().then([=](json::value requestJson) {
      std::string firstName = requestJson[U("first_name")].as_string();
      std::string lastName = requestJson[U("last_name")].as_string();
      std::string middleName = requestJson[U("middle_name")].as_string();
      std::string birthDate = requestJson[U("birth_date")].as_string();
      std::string groupName = requestJson[U("group_name")].as_string();

      if (!isValidInput(firstName) || !isValidInput(lastName) || !isValidInput(middleName) || !isValidInput(birthDate)
          || !isValidInput(groupName)) {
          http_response response(status_codes::BadRequest);
          response.headers().add(U("Access-Control-Allow-Origin"), U("*"));
          response.set_body(U("Invalid input data"));
          request.reply(response);
          return;
      }

      std::string sql =
          "INSERT INTO students (first_name, last_name, middle_name, birth_date, group_name) VALUES (?, ?, ?, ?, ?)";
      std::vector<std::string> params = {firstName, lastName, middleName, birthDate, groupName};

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
    }).wait();
}

void handle_delete(http_request request) {
    auto path = uri::split_path(uri::decode(request.relative_uri().path()));
    if (path.size() == 2 && path[0] == "students") {
        std::string studentId = path[1];

        if (!std::all_of(studentId.begin(), studentId.end(), ::isdigit)) {
            http_response response(status_codes::BadRequest);
            response.headers().add(U("Access-Control-Allow-Origin"), U("*"));
            response.set_body(U("Invalid student ID"));
            request.reply(response);
            return;
        }

        // Проверка, существует ли студент с таким ID
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
            // Студент с таким ID не найден
            http_response response(status_codes::NotFound);
            response.headers().add(U("Access-Control-Allow-Origin"), U("*"));
            response.set_body(U("Student not found"));
            request.reply(response);
            sqlite3_finalize(stmt);
            return;
        }
        sqlite3_finalize(stmt);

        // Продолжение удаления студента, если он существует
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



void closeDB() {
    sqlite3_close(db);
}

void handle_options(http_request request)
{
    http_response response(status_codes::OK);
    response.headers().add(U("Allow"), U("GET, POST, OPTIONS, DELETE"));
    response.headers().add(U("Access-Control-Allow-Origin"), U("*"));
    response.headers().add(U("Access-Control-Allow-Methods"), U("GET, POST, OPTIONS, DELETE"));
    response.headers().add(U("Access-Control-Allow-Headers"), U("Content-Type"));
    request.reply(response);
}




int main() {
    std::string dbPath = "StudentsDB.sqlite";


    if (!initDB(dbPath)) {
        std::cerr << "Failed to open database." << std::endl;
        return 1;
    }

    http_listener listener(U("http://localhost:6502"));

    listener.support(methods::GET, handle_get);
    listener.support(methods::POST, handle_post);
    listener.support(methods::DEL, handle_delete);
    listener.support(methods::OPTIONS, handle_options);

    try {
        listener.open().then([&listener]() { std::cout << "Server is listening on port 6502...\n"; }).wait();
        std::cout << "Press Enter to exit." << std::endl;
        std::string dummy;
        std::getline(std::cin, dummy);
        listener.close().wait();
    }
    catch (std::exception const &e) {
        std::cout << e.what() << std::endl;
    }

    closeDB();

    return 0;
}
