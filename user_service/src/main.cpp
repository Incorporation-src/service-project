#include "crow.h"
#include "user_database.hpp"

#include <memory>
#include <iostream>
#include <string>

int main() {
    crow::SimpleApp app;
    
    std::cout << "   Starting User Service on port 8081..." << std::endl;

    auto db = std::make_shared<UserDatabase>();

    CROW_ROUTE(app, "/")([](){
        return "User Service is running!";
    });
    
    CROW_ROUTE(app, "/status")([](){
        crow::json::wvalue result;
        result["status"] = "running";
        result["service"] = "user-service";
        result["version"] = 1.0;
        return crow::response(result);
    });
    
    CROW_ROUTE(app, "/users").methods("POST"_method)
    ([db](const crow::request& req){
        std::cout << "   POST /users called" << std::endl;
        
        auto x = crow::json::load(req.body);
        if (!x) {
            return crow::response(400, "Invalid JSON");
        }

        if (!x.has("name") || !x.has("email")) {
            return crow::response(400, "Missing required fields: name, email");
        }
        
        std::string name = x["name"].s();
        std::string email = x["email"].s();
        
        if (name.empty() || email.empty()) {
            return crow::response(400, "Name and email cannot be empty");
        }

        User existing = db->getUserByEmail(email);
        if (existing.id != -1) {
            return crow::response(409, "User with this email already exists");
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
        std::cout << "   GET /users/" << user_id << " called" << std::endl;
        
        User user = db->getUser(user_id);
        
        if (user.id == -1) {
            return crow::response(404, "User not found");
        }
        
        crow::json::wvalue result;
        result["id"] = user.id;
        result["name"] = user.name;
        result["email"] = user.email;
        result["created_at"] = user.created_at;
        return crow::response(result);
    });

    CROW_ROUTE(app, "/users/email")
    ([db](const crow::request& req){
        std::cout << "   GET /users/email called" << std::endl;
        
        std::string email = req.url_params.get("address");
        if (email.empty()) {
            return crow::response(400, "Missing email parameter");
        }
        
        User user = db->getUserByEmail(email);
        
        if (user.id == -1) {
            return crow::response(404, "User not found");
        }
        
        crow::json::wvalue result;
        result["id"] = user.id;
        result["name"] = user.name;
        result["email"] = user.email;
        result["created_at"] = user.created_at;
        return crow::response(result);
    });

    CROW_ROUTE(app, "/users")
    ([db](){
        std::cout << "   GET /users called" << std::endl;
        
        std::vector<User> users = db->getAllUsers();
        
        crow::json::wvalue result;
        std::vector<crow::json::wvalue> users_json;
        
        for (const auto& user : users) {
            crow::json::wvalue u;
            u["id"] = user.id;
            u["name"] = user.name;
            u["email"] = user.email;
            u["created_at"] = user.created_at;
            users_json.push_back(std::move(u));
        }
        
        result["users"] = std::move(users_json);
        result["count"] = users.size();
        return crow::response(result);
    });

    CROW_ROUTE(app, "/users/<int>").methods("PUT"_method)
    ([db](const crow::request& req, int user_id){
        std::cout << "   PUT /users/" << user_id << " called" << std::endl;
        
        auto x = crow::json::load(req.body);
        if (!x) {
            return crow::response(400, "Invalid JSON");
        }
        
        if (!x.has("name") || !x.has("email")) {
            return crow::response(400, "Missing required fields: name, email");
        }
        
        std::string name = x["name"].s();
        std::string email = x["email"].s();
        
        if (name.empty() || email.empty()) {
            return crow::response(400, "Name and email cannot be empty");
        }
        
        if (db->updateUser(user_id, name, email)) {
            crow::json::wvalue result;
            result["id"] = user_id;
            result["name"] = name;
            result["email"] = email;
            result["message"] = "User updated";
            return crow::response(result);
        } else {
            return crow::response(404, "User not found");
        }
    });

    CROW_ROUTE(app, "/users/<int>").methods("DELETE"_method)
    ([db](int user_id){
        std::cout << "   DELETE /users/" << user_id << " called" << std::endl;
        
        if (db->deleteUser(user_id)) {
            crow::json::wvalue result;
            result["message"] = "User deleted";
            result["id"] = user_id;
            return crow::response(result);
        } else {
            return crow::response(404, "User not found");
        }
    });
    
    std::cout << "   Registered routes:" << std::endl;
    std::cout << "   GET  /" << std::endl;
    std::cout << "   GET  /status" << std::endl;
    std::cout << "   POST /users" << std::endl;
    std::cout << "   GET  /users" << std::endl;
    std::cout << "   GET  /users/<id>" << std::endl;
    std::cout << "   GET  /users/email?address=..." << std::endl;
    std::cout << "   PUT  /users/<id>" << std::endl;
    std::cout << "   DELETE /users/<id>" << std::endl;
    std::cout << "   Server listening on port 8081" << std::endl;
    
    app.port(8081).multithreaded().run();
}