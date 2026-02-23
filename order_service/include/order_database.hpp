#ifndef ORDERSERVICE_INCLUDE_ORDERDATABASE_HPP
#define ORDERSERVICE_INCLUDE_ORDERDATABASE_HPP

#include <sqlite3.h>
#include <string>
#include <vector>

struct Order {
    int id;
    int user_id;
    std::string title;
    std::string status;
    double amount;
};

class OrderDatabase {
private:
    sqlite3* db;
    std::string db_path;

public:
    OrderDatabase(const std::string& path = "../data/orders.db");
    ~OrderDatabase();
    
    OrderDatabase(const OrderDatabase&) = delete;
    OrderDatabase& operator=(const OrderDatabase&) = delete;
    
    int addOrder(int user_id, const std::string& title, double amount);
    
    Order getOrder(int id);
    std::vector<Order> getOrdersByUser(int user_id);
    std::vector<Order> getAllOrders();
    
    bool updateOrderStatus(int id, const std::string& status);
    bool deleteOrder(int id);
};

#endif // ORDERSERVICE_INCLUDE_ORDERDATABASE_HPP