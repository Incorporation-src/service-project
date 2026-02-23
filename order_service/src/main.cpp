#include "crow.h"
#include "order_database.hpp"

#include <memory>
#include <iostream>
#include <string>

int main() {
    crow::SimpleApp app;
    
    std::cout << "   Starting Order Service on port 8082..." << std::endl;

    auto db = std::make_shared<OrderDatabase>();
    
    CROW_ROUTE(app, "/")([](){
        return "Order Service is running!";
    });
    
    CROW_ROUTE(app, "/status")([](){
        crow::json::wvalue result;
        result["status"] = "running";
        result["service"] = "order-service";
        result["version"] = 1.0;
        return crow::response(result);
    });
    
    CROW_ROUTE(app, "/orders").methods("POST"_method)
    ([db](const crow::request& req){
        std::cout << "   POST /orders called" << std::endl;
        
        auto x = crow::json::load(req.body);
        if (!x) {
            return crow::response(400, "Invalid JSON");
        }
        
        // Проверяем обязательные поля
        if (!x.has("user_id") || !x.has("title") || !x.has("amount")) {
            return crow::response(400, "Missing required fields: user_id, title, amount");
        }
        
        int user_id = x["user_id"].i();
        std::string title = x["title"].s();
        double amount = x["amount"].d();
        
        if (user_id <= 0 || title.empty() || amount <= 0) {
            return crow::response(400, "Invalid data: user_id must be >0, title not empty, amount >0");
        }
        
        int newId = db->addOrder(user_id, title, amount);
        if (newId > 0) {
            crow::json::wvalue result;
            result["id"] = newId;
            result["user_id"] = user_id;
            result["title"] = title;
            result["amount"] = amount;
            result["status"] = "new";
            result["message"] = "Order created successfully";
            return crow::response(201, result);
        } else {
            return crow::response(500, "Failed to create order");
        }
    });

    CROW_ROUTE(app, "/orders/<int>")
    ([db](int order_id){
        std::cout << "   GET /orders/" << order_id << " called" << std::endl;
        
        Order order = db->getOrder(order_id);
        
        if (order.id == -1) {
            return crow::response(404, "Order not found");
        }
        
        crow::json::wvalue result;
        result["id"] = order.id;
        result["user_id"] = order.user_id;
        result["title"] = order.title;
        result["status"] = order.status;
        result["amount"] = order.amount;
        return crow::response(result);
    });

    CROW_ROUTE(app, "/orders/user/<int>")
    ([db](int user_id){
        std::cout << "   GET /orders/user/" << user_id << " called" << std::endl;
        
        std::vector<Order> orders = db->getOrdersByUser(user_id);
        
        crow::json::wvalue result;
        std::vector<crow::json::wvalue> orders_json;
        
        for (const auto& order : orders) {
            crow::json::wvalue o;
            o["id"] = order.id;
            o["title"] = order.title;
            o["status"] = order.status;
            o["amount"] = order.amount;
            orders_json.push_back(std::move(o));
        }
        
        result["user_id"] = user_id;
        result["orders"] = std::move(orders_json);
        result["count"] = orders.size();
        return crow::response(result);
    });

    CROW_ROUTE(app, "/orders")
    ([db](){
        std::cout << "   GET /orders called" << std::endl;
        
        std::vector<Order> orders = db->getAllOrders();
        
        crow::json::wvalue result;
        std::vector<crow::json::wvalue> orders_json;
        
        for (const auto& order : orders) {
            crow::json::wvalue o;
            o["id"] = order.id;
            o["user_id"] = order.user_id;
            o["title"] = order.title;
            o["status"] = order.status;
            o["amount"] = order.amount;
            orders_json.push_back(std::move(o));
        }
        
        result["orders"] = std::move(orders_json);
        result["count"] = orders.size();
        return crow::response(result);
    });

    CROW_ROUTE(app, "/orders/<int>/status").methods("PUT"_method)
    ([db](const crow::request& req, int order_id){
        std::cout << "   PUT /orders/" << order_id << "/status called" << std::endl;
        
        auto x = crow::json::load(req.body);
        if (!x || !x.has("status")) {
            return crow::response(400, "Missing status field");
        }
        
        std::string status = x["status"].s();
        if (status.empty()) {
            return crow::response(400, "Status cannot be empty");
        }
        
        if (db->updateOrderStatus(order_id, status)) {
            crow::json::wvalue result;
            result["id"] = order_id;
            result["status"] = status;
            result["message"] = "Status updated";
            return crow::response(result);
        } else {
            return crow::response(404, "Order not found");
        }
    });

    CROW_ROUTE(app, "/orders/<int>").methods("DELETE"_method)
    ([db](int order_id){
        std::cout << "   DELETE /orders/" << order_id << " called" << std::endl;
        
        if (db->deleteOrder(order_id)) {
            crow::json::wvalue result;
            result["message"] = "Order deleted";
            result["id"] = order_id;
            return crow::response(result);
        } else {
            return crow::response(404, "Order not found");
        }
    });
    
    std::cout << "   Registered routes:" << std::endl;
    std::cout << "   GET  /" << std::endl;
    std::cout << "   GET  /status" << std::endl;
    std::cout << "   POST /orders" << std::endl;
    std::cout << "   GET  /orders/<id>" << std::endl;
    std::cout << "   GET  /orders/user/<user_id>" << std::endl;
    std::cout << "   GET  /orders" << std::endl;
    std::cout << "   PUT  /orders/<id>/status" << std::endl;
    std::cout << "   DELETE /orders/<id>" << std::endl;
    
    std::cout << "   Server listening on port 8082" << std::endl;
    
    app.port(8082).multithreaded().run();
}