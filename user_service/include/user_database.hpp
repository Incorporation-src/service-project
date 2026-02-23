#ifndef USERSERVICE_INCLUDE_DATABASE_HPP
#define USERSERVICE_INCLUDE_DATABASE_HPP

#include <sqlite3.h>
#include <string>
#include <vector>

struct User {
    int id;
    std::string name;
    std::string email;
    std::string created_at;
};

class UserDatabase {
private:
    sqlite3* db;
    std::string db_path;

public:
    UserDatabase(const std::string& path = "../data/users.db");
    ~UserDatabase();
    
    UserDatabase(const UserDatabase&) = delete;
    UserDatabase& operator=(const UserDatabase&) = delete;

    int addUser(const std::string& name, const std::string& email);
    User getUser(int id);
    User getUserByEmail(const std::string& email);
    std::vector<User> getAllUsers();
    bool updateUser(int id, const std::string& name, const std::string& email);
    bool deleteUser(int id);
};

#endif // USERSERVICE_INCLUDE_DATABASE_HPP