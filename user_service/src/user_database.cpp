#include "user_database.hpp"

#include <iostream>
#include <cstdlib>

#include <filesystem>

UserDatabase::UserDatabase(const std::string& path) : db_path(path) 
{
    size_t pos = db_path.find_last_of('/');
    if (pos != std::string::npos) 
    {
        std::string dir = db_path.substr(0, pos);
        std::filesystem::create_directories(dir);
    }
    
    std::cout << "   User Database path: " << db_path << std::endl;

    int rc = sqlite3_open(db_path.c_str(), &db);
    if (rc != SQLITE_OK) 
    {
        std::cerr << "   Can't open database: " << sqlite3_errmsg(db) << std::endl;
        return;
    }

    const char* sql_create = 
        "CREATE TABLE IF NOT EXISTS users ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "name TEXT NOT NULL,"
        "email TEXT NOT NULL UNIQUE,"
        "created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP"
        ");";
    
    char* errMsg = nullptr;
    rc = sqlite3_exec(db, sql_create, nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK) 
    {
        std::cerr << "   SQL error: " << errMsg << std::endl;
        sqlite3_free(errMsg);
    } 
    else 
    {
        std::cout << "   Users table ready" << std::endl;
    }
}

UserDatabase::~UserDatabase() 
{
    if (db) 
    {
        sqlite3_close(db);
        std::cout << "   Database closed" << std::endl;
    }
}

int UserDatabase::addUser(const std::string& name, const std::string& email) 
{
    const char* sql_insert = "INSERT INTO users (name, email) VALUES (?, ?);";
    sqlite3_stmt* stmt;
    
    int rc = sqlite3_prepare_v2(db, sql_insert, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) 
    {
        std::cerr << "   Prepare error: " << sqlite3_errmsg(db) << std::endl;

        return -1;
    }
    
    sqlite3_bind_text(stmt, 1, name.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, email.c_str(), -1, SQLITE_STATIC);
    
    rc = sqlite3_step(stmt);
    int newId = -1;
    if (rc == SQLITE_DONE) 
    {
        newId = sqlite3_last_insert_rowid(db);
        std::cout << "   User created with ID: " << newId << std::endl;
    } 
    else 
    {
        std::cerr << "   Insert error: " << sqlite3_errmsg(db) << std::endl;
    }
    
    sqlite3_finalize(stmt);
    
    return newId;
}

User UserDatabase::getUser(int id) 
{
    User user;
    user.id = -1;
    
    const char* sql_select = "SELECT id, name, email, created_at FROM users WHERE id = ?;";
    sqlite3_stmt* stmt;
    
    int rc = sqlite3_prepare_v2(db, sql_select, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) 
    {
        std::cerr << "   Prepare error: " << sqlite3_errmsg(db) << std::endl;

        return user;
    }
    
    sqlite3_bind_int(stmt, 1, id);
    
    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) 
    {
        user.id = sqlite3_column_int(stmt, 0);
        user.name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        user.email = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        user.created_at = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
    }
    
    sqlite3_finalize(stmt);
    return user;
}

User UserDatabase::getUserByEmail(const std::string& email) 
{
    User user;
    user.id = -1;
    
    const char* sql_select = "SELECT id, name, email, created_at FROM users WHERE email = ?;";
    sqlite3_stmt* stmt;
    
    int rc = sqlite3_prepare_v2(db, sql_select, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) 
    {
        std::cerr << "   Prepare error: " << sqlite3_errmsg(db) << std::endl;
        return user;
    }
    
    sqlite3_bind_text(stmt, 1, email.c_str(), -1, SQLITE_STATIC);
    
    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) 
    {
        user.id = sqlite3_column_int(stmt, 0);
        user.name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        user.email = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        user.created_at = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
    }
    
    sqlite3_finalize(stmt);
    return user;
}

std::vector<User> UserDatabase::getAllUsers() 
{
    std::vector<User> users;
    
    const char* sql_select = "SELECT id, name, email, created_at FROM users ORDER BY id DESC;";
    sqlite3_stmt* stmt;
    
    int rc = sqlite3_prepare_v2(db, sql_select, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) 
    {
        std::cerr << "   Prepare error: " << sqlite3_errmsg(db) << std::endl;
        return users;
    }
    
    while (sqlite3_step(stmt) == SQLITE_ROW) 
    {
        User user;
        user.id = sqlite3_column_int(stmt, 0);
        user.name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        user.email = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        user.created_at = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        users.push_back(user);
    }
    
    sqlite3_finalize(stmt);

    return users;
}

bool UserDatabase::updateUser(int id, const std::string& name, const std::string& email) 
{
    const char* sql_update = "UPDATE users SET name = ?, email = ? WHERE id = ?;";
    sqlite3_stmt* stmt;
    
    int rc = sqlite3_prepare_v2(db, sql_update, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) return false;
    
    sqlite3_bind_text(stmt, 1, name.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, email.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 3, id);
    
    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    
    if (rc == SQLITE_DONE) 
    {
        std::cout << "   User " << id << " updated" << std::endl;

        return true;
    }

    return false;
}

bool UserDatabase::deleteUser(int id) 
{
    const char* sql_delete = "DELETE FROM users WHERE id = ?;";
    sqlite3_stmt* stmt;
    
    int rc = sqlite3_prepare_v2(db, sql_delete, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) 
    {
        return false;
    }
    
    sqlite3_bind_int(stmt, 1, id);
    
    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    
    if (rc == SQLITE_DONE) 
    {
        std::cout << "   User " << id << " deleted" << std::endl;

        return true;
    }

    return false;
}