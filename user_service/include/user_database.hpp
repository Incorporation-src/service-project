#ifndef USERSERVICE_INCLUDE_DATABASE_HPP
#define USERSERVICE_INCLUDE_DATABASE_HPP

#include <sqlite3.h>
#include <string>
#include <vector>

struct User {
    int id;
    std::string name;
    std::string email;
};

class UserDatabase 
{
public:
    UserDatabase();

    ~UserDatabase();

    int addUser(const std::string &name, const std::string &email);

    User getUser(int id);

    std::vector<User> getAllUsers();

private:
    sqlite3 *db;
    std::string db_path = "db/users.db";
};

#endif // USERSERVICE_INCLUDE_DATABASE_HPP