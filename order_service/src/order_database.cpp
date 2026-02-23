#include "order_database.hpp"

#include <iostream>
#include <cstdlib>
#include <filesystem>

OrderDatabase::OrderDatabase(const std::string& path) : db_path(path) 
{
    size_t pos = db_path.find_last_of('/');
    if (pos != std::string::npos) 
    {
        std::string dir = db_path.substr(0, pos);
        std::filesystem::create_directories(dir);
    }
    
    std::cout << "   Order Database path: " << db_path << std::endl;

    int rc = sqlite3_open(db_path.c_str(), &db);
    if (rc != SQLITE_OK) 
    {
        std::cerr << "   Can't open database: " << sqlite3_errmsg(db) << std::endl;
        return;
    }

    const char* sql_create = 
        "CREATE TABLE IF NOT EXISTS orders ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "user_id INTEGER NOT NULL,"
        "title TEXT NOT NULL,"
        "status TEXT DEFAULT 'new',"
        "amount REAL NOT NULL,"
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
        std::cout << "   Orders table ready" << std::endl;
    }
}

OrderDatabase::~OrderDatabase() 
{
    if (db) 
    {
        sqlite3_close(db);
        std::cout << "   Database closed" << std::endl;
    }
}

int OrderDatabase::addOrder(int user_id, const std::string& title, double amount) 
{
    const char* sql_insert = "INSERT INTO orders (user_id, title, amount) VALUES (?, ?, ?);";
    sqlite3_stmt* stmt;
    
    int rc = sqlite3_prepare_v2(db, sql_insert, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) 
    {
        std::cerr << "   Prepare error: " << sqlite3_errmsg(db) << std::endl;

        return -1;
    }
    
    sqlite3_bind_int(stmt, 1, user_id);
    sqlite3_bind_text(stmt, 2, title.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_double(stmt, 3, amount);
    
    rc = sqlite3_step(stmt);
    int newId = -1;
    if (rc == SQLITE_DONE) 
    {
        newId = sqlite3_last_insert_rowid(db);
        std::cout << "   Order created with ID: " << newId << std::endl;
    } 
    else 
    {
        std::cerr << "   Insert error: " << sqlite3_errmsg(db) << std::endl;
    }
    
    sqlite3_finalize(stmt);

    return newId;
}

Order OrderDatabase::getOrder(int id) 
{
    Order order;
    order.id = -1;
    
    const char* sql_select = "SELECT id, user_id, title, status, amount FROM orders WHERE id = ?;";
    sqlite3_stmt* stmt;
    
    int rc = sqlite3_prepare_v2(db, sql_select, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) 
    {
        std::cerr << "   Prepare error: " << sqlite3_errmsg(db) << std::endl;
        return order;
    }
    
    sqlite3_bind_int(stmt, 1, id);
    
    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) 
    {
        order.id = sqlite3_column_int(stmt, 0);
        order.user_id = sqlite3_column_int(stmt, 1);
        order.title = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        order.status = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        order.amount = sqlite3_column_double(stmt, 4);
    }
    
    sqlite3_finalize(stmt);

    return order;
}

std::vector<Order> OrderDatabase::getOrdersByUser(int user_id) 
{
    std::vector<Order> orders;
    
    const char* sql_select = "SELECT id, user_id, title, status, amount FROM orders WHERE user_id = ? ORDER BY id DESC;";
    sqlite3_stmt* stmt;
    
    int rc = sqlite3_prepare_v2(db, sql_select, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) 
    {
        std::cerr << "   Prepare error: " << sqlite3_errmsg(db) << std::endl;
        return orders;
    }
    
    sqlite3_bind_int(stmt, 1, user_id);
    
    while (sqlite3_step(stmt) == SQLITE_ROW) 
    {
        Order order;
        order.id = sqlite3_column_int(stmt, 0);
        order.user_id = sqlite3_column_int(stmt, 1);
        order.title = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        order.status = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        order.amount = sqlite3_column_double(stmt, 4);
        orders.push_back(order);
    }
    
    sqlite3_finalize(stmt);

    return orders;
}

std::vector<Order> OrderDatabase::getAllOrders() 
{
    std::vector<Order> orders;
    
    const char* sql_select = "SELECT id, user_id, title, status, amount FROM orders ORDER BY id DESC;";
    sqlite3_stmt* stmt;
    
    int rc = sqlite3_prepare_v2(db, sql_select, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) 
    {
        std::cerr << "   Prepare error: " << sqlite3_errmsg(db) << std::endl;
        return orders;
    }
    
    while (sqlite3_step(stmt) == SQLITE_ROW) 
    {
        Order order;
        order.id = sqlite3_column_int(stmt, 0);
        order.user_id = sqlite3_column_int(stmt, 1);
        order.title = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        order.status = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        order.amount = sqlite3_column_double(stmt, 4);
        orders.push_back(order);
    }
    
    sqlite3_finalize(stmt);

    return orders;
}

bool OrderDatabase::updateOrderStatus(int id, const std::string& status) 
{
    const char* sql_update = "UPDATE orders SET status = ? WHERE id = ?;";
    sqlite3_stmt* stmt;
    
    int rc = sqlite3_prepare_v2(db, sql_update, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) return false;
    
    sqlite3_bind_text(stmt, 1, status.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 2, id);
    
    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    
    return rc == SQLITE_DONE;
}

bool OrderDatabase::deleteOrder(int id) 
{
    const char* sql_delete = "DELETE FROM orders WHERE id = ?;";
    sqlite3_stmt* stmt;
    
    int rc = sqlite3_prepare_v2(db, sql_delete, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) return false;
    
    sqlite3_bind_int(stmt, 1, id);
    
    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    
    return rc == SQLITE_DONE;
}