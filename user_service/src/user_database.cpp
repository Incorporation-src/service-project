#include "user_database.hpp"

#include <iostream>
#include <cstdlib>

#include <filesystem>

UserDatabase::UserDatabase() {
    std::string base_path = "../data";

    std::filesystem::create_directories(base_path);

    db_path = base_path + "/users.db";
    
    std::cout << "Database path: " << db_path << std::endl;
    
    int rc = sqlite3_open(db_path.c_str(), &db);
    if (rc) {
        std::cerr << "Can't open database: " << sqlite3_errmsg(db) << std::endl;
        return;
    }
    
    const char* sql_create = 
        "CREATE TABLE IF NOT EXISTS users ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "name TEXT NOT NULL,"
        "email TEXT NOT NULL UNIQUE"
        ");";
    
    char* errMsg = nullptr;
    rc = sqlite3_exec(db, sql_create, nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK) {
        std::cerr << "SQL error: " << errMsg << std::endl;
        sqlite3_free(errMsg);
    }
}

int UserDatabase::addUser(const std::string &name, const std::string &email)
{
    const char* sql_insert = "INSERT INTO users (name, email) VALUES (?, ?);";
    
    sqlite3_stmt* stmt;
    
    int rc = sqlite3_prepare_v2(db, sql_insert, -1, &stmt, nullptr);
    if(rc != SQLITE_OK) 
    {
        return -1;
    }
        
    sqlite3_bind_text(stmt, 1, name.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, email.c_str(), -1, SQLITE_STATIC);
        
    rc = sqlite3_step(stmt);
        
    int newId = -1;
    if(rc == SQLITE_DONE) 
    {
        newId = sqlite3_last_insert_rowid(db);
    }
        
    sqlite3_finalize(stmt);
    return newId;
}

User UserDatabase::getUser(int id) {
    User user;
    user.id = -1;
        
    const char *sql_select = "SELECT id, name, email FROM users WHERE id = ?;";
    sqlite3_stmt *stmt;
        
    int rc = sqlite3_prepare_v2(db, sql_select, -1, &stmt, nullptr);
    if(rc != SQLITE_OK)
    {
        return user;
    }

    sqlite3_bind_int(stmt, 1, id);
        
    rc = sqlite3_step(stmt);
    if(rc == SQLITE_ROW) 
    {
        user.id = sqlite3_column_int(stmt, 0);
        user.name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        user.email = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
    }
        
    sqlite3_finalize(stmt);
        
    return user;
}

std::vector<User> UserDatabase::getAllUsers() {
    std::vector<User> users;
        
    const char* sql_select = "SELECT id, name, email FROM users ORDER BY id;";
    sqlite3_stmt* stmt;
        
    int rc = sqlite3_prepare_v2(db, sql_select, -1, &stmt, nullptr);
    if(rc != SQLITE_OK)
    {
        return users;
    }

    while(sqlite3_step(stmt) == SQLITE_ROW)
    {
        User user;
        user.id = sqlite3_column_int(stmt, 0);
        user.name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        user.email = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        users.push_back(user);
    }
        
    sqlite3_finalize(stmt);
        
    return users;
}

UserDatabase::~UserDatabase()
{
    sqlite3_close(db);
}