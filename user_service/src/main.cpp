#include "crow.h"
#include "user_database.hpp"

#include <memory>
#include <iostream>

int main() {
    crow::SimpleApp app;
    
    std::cout << "Starting User Service on port 8081..." << std::endl;

    auto db = std::make_shared<UserDatabase>();
    
    CROW_ROUTE(app, "/")([](){
        return "User Service is running!";
    });
    
    CROW_ROUTE(app, "/status")([](){
        crow::json::wvalue result;
        result["status"] = "running";
        result["service"] = "user-service";
        result["version"] = 1.0;
        return result;
    });
    
    CROW_ROUTE(app, "/users").methods("POST"_method)
    ([db](const crow::request& req){
        std::cout << "POST /users called" << std::endl;
        
        auto x = crow::json::load(req.body);
        if (!x) {
            return crow::response(400, "Invalid JSON");
        }
        
        std::string name = x["name"].s();
        std::string email = x["email"].s();
        
        if (name.empty() || email.empty()) {
            return crow::response(400, "Name and email are required");
        }
        
        int newId = db->addUser(name, email);
        if (newId > 0) {
            crow::json::wvalue result;
            result["id"] = newId;
            result["name"] = name;
            result["email"] = email;
            result["message"] = "User created successfully";
            return crow::response(201, result);
        } else {
            return crow::response(500, "Failed to create user");
        }
    });
    
    CROW_ROUTE(app, "/users/<int>")
    ([db](int user_id){
        std::cout << "GET /users/" << user_id << " called" << std::endl;
        
        User user = db->getUser(user_id);
        
        if (user.id == -1) {
            return crow::response(404, "User not found");
        }
        
        crow::json::wvalue result;
        result["id"] = user.id;
        result["name"] = user.name;
        result["email"] = user.email;
        return crow::response(result);
    });

    CROW_ROUTE(app, "/users")
    ([db](){
        std::cout << "GET /users called" << std::endl;
        
        std::vector<User> users = db->getAllUsers();
        
        crow::json::wvalue result;
        std::vector<crow::json::wvalue> users_json;
        
        for (const auto& user : users) {
            crow::json::wvalue u;
            u["id"] = user.id;
            u["name"] = user.name;
            u["email"] = user.email;
            users_json.push_back(std::move(u));
        }
        
        result["users"] = std::move(users_json);
        result["count"] = users.size();
        return crow::response(result);
    });
    
    std::cout << "   Registered routes:" << std::endl;
    std::cout << "   GET  /" << std::endl;
    std::cout << "   GET  /status" << std::endl;
    std::cout << "   POST /users" << std::endl;
    std::cout << "   GET  /users/<int>" << std::endl;
    std::cout << "   GET  /users" << std::endl;

    std::cout << "Server listening on port 8081" << std::endl;
    
    app.port(8081).multithreaded().run();
}